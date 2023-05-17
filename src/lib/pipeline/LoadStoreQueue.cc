#include "simeng/pipeline/LoadStoreQueue.hh"

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>

namespace simeng {
namespace pipeline {

/** Check whether requests `a` and `b` overlap. */
bool requestsOverlap(memory::MemoryAccessTarget a,
                     memory::MemoryAccessTarget b) {
  // Check whether one region ends before the other begins, implying no overlap,
  // and negate
  return !(a.vaddr + a.size <= b.vaddr || b.vaddr + b.size <= a.vaddr);
}

LoadStoreQueue::LoadStoreQueue(
    unsigned int maxCombinedSpace, std::shared_ptr<memory::MMU> mmu,
    span<PipelineBuffer<std::shared_ptr<Instruction>>> completionSlots,
    std::function<void(span<Register>, span<RegisterValue>)> forwardOperands,
    CompletionOrder completionOrder, bool exclusive, uint16_t loadBandwidth,
    uint16_t storeBandwidth, uint16_t permittedRequests,
    uint16_t permittedLoads, uint16_t permittedStores)
    : completionSlots_(completionSlots),
      forwardOperands_(forwardOperands),
      maxCombinedSpace_(maxCombinedSpace),
      combined_(true),
      mmu_(mmu),
      completionOrder_(completionOrder),
      exclusive_(exclusive),
      loadBandwidth_(loadBandwidth),
      storeBandwidth_(storeBandwidth),
      totalLimit_(permittedRequests),
      // Set per-cycle limits for each request type
      reqLimits_{permittedLoads, permittedStores} {};

LoadStoreQueue::LoadStoreQueue(
    unsigned int maxLoadQueueSpace, unsigned int maxStoreQueueSpace,
    std::shared_ptr<memory::MMU> mmu,
    span<PipelineBuffer<std::shared_ptr<Instruction>>> completionSlots,
    std::function<void(span<Register>, span<RegisterValue>)> forwardOperands,
    CompletionOrder completionOrder, bool exclusive, uint16_t loadBandwidth,
    uint16_t storeBandwidth, uint16_t permittedRequests,
    uint16_t permittedLoads, uint16_t permittedStores)
    : completionSlots_(completionSlots),
      forwardOperands_(forwardOperands),
      maxLoadQueueSpace_(maxLoadQueueSpace),
      maxStoreQueueSpace_(maxStoreQueueSpace),
      combined_(false),
      mmu_(mmu),
      completionOrder_(completionOrder),
      exclusive_(exclusive),
      loadBandwidth_(loadBandwidth),
      storeBandwidth_(storeBandwidth),
      totalLimit_(permittedRequests),
      // Set per-cycle limits for each request type
      reqLimits_{permittedLoads, permittedStores} {};

unsigned int LoadStoreQueue::getLoadQueueSpace() const {
  if (combined_) {
    return getCombinedSpace();
  } else {
    return getLoadQueueSplitSpace();
  }
}
unsigned int LoadStoreQueue::getStoreQueueSpace() const {
  if (combined_) {
    return getCombinedSpace();
  } else {
    return getStoreQueueSplitSpace();
  }
}
unsigned int LoadStoreQueue::getTotalSpace() const {
  if (combined_) {
    return getCombinedSpace();
  } else {
    return getLoadQueueSplitSpace() + getStoreQueueSplitSpace();
  }
}

unsigned int LoadStoreQueue::getLoadQueueSplitSpace() const {
  return maxLoadQueueSpace_ - loadQueue_.size();
}
unsigned int LoadStoreQueue::getStoreQueueSplitSpace() const {
  return maxStoreQueueSpace_ - storeQueue_.size();
}
unsigned int LoadStoreQueue::getCombinedSpace() const {
  return maxCombinedSpace_ - loadQueue_.size() - storeQueue_.size();
}

void LoadStoreQueue::addLoad(const std::shared_ptr<Instruction>& insn) {
  loadQueue_.push_back(insn);
}
void LoadStoreQueue::addStore(const std::shared_ptr<Instruction>& insn) {
  storeQueue_.push_back({insn, {}});
}

void LoadStoreQueue::startLoad(const std::shared_ptr<Instruction>& insn) {
  const auto& ld_addresses = insn->getGeneratedAddresses();
  if (ld_addresses.size() == 0) {
    // Early execution if not addresses need to be accessed
    insn->execute();
    completedLoads_.push(insn);
  } else {
    // If the completion order is inorder, reserve an entry in completedLoads_
    // now
    if (completionOrder_ == CompletionOrder::INORDER)
      completedLoads_.push(insn);

    // Detect reordering conflicts
    if (storeQueue_.size() > 0) {
      uint64_t seqId = insn->getSequenceId();
      for (auto itSt = storeQueue_.rbegin(); itSt != storeQueue_.rend();
           itSt++) {
        const auto& store = itSt->first;
        // If entry is earlier in the program order than load, detect conflicts
        if (store->getSequenceId() < seqId) {
          const auto& str_addresses = store->getGeneratedAddresses();
          // Iterate over possible overlaps between store and load addresses
          for (const auto& strAddr : str_addresses) {
            for (const auto& ldAddr : ld_addresses) {
              if (requestsOverlap(strAddr, ldAddr)) {
                // Conflict exists, add load instruction to conflictionMap_ and
                // delay until store retires
                conflictionMap_[store->getSequenceId()].push_back(insn);
                return;
              }
            }
          }
        }
      }
    }
    // No conflict found, process load
    requestLoadQueue_[tickCounter_ + insn->getLSQLatency()].push_back(insn);
    // Register active load
    requestedLoads_.emplace(insn->getSequenceId(), insn);
  }
}

void LoadStoreQueue::supplyStoreData(const std::shared_ptr<Instruction>& insn) {
  if (!insn->isStoreData()) return;
  // Get identifier values
  const uint64_t macroOpNum = insn->getInstructionId();
  const int microOpNum = insn->getMicroOpIndex();

  // Get data
  const auto& data = insn->getData();

  // Find storeQueue_ entry which is linked to the store data operation
  auto itSt = storeQueue_.begin();
  while (itSt != storeQueue_.end()) {
    auto& entry = itSt->first;
    // Pair entry and incoming store data operation with macroOp identifier and
    // microOp index value pre-detemined in microDecoder
    if (entry->getInstructionId() == macroOpNum &&
        entry->getMicroOpIndex() == microOpNum) {
      // Supply data to be stored by operations
      itSt->second = data;
      break;
    } else {
      itSt++;
    }
  }
}

bool LoadStoreQueue::commitStore(const std::shared_ptr<Instruction>& uop) {
  assert(storeQueue_.size() > 0 &&
         "Attempted to commit a store from an empty queue");
  assert(storeQueue_.front().first->getSequenceId() == uop->getSequenceId() &&
         "Attempted to commit a store that wasn't present at the front of the "
         "store queue");

  const auto& addresses = uop->getGeneratedAddresses();
  const auto& data = storeQueue_.front().second;

  // Early exit if there's no addresses to process
  if (addresses.size() == 0) {
    // TODO: Check if atomic lock needs to be released
    storeQueue_.pop_front();
    return false;
  }

  // Submit request write to memory interface early as the architectural state
  // considers the store to be retired and thus its operation complete
  mmu_->requestWrite(uop, data);
  // Still add sequenceID to requestQueue_ to ensure contention of resources is
  // correctly simulated
  requestStoreQueue_[tickCounter_ + uop->getLSQLatency()].push_back(uop);

  // Check all loads that have requested memory
  violatingLoad_ = nullptr;
  for (const auto& load : requestedLoads_) {
    // Skip loads that are younger than the oldest violating load
    if (violatingLoad_ &&
        load.second->getSequenceId() > violatingLoad_->getSequenceId())
      continue;
    // Violation invalid if the load and store entries are generated by the same
    // instruction
    if (load.second->getSequenceId() != uop->getSequenceId()) {
      const auto& loadedAddresses = load.second->getGeneratedAddresses();
      // Iterate over store addresses
      for (const auto& storeReq : addresses) {
        // Iterate over load addresses
        for (const auto& loadReq : loadedAddresses) {
          // Check for overlapping requests, and flush if discovered
          if (requestsOverlap(storeReq, loadReq)) {
            violatingLoad_ = load.second;
          }
        }
      }
    }
  }

  // Resolve any conflicts
  const auto& itr = conflictionMap_.find(uop->getSequenceId());
  if (itr != conflictionMap_.end()) {
    // For each load, we can now execute them given the conflicting
    // store has now been triggered
    auto ldVec = itr->second;
    for (auto load : ldVec) {
      std::queue<memory::MemoryAccessTarget> targets;
      for (auto const& addr : load->getGeneratedAddresses()) {
        targets.emplace(addr);
      }
      // Use store's latency to ensure that the load doesn't overtake the store
      // it conflicted with
      uint64_t strLat = tickCounter_ + uop->getLSQLatency();
      requestLoadQueue_[strLat + load->getLSQLatency()].push_back(load);
      // Register active load
      requestedLoads_.emplace(load->getSequenceId(), load);
    }
    // Remove all entries for this store from conflictionMap_
    conflictionMap_.erase(uop->getSequenceId());
  }

  if (uop->isStoreCond()) {
    assert(requestedCondStore_ == nullptr &&
           "[SimEng:LoadStoreQueue] Tried to issue a second conditional store "
           "whilst one is already in flight.");
    requestedCondStore_ = uop;
  }
  storeQueue_.pop_front();

  return violatingLoad_ != nullptr;
}

bool LoadStoreQueue::checkCondStore(const uint64_t sequenceId) {
  if (completedConditionalStore_ == -1) return false;
  if (completedConditionalStore_ != sequenceId) {
    std::cerr
        << "[SimEng:LoadStoreQueue] SequenceID of conditional-store at the "
           "front of the ROB is not equal to the completed conditional-Store."
        << std::endl;
    exit(1);
  }
  completedConditionalStore_ = -1;
  return true;
}

void LoadStoreQueue::commitLoad(const std::shared_ptr<Instruction>& uop) {
  assert(loadQueue_.size() > 0 &&
         "Attempted to commit a load from an empty queue");
  assert(loadQueue_.front()->getSequenceId() == uop->getSequenceId() &&
         "Attempted to commit a load that wasn't present at the front of the "
         "load queue");

  auto it = loadQueue_.begin();
  while (it != loadQueue_.end()) {
    const auto& entry = *it;
    if (entry->isLoad()) {
      requestedLoads_.erase(entry->getSequenceId());
      it = loadQueue_.erase(it);
      break;
    } else {
      it++;
    }
  }
}

void LoadStoreQueue::purgeFlushed() {
  // Remove flushed loads from load queue
  auto itLd = loadQueue_.begin();
  while (itLd != loadQueue_.end()) {
    const auto& entry = *itLd;
    if (entry->isFlushed()) {
      requestedLoads_.erase(entry->getSequenceId());
      itLd = loadQueue_.erase(itLd);
    } else {
      itLd++;
    }
  }

  // Remove flushed stores from store queue and confliction queue if an entry
  // exists
  auto itSt = storeQueue_.begin();
  while (itSt != storeQueue_.end()) {
    const auto& entry = itSt->first;
    if (entry->isFlushed()) {
      // Can erase all load entries as they must be younger than flushed store
      conflictionMap_.erase(entry->getSequenceId());
      itSt = storeQueue_.erase(itSt);
    } else {
      itSt++;
    }
  }

  // Remove flushed loads from confliction queue
  for (auto itCnflct = conflictionMap_.begin();
       itCnflct != conflictionMap_.end(); itCnflct++) {
    auto ldItr = itCnflct->second.begin();
    while (ldItr != itCnflct->second.end()) {
      if ((*ldItr)->isFlushed()) {
        ldItr = itCnflct->second.erase(ldItr);
      } else {
        ldItr++;
      }
    }
  }

  // Remove flushed loads and stores from request queues
  auto itLdReq = requestLoadQueue_.begin();
  while (itLdReq != requestLoadQueue_.end()) {
    auto itInsn = itLdReq->second.begin();
    while (itInsn != itLdReq->second.end()) {
      if ((*itInsn)->isFlushed()) {
        itInsn = itLdReq->second.erase(itInsn);
      } else {
        itInsn++;
      }
    }
    if (itLdReq->second.size() == 0) {
      itLdReq = requestLoadQueue_.erase(itLdReq);
    } else {
      itLdReq++;
    }
  }

  // Don't need to purge stores as they will only be sent at commit
}

void LoadStoreQueue::tick() {
  tickCounter_++;
  // Send memory requests adhering to set bandwidth and number of permitted
  // requests per cycle
  // Index 0: loads, index 1: stores
  std::array<uint16_t, 2> reqCounts = {0, 0};
  std::array<uint64_t, 2> dataTransfered = {0, 0};
  std::array<bool, 2> exceededLimits = {false, false};
  auto itLoad = requestLoadQueue_.begin();
  auto itStore = requestStoreQueue_.begin();
  while (requestLoadQueue_.size() + requestStoreQueue_.size() > 0) {
    // Choose which request type to schedule next
    bool chooseLoad = false;
    std::pair<bool, uint64_t> earliestLoad;
    std::pair<bool, uint64_t> earliestStore;
    // Determine if a load request can be scheduled
    if (requestLoadQueue_.size() == 0 || exceededLimits[accessType::LOAD]) {
      earliestLoad = {false, 0};
    } else {
      earliestLoad = {true, itLoad->first};
    }
    // Determine if a store request can be scheduled
    if (requestStoreQueue_.size() == 0 || exceededLimits[accessType::STORE]) {
      earliestStore = {false, 0};
    } else {
      earliestStore = {true, itStore->first};
    }
    // Choose between available requests favouring those constructed earlier
    // (store requests on a tie)
    if (earliestLoad.first) {
      chooseLoad = !(earliestStore.first &&
                     (earliestLoad.second >= earliestStore.second));
    } else if (!earliestStore.first) {
      break;
    }

    // Get next request to schedule
    auto& itReq = chooseLoad ? itLoad : itStore;
    auto itInsn = itReq->second.begin();
    auto bandwidth = chooseLoad ? loadBandwidth_ : storeBandwidth_;

    // Check if earliest request is ready
    if (itReq->first <= tickCounter_) {
      // Identify request type
      uint8_t isStore = 0;
      if (!chooseLoad) {
        isStore = 1;
      }
      // If LSQ only allows one type of request within a cycle, prevent other
      // type from being scheduled
      if (exclusive_) exceededLimits[!isStore] = true;

      // Iterate over requests ready this cycle
      while (itInsn != itReq->second.end()) {
        // Speculatively increment count of this request type
        reqCounts[isStore]++;

        // Ensure the limit on the number of permitted operations is adhered
        // to
        if (reqCounts[isStore] + reqCounts[!isStore] > totalLimit_) {
          // No more requests can be scheduled this cycle
          exceededLimits = {true, true};
          break;
        } else if (reqCounts[isStore] > reqLimits_[isStore]) {
          // No more requests of this type can be scheduled this cycle
          exceededLimits[isStore] = true;
          // Remove speculative increment to ensure it doesn't count for
          // comparisons aginast the totalLimit_
          reqCounts[isStore]--;
          break;
        } else {
          // Schedule requests from the queue of addresses in
          // request[Load|Store]Queue_ entry
          if (!isStore) {
            mmu_->requestRead((*itInsn));
          }
          // Remove entry from vector
          itInsn = itReq->second.erase(itInsn);

          // auto& addressQueue = itInsn->reqAddresses;
          // while (addressQueue.size()) {
          //   const simeng::memory::MemoryAccessTarget req =
          //   addressQueue.front();

          //   // Ensure the limit on the data transfered per cycle is adhered
          //   to assert(req.size <= bandwidth &&
          //          "Individual memory request from LoadStoreQueue exceeds L1
          //          " "bandwidth set and thus will never be submitted");
          //   dataTransfered[isStore] += req.size;
          //   if (dataTransfered[isStore] > bandwidth) {
          //     // No more requests can be scheduled this cycle
          //     exceededLimits[isStore] = true;
          //     itInsn = itReq->second.end();
          //     break;
          //   }

          //   // Request a read from the memory interface if the requestQueue_
          //   // entry represents a read
          //   if (!isStore) {
          //     mmu_->requestRead(req, itInsn->insn->getSequenceId(),
          //                       itInsn->insn->getInstructionId(),
          //                       itInsn->insn->isLoadReserved());
          //   }

          //   // Remove processed address from queue
          //   addressQueue.pop();
          // }
          // // Remove entry from vector iff all of its requests have been
          // // scheduled
          // if (addressQueue.size() == 0) {
          //   itInsn = itReq->second.erase(itInsn);
          // }
        }
      }

      // If all instructions for currently selected cycle in
      // request[Load|Store]Queue_ have been scheduled, erase entry
      if (itReq->second.size() == 0) {
        if (chooseLoad) {
          itReq = requestLoadQueue_.erase(itReq);
        } else {
          itReq = requestStoreQueue_.erase(itReq);
        }
      }
    } else {
      break;
    }
  }

  // Initialise completion counter
  size_t count = 0;

  // Process completed conditional store request.
  // No need to check if flushed as conditional store must be at front of
  // ROB to be committed
  // Check to see if conditional store is ready, if yes then forward result
  // and pass to writeback
  if (requestedCondStore_ != nullptr) {
    if (requestedCondStore_->isCondResultReady()) {
      completedConditionalStore_ = requestedCondStore_->getSequenceId();
      // Forward result. Given only 1 conditional store can be processed at a
      // time (as it can only be sent when at the front of the ROB, and blocks
      // further commits until the result has been returned), there is
      // guarenteed to be space in the completion slot.
      forwardOperands_(requestedCondStore_->getDestinationRegisters(),
                       requestedCondStore_->getResults());
      completionSlots_[count].getTailSlots()[0] =
          std::move(requestedCondStore_);
      count++;
      requestedCondStore_ = nullptr;
    }
  }

  // Process completed read requests
  auto load = requestedLoads_.begin();
  while (load != requestedLoads_.end()) {
    if (load->second->hasAllData() && !load->second->hasExecuted()) {
      // This load has completed
      load->second->execute();
      if (load->second->isStoreData()) {
        supplyStoreData(load->second);
      }
      // If the completion order is OoO, add entry to completedLoads_
      if (completionOrder_ == CompletionOrder::OUTOFORDER)
        completedLoads_.push(load->second);
    }
    load++;
  }

  // Pop from the front of the completed loads queue and send to writeback
  while (completedLoads_.size() > 0 && count < completionSlots_.size()) {
    // Skip a completion slot if stalled
    if (completionSlots_[count].isStalled()) {
      count++;
      continue;
    }

    const auto& insn = completedLoads_.front();

    // Don't process load instruction if it has been flushed
    if (insn->isFlushed()) {
      completedLoads_.pop();
      continue;
    }

    // If the load at the front of the queue is yet to execute, continue
    // processing next cycle
    if (!insn->hasExecuted()) {
      break;
    }

    // Forward the results
    forwardOperands_(insn->getDestinationRegisters(), insn->getResults());

    completionSlots_[count].getTailSlots()[0] = std::move(insn);

    completedLoads_.pop();

    count++;
  }
}

std::shared_ptr<Instruction> LoadStoreQueue::getViolatingLoad() const {
  return violatingLoad_;
}

bool LoadStoreQueue::isCombined() const { return combined_; }

}  // namespace pipeline
}  // namespace simeng

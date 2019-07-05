#include "LoadStoreQueue.hh"

#include <cassert>
#include <cstring>

namespace simeng {
namespace pipeline {

/** Check whether requests `a` and `b` overlap. */
bool requestsOverlap(std::pair<uint64_t, uint8_t> a,
                     std::pair<uint64_t, uint8_t> b) {
  // Check whether one region ends before the other begins, implying no overlap,
  // and negate
  return !(a.first + a.second <= b.first || b.first + b.second <= a.first);
}

LoadStoreQueue::LoadStoreQueue(
    unsigned int maxCombinedSpace, MemoryInterface& memory,
    span<PipelineBuffer<std::shared_ptr<Instruction>>> completionSlots,
    std::function<void(span<Register>, span<RegisterValue>)> forwardOperands)
    : completionSlots_(completionSlots),
      forwardOperands_(forwardOperands),
      maxCombinedSpace_(maxCombinedSpace),
      combined_(true),
      memory_(memory){};

LoadStoreQueue::LoadStoreQueue(
    unsigned int maxLoadQueueSpace, unsigned int maxStoreQueueSpace,
    MemoryInterface& memory,
    span<PipelineBuffer<std::shared_ptr<Instruction>>> completionSlots,
    std::function<void(span<Register>, span<RegisterValue>)> forwardOperands)
    : completionSlots_(completionSlots),
      forwardOperands_(forwardOperands),
      maxLoadQueueSpace_(maxLoadQueueSpace),
      maxStoreQueueSpace_(maxStoreQueueSpace),
      combined_(false),
      memory_(memory){};

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
  storeQueue_.push_back(insn);
}

void LoadStoreQueue::startLoad(const std::shared_ptr<Instruction>& insn) {
  const auto& addresses = insn->getGeneratedAddresses();
  for (auto const& request : addresses) {
    memory_.requestRead({request.first, request.second});
  }
}

bool LoadStoreQueue::commitStore(const std::shared_ptr<Instruction>& uop) {
  assert(storeQueue_.size() > 0 &&
         "Attempted to commit a store from an empty queue");
  assert(storeQueue_.front()->getSequenceId() == uop->getSequenceId() &&
         "Attempted to commit a store that wasn't present at the front of the "
         "store queue");

  const auto& addresses = uop->getGeneratedAddresses();
  const auto& data = uop->getData();
  for (size_t i = 0; i < addresses.size(); i++) {
    const auto& request = addresses[i];
    memory_.requestWrite({request.first, request.second}, data[i]);
  }

  for (const auto& load : loadQueue_) {
    // Find all loads ready to commit
    // TODO: Partially ready loads also need disambiguation
    if (load->hasExecuted()) {
      const auto& loadedAddresses = load->getGeneratedAddresses();
      // Iterate over store addresses
      for (const auto& storeReq : addresses) {
        // Iterate over load addresses
        for (const auto& loadReq : loadedAddresses) {
          // Check for overlapping requests, and flush if discovered
          if (requestsOverlap(storeReq, loadReq)) {
            violatingLoad_ = load;

            storeQueue_.pop_front();
            return true;
          }
        }
      }
    }
  }

  storeQueue_.pop_front();
  return false;
}

void LoadStoreQueue::commitLoad(const std::shared_ptr<Instruction>& uop) {
  assert(loadQueue_.size() > 0 &&
         "Attempted to commit a load from an empty queue");
  assert(loadQueue_.front()->getSequenceId() == uop->getSequenceId() &&
         "Attempted to commit a load that wasn't present at the front of the "
         "load queue");

  loadQueue_.pop_front();
}

void LoadStoreQueue::purgeFlushed() {
  auto it = loadQueue_.begin();
  while (it != loadQueue_.end()) {
    auto& entry = *it;
    if (entry->isFlushed()) {
      it = loadQueue_.erase(it);
    } else {
      it++;
    }
  }

  it = storeQueue_.begin();
  while (it != storeQueue_.end()) {
    auto& entry = *it;
    if (entry->isFlushed()) {
      it = storeQueue_.erase(it);
    } else {
      it++;
    }
  }
}

void LoadStoreQueue::tick() {
  // Process completed read requests
  for (const auto& response : memory_.getCompletedReads()) {
    auto address = response.first.address;
    const auto& data = response.second;

    // TODO: Detect and handle any faults
    // TODO: Create a data structure to allow direct lookup of requests waiting
    // on a read, rather than iterating over the queue (see DispatchIssueQueue's
    // dependency matrix)
    for (const auto& load : loadQueue_) {
      // Ignore completed or not-yet-ready loads
      if (!load->hasAllData()) {
        load->supplyData(address, data);
        if (load->hasAllData()) {
          // This load has completed
          load->execute();
          completedLoads_.push(load);
        }
      }
    }
  }
  memory_.clearCompletedReads();

  // Pop from the front of the completed loads queue and send to writeback
  size_t count = 0;
  while (completedLoads_.size() > 0 && count < completionSlots_.size()) {
    const auto& insn = completedLoads_.front();
    completionSlots_[count].getTailSlots()[0] = insn;

    // Forward the results
    forwardOperands_(insn->getDestinationRegisters(), insn->getResults());

    completedLoads_.pop();
  }
}

std::shared_ptr<Instruction> LoadStoreQueue::getViolatingLoad() const {
  return violatingLoad_;
}

bool LoadStoreQueue::isCombined() const { return combined_; }

}  // namespace pipeline
}  // namespace simeng
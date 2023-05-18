#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <queue>

#include "simeng/Config.hh"
#include "simeng/Instruction.hh"
#include "simeng/OS/Constants.hh"
#include "simeng/Port.hh"
#include "simeng/memory/MemPacket.hh"
#include "simeng/memory/MemRequests.hh"
#include "simeng/span.hh"
#include "simeng/util/Math.hh"

typedef std::function<uint64_t(uint64_t, uint64_t)> VAddrTranslator;

namespace simeng {

namespace memory {

class MMU {
 public:
  MMU(VAddrTranslator fn);

  ~MMU() {}

  /** Tick the memory model to process the request queue. */
  void tick();

  /** Queue a read request. Returns true if there is space for the request.
   * Return false otherwise. */
  bool requestRead(const std::shared_ptr<Instruction>& uop);

  /** Queue a write request. Returns true if there is space for the request.
   * Return false otherwise. */
  bool requestWrite(const std::shared_ptr<Instruction>& uop,
                    const std::vector<RegisterValue>& data);

  /** Process a write request of `data` to the target location that is not
   * associated to an instruction, or bound band bandwidth limits. */
  void requestWrite(const MemoryAccessTarget& target,
                    const RegisterValue& data);

  /** Queue a read request from the supplied target location. This has zero
   * latency as instruction cache is not currently modelled. */
  void requestInstrRead(const MemoryAccessTarget& target);

  /** Retrieve all completed instruction read requests. */
  const span<MemoryReadResult> getCompletedInstrReads() const;

  /** Clear the completed instruction reads. */
  void clearCompletedIntrReads();

  /** Returns true if there are any oustanding memory requests in-flight. */
  bool hasPendingRequests() const;

  /** Method to set the TID for the MMU. */
  void setTid(uint64_t tid);

  /** Function used to initialise the Data Port used for bidirection
   * communication. */
  std::shared_ptr<Port<std::unique_ptr<MemPacket>>> initPort();

 private:
  /** Process load or store requests. */
  void processRequests(const bool isStore);

  /** Method used to buffer data requests to memory. */
  void issueRequest(std::unique_ptr<MemPacket> request);

  /** Open a new cache line monitor. */
  void openLLSCMonitor(const std::shared_ptr<Instruction>& loadRes);

  /** Checks whether a valid monitor is open for a store conditional. Returns
   * whether the store can proceed or not. */
  bool checkLLSCMonitor(const std::shared_ptr<Instruction>& strCond);

  /** Potentially updates the local cache line monitor to enforce correct LL/SC
   * behaviour. */
  void updateLLSCMonitor(const MemoryAccessTarget& storeTarget);

  /** Returns true if unsigned overflow occurs. */
  bool unsignedOverflow(uint64_t a, uint64_t b) const {
    return (a + b) < a || (a + b) < b;
  }

  /** A map containing all load instructions waiting for their results.
   * Key = Instruction sequenceID
   * Value = Instruction */
  std::map<uint64_t, std::shared_ptr<Instruction>> requestedLoads_;

  /** A vector containing all completed Instruction read requests. */
  std::vector<MemoryReadResult> completedInstrReads_;

  /** The number of pending data requests. */
  uint64_t pendingDataRequests_ = 0;

  /** TID of the process currently communicating with this MMU. */
  uint64_t tid_ = 0;

  // We model "weak" LL/SC support (as is the case in the majority of hardware)
  // and so only one monitor can be usable. Atomics are processed when at the
  // head of ROB so no speculation, and are assumed to be correctly aligned.
  /** The cache line monitor represented as a pair. Containes a set of cache
   * line addresses within monitor, and whether the monitor is valid. */
  std::pair<std::set<uint64_t>, bool> cacheLineMonitor_;

  /** Width of a cache line. */
  const uint64_t cacheLineWidth_;

  /** Fixed array containing vectors for all loads and store requests.
   * First in array contains all load requests for a number of instructions.
   * Each inner vector represents a single instruction.
   *
   * Second in array contains all store requests for a number of instructions.
   * Each inner vector represents a single instruction. */
  std::array<std::vector<std::vector<std::unique_ptr<MemPacket>>>, 2>
      loadsStores_;

  /** Constant indexes for the loadStores_ array. */
  static constexpr uint8_t LD = 0;
  static constexpr uint8_t STR = 1;

  /** The per-cycle total load bandwidth. */
  uint64_t loadBandwidth_;

  /** The per-cycle total store bandwidth. */
  uint64_t storeBandwidth_;

  /** The number of total requests (instructions) permitted per cycle. */
  uint64_t requestLimit_;

  /** The number of load requests (instructions) permitted per cycle. */
  uint64_t loadRequestLimit_;

  /** The number of store requests (instructions) permitted per cycle. */
  uint64_t storeRequestLimit_;

  /** If true, then load and stores can share pipes. If false then there are
   * individual load and store pipes. */
  bool exclusiveRequests_;

  /** Callback function which invokes the OS for translation on
   * TLB misses. */
  VAddrTranslator translate_;

  /** Data port used for communication with the memory hierarchy. */
  std::shared_ptr<Port<std::unique_ptr<MemPacket>>> port_ = nullptr;
};

}  // namespace memory
}  // namespace simeng

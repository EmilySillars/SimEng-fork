#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "simeng/Config.hh"
#include "simeng/Core.hh"
#include "simeng/OS/Constants.hh"
#include "simeng/OS/PageFrameAllocator.hh"
#include "simeng/OS/SyscallHandler.hh"
#include "simeng/OS/Vma.hh"
#include "simeng/SpecialFileDirGen.hh"
#include "simeng/arch/Architecture.hh"
#include "simeng/arch/aarch64/Architecture.hh"
#include "simeng/arch/aarch64/Instruction.hh"
#include "simeng/arch/riscv/Architecture.hh"
#include "simeng/arch/riscv/Instruction.hh"
#include "simeng/memory/MMU.hh"
#include "simeng/memory/SimpleMem.hh"
#include "simeng/span.hh"

// Forward declare RegressionTest class so that it can be declared a friend.
class RegressionTest;

namespace simeng {
namespace OS {

using namespace simeng::OS::defaults;

// Program used when no executable is provided; counts down from
// 1024*1024, with an independent `orr` at the start of each branch.
static uint32_t hex_[8] = {
    0x320C03E0,  // orr w0, wzr, #1048576
    0x320003E1,  // orr w0, wzr, #1
    0x71000400,  // subs w0, w0, #1
    0x54FFFFC1,  // b.ne -8
                 // .exit:
    0xD2800000,  // mov x0, #0
    0xD2800BC8,  // mov x8, #94
    0xD4000001,  // svc #0
};

/** A simple, lightweight Operating System kernel based on Linux to emulate
 * syscalls and manage process execution. */
class SimOS {
 public:
  /** Construct a SimOS object which creates the initial Process from a byte
   * stream. */
  SimOS(std::shared_ptr<simeng::memory::Mem> mem,
        simeng::span<char> instrBytes);

  /** Construct a SimOS object from a binary file specified via the runtime
   * arguments of SimEng. */
  SimOS(std::shared_ptr<simeng::memory::Mem> mem, std::string executablePath,
        std::vector<std::string> executableArgs);

  /** Tick SimOS. */
  void tick();

  /** Create a new Process Object.
   * A span<char> is optionally passed into the function to indicate if the
   * Process is created via raw bytes or via a compiled binary.
   * Returns the tid of the process that was created. */
  uint64_t createProcess(span<char> instructionBytes = span<char>());

  /** Get a process with specified `tid`. */
  const std::shared_ptr<Process>& getProcess(uint64_t tid);

  /** Get the number of active processes. */
  const size_t getNumProcesses() { return processes_.size(); }

  /** Terminate the process with threadID = `tid`, and set the corresponding
   * core to an idle state if applicable. */
  void terminateThread(uint64_t tid);

  /** Terminate all processes with threadGroupID = `tgid`, and set corresponding
   * cores to an idle state if applicable. */
  void terminateThreadGroup(uint64_t tgid);

  /** Method which allocates multiple page frames of size 'PAGE_SIZE' to cover
   * an address range of 'size' and returns the starting physical address. */
  uint64_t requestPageFrames(size_t size);

  /** Method which handles process specific page table translation. */
  uint64_t handleVAddrTranslation(uint64_t vaddr, uint64_t tid);

  /** This method returns a callback function that is passed to the MMU.
   * The callback function will be used by the MMU to handle TLB misses. The
   * callback invokes SimOS for virtual address translations. */
  virtual VAddrTranslator getVAddrTranslator();

  /** Get shared_ptr to syscallHandler instance. */
  std::shared_ptr<SyscallHandler> getSyscallHandler() const {
    return syscallHandler_;
  }

  /** Register a core with the OS to enable process scheduling. */
  void registerCore(std::shared_ptr<simeng::Core> core) {
    cores_.push_back(core);
  }

  /** Check if OS has halted. */
  bool hasHalted() const { return halted_; };

  /** Unique pointer to host backed file mmap. */
  std::unique_ptr<HostBackedFileMMaps> const hfmmap_ =
      std::make_unique<HostBackedFileMMaps>();

  /** Set up friend class with RegressionTest to enable exclusive access to
   * private functions. */
  friend class ::RegressionTest;

 private:
  /** Private constructor, called by all public constructors to perform common
   * logic. */
  SimOS(std::shared_ptr<simeng::memory::Mem> mem);

  /** Construct the special file directory. */
  void createSpecialFileDirectory() const;

  /** Change the state of a process or a core.
   * Used by the terminateThread & terminateThreadGroup functions. */
  void terminateThreadHelper(std::shared_ptr<Process> proc);

  /** The path of user defined Executable. */
  std::string executablePath_ = DEFAULT_STR;

  /** The runtime arguments of the user defined executable. */
  std::vector<std::string> executableArgs_ = {};

  /** The map of user-space processes running above the OS kernel.
   * Key = process' TID
   * Value = Shared pointer to process object with TID = key. */
  std::unordered_map<uint64_t, std::shared_ptr<Process>> processes_ = {};

  /** Queue of processes waiting to be scheduled. */
  std::queue<std::shared_ptr<Process>> waitingProcs_ = {};

  /** Queue of processes that have successfully sent an interrupt signal to a
   * core and are waiting to be scheduled on it. */
  std::queue<std::shared_ptr<Process>> scheduledProcs_ = {};

  /** The list of cores. */
  std::vector<std::shared_ptr<simeng::Core>> cores_ = {};

  /** Reference to the simulation memory shared pointer */
  std::shared_ptr<simeng::memory::Mem> memory_ = nullptr;

  /** SyscallHandler Object to process all syscalls. */
  std::shared_ptr<SyscallHandler> syscallHandler_;

  /** Indicates if all processes have completed or a core has halted due to an
   * exception. */
  bool halted_ = false;

  /** The value of the next TID value that should be assigned to a process on
   * instantiation. */
  uint64_t nextFreeTID_ = 0;

  /** Reference to the PageFrameAllocator object.  */
  PageFrameAllocator pageFrameAllocator_;
};

}  // namespace OS
}  // namespace simeng

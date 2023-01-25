#include "simeng/memory/Mem.hh"

namespace simeng {
namespace memory {
uint64_t DataPacket::pktIdCtr = 0;

DataPacket::DataPacket(DataPacketAccessType accType) {
  id = pktIdCtr++;
  type = accType;
}

ReadPacket::ReadPacket(uint64_t addr, size_t sz) : DataPacket(READ) {
  address = addr;
  size = sz;
};

ReadRespPacket* ReadPacket::makeResponse(uint64_t bytesRead, char* data) {
  return new ReadRespPacket(this->id, bytesRead, data);
}

ReadRespPacket::ReadRespPacket(uint64_t req_id, size_t bytes_read, char* dt)
    : DataPacket(READ) {
  reqId = req_id;
  bytesRead = bytes_read;
  data = dt;
}

WritePacket::WritePacket(uint64_t addr, size_t sz, const char* dt)
    : DataPacket(WRITE), data(dt) {
  address = addr;
  size = sz;
  data = dt;
}

WriteRespPacket* WritePacket::makeResponse(uint64_t bytesReturned) {
  return new WriteRespPacket(this->id, bytesReturned);
}

WriteRespPacket::WriteRespPacket(uint64_t req_id, size_t bytes_written)
    : DataPacket(WRITE) {
  reqId = req_id;
  bytesWritten = bytes_written;
}

}  // namespace memory
}  // namespace simeng
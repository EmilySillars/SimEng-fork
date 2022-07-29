#include "simeng/Statistics.hh"

namespace simeng {

Statistics::Statistics(std::string outfile) { outfile_.open(outfile); }
Statistics::~Statistics() { outfile_.close(); }

uint64_t Statistics::registerStat(std::string name) {
  // If the stat is already registered, return existing id
  auto it = std::find(statNames_.begin(), statNames_.end(), name);
  uint64_t id = 0;
  if (it != statNames_.end()) {
    id = it - statNames_.begin();
  } else {
    id = statNames_.size();
    // Add additional entry to each vector
    statNames_.push_back(name);
    regionStats_.push_back(0);
    fullSimulationStats_.push_back(0);
  }
  return id;
}

void Statistics::incrementStat(uint64_t id, uint64_t value) {
  assert(id < regionStats_.size() && "Id not registered");
  regionStats_[id] += value;
  fullSimulationStats_[id] += value;
}

void Statistics::dumpStats(uint64_t dumpAddress) {
  outfile_ << "===== " << dumpCounter_ << " === " << std::hex << dumpAddress
           << std::dec << " =====\n";
  for (int i = 0; i < statNames_.size(); i++) {
    outfile_ << statNames_[i] << ": " << regionStats_[i] << "\n";
  }
  dumpCounter_++;
}

void Statistics::resetStats() {
  for (int i = 0; i < regionStats_.size(); i++) regionStats_[i] = 0;
}

void Statistics::getGeneralSimulationStats(
    std::map<std::string, std::string>& statMap) {
  // Fill statMap with values based on keys in statMap
  for (auto& key : statMap) {
    auto it = std::find(statNames_.begin(), statNames_.end(), key.first);
    assert(it != statNames_.end() && "Invalid simulation statistic name");
    statMap[key.first] =
        std::to_string(fullSimulationStats_[it - statNames_.begin()]);
  }
}
}  // namespace simeng

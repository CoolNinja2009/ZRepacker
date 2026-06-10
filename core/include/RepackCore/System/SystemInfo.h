#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace RepackCore {

struct HardwareProfile {
    uint32_t logicalCores = 1;
    uint64_t totalRam = 0;
    uint64_t availableRam = 0;
};

class HardwareProfiler {
public:
    HardwareProfile query() const;
    uint64_t estimateRamUsage(const std::string& mode) const;
    uint32_t recommendThreadCount(const std::string& mode) const;
};

class DiskSpaceChecker {
public:
    uint64_t availableBytes(const std::filesystem::path& path) const;
    bool hasEnoughSpace(const std::filesystem::path& path, uint64_t bytes) const;
};

}


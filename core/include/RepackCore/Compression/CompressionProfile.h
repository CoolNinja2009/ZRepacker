#pragma once

#include <cstdint>
#include <string>

namespace RepackCore {

enum class CompressionProfile {
    Fast,
    Balanced,
    Maximum,
    Extreme
};

struct CompressionProfileSettings {
    CompressionProfile profile = CompressionProfile::Balanced;
    std::string algorithm = "store";
    int level = 0;
    uint64_t dictionarySize = 64ull * 1024ull * 1024ull;
    uint64_t chunkSize = 64ull * 1024ull * 1024ull;
    bool useSrep = false;
    bool usePrecomp = false;
    int threadCount = 0;
};

CompressionProfile parseCompressionProfile(const std::string& value);
CompressionProfileSettings defaultsForProfile(CompressionProfile profile);
std::string toString(CompressionProfile profile);

}


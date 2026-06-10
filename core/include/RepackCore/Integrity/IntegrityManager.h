#pragma once

#include <filesystem>
#include <istream>
#include <string>
#include "RepackCore/Archive/ArchiveManifest.h"
#include "RepackCore/Common/Result.h"

namespace RepackCore {

class IntegrityManager {
public:
    uint32_t crc32File(const std::filesystem::path& path) const;
    uint64_t hash64File(const std::filesystem::path& path) const;
    uint32_t crc32Stream(std::istream& stream, uint64_t bytes) const;
    uint64_t hash64Stream(std::istream& stream, uint64_t bytes) const;

    Result<void> verifyFile(const std::filesystem::path& path, const FileEntry& expected) const;
    Result<void> verifyArchiveParts(const std::filesystem::path& folder,
                                    const ArchiveManifest& manifest,
                                    bool deepHashVerification = true) const;
};

uint32_t crc32Bytes(const unsigned char* data, size_t size, uint32_t seed = 0xFFFFFFFFu);
uint64_t fnv1a64Bytes(const unsigned char* data, size_t size, uint64_t seed = 1469598103934665603ull);
std::string hex32(uint32_t value);
std::string hex64(uint64_t value);

}

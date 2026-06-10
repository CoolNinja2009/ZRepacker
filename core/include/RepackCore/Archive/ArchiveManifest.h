#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include "RepackCore/Compression/CompressionProfile.h"

namespace RepackCore {

struct FileEntry {
    std::string relativePath;
    uint64_t originalSize = 0;
    uint64_t archiveOffset = 0;
    uint64_t storedSize = 0;
    uint32_t crc32 = 0;
    uint64_t hash64 = 0;
};

struct ArchivePart {
    std::string fileName;
    uint32_t index = 0;
    uint64_t size = 0;
    uint32_t crc32 = 0;
    uint64_t hash64 = 0;
};

struct ArchiveManifest {
    std::string formatVersion = "2";
    std::string gameName;
    std::string version;
    std::string publisher;
    std::string installerTitle;
    std::string installerDescription;
    std::string accentColor = "#4F8CFF";
    std::string algorithm = "store";
    std::string profile = "balanced";
    uint64_t installSize = 0;
    uint64_t compressedSize = 0;
    uint64_t splitSize = 0;
    std::vector<FileEntry> files;
    std::vector<ArchivePart> parts;
};

std::string serializeManifest(const ArchiveManifest& manifest);
ArchiveManifest parseManifest(const std::string& text);

}


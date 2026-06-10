#include "RepackCore/RepackCore.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace RepackCore {

namespace {

constexpr char kMagic[8] = {'R', 'P', 'K', 'B', 'I', 'N', '1', '\n'};
constexpr uint32_t kHeaderSize = 64;

// Each file is split into chunks of up to this many original bytes.
// Each chunk is compressed independently and stored with an 8-byte
// header:  uint32_t compressedSize  |  uint32_t originalSize.
constexpr uint64_t kCompressionChunkSize = 64ull * 1024ull * 1024ull; // 64 MB

struct PartHeader {
    char magic[8];
    uint32_t headerSize;
    uint32_t partIndex;
    uint64_t payloadBytes;
    uint64_t reserved[5];
};

static_assert(sizeof(PartHeader) == kHeaderSize);

std::string normalizePath(const fs::path& p) {
    auto s = p.generic_string();
    while (!s.empty() && (s.front() == '/' || s.front() == '\\')) s.erase(s.begin());
    return s;
}

std::string escape(const std::string& value) {
    std::string out;
    for (char c : value) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        default: out += c; break;
        }
    }
    return out;
}

std::string unescape(const std::string& value) {
    std::string out;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char n = value[++i];
            if (n == 'n') out += '\n';
            else if (n == 't') out += '\t';
            else out += n;
        } else {
            out += value[i];
        }
    }
    return out;
}

std::vector<unsigned char> readBlock(std::istream& input, size_t maxBytes) {
    std::vector<unsigned char> buffer(maxBytes);
    input.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    buffer.resize(static_cast<size_t>(input.gcount()));
    return buffer;
}

void writeHeader(std::fstream& file, uint32_t index, uint64_t payloadBytes) {
    PartHeader h{};
    std::copy(std::begin(kMagic), std::end(kMagic), h.magic);
    h.headerSize = kHeaderSize;
    h.partIndex = index;
    h.payloadBytes = payloadBytes;
    file.seekp(0);
    file.write(reinterpret_cast<const char*>(&h), sizeof(h));
}

Result<PartHeader> readHeader(std::ifstream& file) {
    PartHeader h{};
    file.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (!file || !std::equal(std::begin(kMagic), std::end(kMagic), h.magic)) {
        return Result<PartHeader>::failure("Invalid archive part header.");
    }
    return Result<PartHeader>::success(h);
}

std::vector<fs::path> scanFiles(const fs::path& root) {
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (entry.is_regular_file()) files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}

void writeTextFile(const fs::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

}

CompressionProfile parseCompressionProfile(const std::string& value) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    if (v == "fast") return CompressionProfile::Fast;
    if (v == "maximum") return CompressionProfile::Maximum;
    if (v == "extreme") return CompressionProfile::Extreme;
    return CompressionProfile::Balanced;
}

CompressionProfileSettings defaultsForProfile(CompressionProfile profile) {
    CompressionProfileSettings s;
    s.profile = profile;
    switch (profile) {
    case CompressionProfile::Fast:
        s.algorithm = "store";
        s.level = 1;
        s.chunkSize = 64ull * 1024ull * 1024ull;
        break;
    case CompressionProfile::Balanced:
        s.algorithm = "store";
        s.level = 5;
        s.chunkSize = 128ull * 1024ull * 1024ull;
        break;
    case CompressionProfile::Maximum:
        s.algorithm = "lzma2";
        s.level = 9;
        s.dictionarySize = 512ull * 1024ull * 1024ull;
        s.chunkSize = 256ull * 1024ull * 1024ull;
        s.useSrep = true;
        break;
    case CompressionProfile::Extreme:
        s.algorithm = "lzma2";
        s.level = 9;
        s.dictionarySize = 1024ull * 1024ull * 1024ull;
        s.chunkSize = 512ull * 1024ull * 1024ull;
        s.useSrep = true;
        s.usePrecomp = true;
        break;
    }
    s.threadCount = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    return s;
}

std::string toString(CompressionProfile profile) {
    switch (profile) {
    case CompressionProfile::Fast: return "fast";
    case CompressionProfile::Balanced: return "balanced";
    case CompressionProfile::Maximum: return "maximum";
    case CompressionProfile::Extreme: return "extreme";
    }
    return "balanced";
}

std::string serializeManifest(const ArchiveManifest& m) {
    std::ostringstream out;
    out << "formatVersion=" << escape(m.formatVersion) << "\n";
    out << "gameName=" << escape(m.gameName) << "\n";
    out << "version=" << escape(m.version) << "\n";
    out << "publisher=" << escape(m.publisher) << "\n";
    out << "installerTitle=" << escape(m.installerTitle) << "\n";
    out << "installerDescription=" << escape(m.installerDescription) << "\n";
    out << "accentColor=" << escape(m.accentColor) << "\n";
    out << "algorithm=" << escape(m.algorithm) << "\n";
    out << "profile=" << escape(m.profile) << "\n";
    out << "installSize=" << m.installSize << "\n";
    out << "compressedSize=" << m.compressedSize << "\n";
    out << "splitSize=" << m.splitSize << "\n";
    out << "parts=" << m.parts.size() << "\n";
    for (const auto& p : m.parts) {
        out << "part|" << escape(p.fileName) << "|" << p.index << "|" << p.size << "|"
            << p.crc32 << "|" << p.hash64 << "\n";
    }
    out << "files=" << m.files.size() << "\n";
    for (const auto& f : m.files) {
        out << "file|" << escape(f.relativePath) << "|" << f.originalSize << "|"
            << f.archiveOffset << "|" << f.storedSize << "|" << f.crc32 << "|" << f.hash64 << "\n";
    }
    return out.str();
}

ArchiveManifest parseManifest(const std::string& text) {
    ArchiveManifest m;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("part|", 0) == 0) {
            std::vector<std::string> fields;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, '|')) fields.push_back(part);
            if (fields.size() == 6) {
                ArchivePart p;
                p.fileName = unescape(fields[1]);
                p.index = static_cast<uint32_t>(std::stoul(fields[2]));
                p.size = std::stoull(fields[3]);
                p.crc32 = static_cast<uint32_t>(std::stoul(fields[4]));
                p.hash64 = std::stoull(fields[5]);
                m.parts.push_back(p);
            }
        } else if (line.rfind("file|", 0) == 0) {
            std::vector<std::string> fields;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, '|')) fields.push_back(part);
            if (fields.size() == 7) {
                FileEntry f;
                f.relativePath = unescape(fields[1]);
                f.originalSize = std::stoull(fields[2]);
                f.archiveOffset = std::stoull(fields[3]);
                f.storedSize = std::stoull(fields[4]);
                f.crc32 = static_cast<uint32_t>(std::stoul(fields[5]));
                f.hash64 = std::stoull(fields[6]);
                m.files.push_back(f);
            }
        } else {
            const auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            const auto key = line.substr(0, pos);
            const auto value = unescape(line.substr(pos + 1));
            if (key == "formatVersion") m.formatVersion = value;
            else if (key == "gameName") m.gameName = value;
            else if (key == "version") m.version = value;
            else if (key == "publisher") m.publisher = value;
            else if (key == "installerTitle") m.installerTitle = value;
            else if (key == "installerDescription") m.installerDescription = value;
            else if (key == "accentColor") m.accentColor = value;
            else if (key == "algorithm") m.algorithm = value;
            else if (key == "profile") m.profile = value;
            else if (key == "installSize") m.installSize = std::stoull(value);
            else if (key == "compressedSize") m.compressedSize = std::stoull(value);
            else if (key == "splitSize") m.splitSize = std::stoull(value);
        }
    }
    return m;
}

uint32_t crc32Bytes(const unsigned char* data, size_t size, uint32_t seed) {
    uint32_t crc = seed;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
        }
    }
    return crc;
}

uint64_t fnv1a64Bytes(const unsigned char* data, size_t size, uint64_t seed) {
    uint64_t h = seed;
    for (size_t i = 0; i < size; ++i) {
        h ^= data[i];
        h *= 1099511628211ull;
    }
    return h;
}

std::string hex32(uint32_t value) {
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << value;
    return ss.str();
}

std::string hex64(uint64_t value) {
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << value;
    return ss.str();
}

uint32_t IntegrityManager::crc32File(const fs::path& path) const {
    std::ifstream in(path, std::ios::binary);
    uint32_t crc = 0xFFFFFFFFu;
    while (in) {
        auto block = readBlock(in, 1024 * 1024);
        if (!block.empty()) crc = crc32Bytes(block.data(), block.size(), crc);
    }
    return crc ^ 0xFFFFFFFFu;
}

uint64_t IntegrityManager::hash64File(const fs::path& path) const {
    std::ifstream in(path, std::ios::binary);
    uint64_t h = 1469598103934665603ull;
    while (in) {
        auto block = readBlock(in, 1024 * 1024);
        if (!block.empty()) h = fnv1a64Bytes(block.data(), block.size(), h);
    }
    return h;
}

uint32_t IntegrityManager::crc32Stream(std::istream& stream, uint64_t bytes) const {
    uint32_t crc = 0xFFFFFFFFu;
    uint64_t remaining = bytes;
    while (remaining > 0 && stream) {
        auto block = readBlock(stream, static_cast<size_t>(std::min<uint64_t>(1024 * 1024, remaining)));
        if (block.empty()) break;
        remaining -= block.size();
        crc = crc32Bytes(block.data(), block.size(), crc);
    }
    return crc ^ 0xFFFFFFFFu;
}

uint64_t IntegrityManager::hash64Stream(std::istream& stream, uint64_t bytes) const {
    uint64_t h = 1469598103934665603ull;
    uint64_t remaining = bytes;
    while (remaining > 0 && stream) {
        auto block = readBlock(stream, static_cast<size_t>(std::min<uint64_t>(1024 * 1024, remaining)));
        if (block.empty()) break;
        remaining -= block.size();
        h = fnv1a64Bytes(block.data(), block.size(), h);
    }
    return h;
}

Result<void> IntegrityManager::verifyFile(const fs::path& path, const FileEntry& expected) const {
    if (!fs::exists(path)) return Result<void>::failure(expected.relativePath + " is missing after extraction.");
    if (fs::file_size(path) != expected.originalSize) return Result<void>::failure(expected.relativePath + " has an invalid size.");
    if (crc32File(path) != expected.crc32) return Result<void>::failure(expected.relativePath + " failed CRC verification.");
    if (hash64File(path) != expected.hash64) return Result<void>::failure(expected.relativePath + " failed hash verification.");
    return Result<void>::success();
}

Result<void> IntegrityManager::verifyArchiveParts(const fs::path& folder,
                                                  const ArchiveManifest& manifest,
                                                  bool deepHashVerification) const {
    for (const auto& part : manifest.parts) {
        const auto path = folder / part.fileName;
        if (!fs::exists(path)) return Result<void>::failure(part.fileName + " is missing. Installation cannot continue.");
        if (fs::file_size(path) != part.size) return Result<void>::failure(part.fileName + " is corrupted or incomplete.");
        if (!deepHashVerification) continue;
        if (crc32File(path) != part.crc32) return Result<void>::failure(part.fileName + " failed CRC verification.");
        if (hash64File(path) != part.hash64) return Result<void>::failure(part.fileName + " failed hash verification.");
    }
    return Result<void>::success();
}

Result<ArchiveManifest> ArchiveManager::createArchive(const RepackProject& project, IProgressSink& progress) {
    if (!fs::exists(project.sourceFolder)) return Result<ArchiveManifest>::failure("Source folder does not exist.");
    fs::create_directories(project.outputFolder);

    ArchiveManifest manifest;
    manifest.gameName = project.gameName;
    manifest.version = project.version;
    manifest.publisher = project.publisher;
    manifest.installerTitle = project.branding.installerTitle.empty() ? project.gameName + " Setup" : project.branding.installerTitle;
    manifest.installerDescription = project.branding.installerDescription;
    manifest.accentColor = project.branding.accentColor;
    manifest.algorithm = project.compression.algorithm;
    manifest.profile = toString(project.compression.profile);
    manifest.splitSize = project.splitSize;

    const auto files = scanFiles(project.sourceFolder);
    for (const auto& file : files) manifest.installSize += fs::file_size(file);

    // Create compression codec from project settings
    auto codec = createCodec(project.compression.algorithm,
                             project.compression.level);

    IntegrityManager integrity;
    uint32_t partIndex = 1;
    uint64_t globalPayloadOffset = 0;
    uint64_t currentPayloadBytes = 0;
    fs::path currentPartPath = project.outputFolder / ("data" + std::to_string(partIndex) + ".bin");
    std::fstream part(currentPartPath, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!part) return Result<ArchiveManifest>::failure("Could not create " + currentPartPath.string());
    PartHeader emptyHeader{};
    part.write(reinterpret_cast<const char*>(&emptyHeader), sizeof(emptyHeader));

    uint64_t processed = 0;
    for (const auto& sourceFile : files) {
        FileEntry entry;
        entry.relativePath = normalizePath(fs::relative(sourceFile, project.sourceFolder));
        entry.originalSize = fs::file_size(sourceFile);
        entry.archiveOffset = globalPayloadOffset;
        entry.crc32 = integrity.crc32File(sourceFile);
        entry.hash64 = integrity.hash64File(sourceFile);

        std::ifstream in(sourceFile, std::ios::binary);
        if (!in) return Result<ArchiveManifest>::failure("Could not read " + sourceFile.string());

        // Split the file into chunks, compress each, write [compSize][origSize][data]
        uint64_t remaining = entry.originalSize;
        while (remaining > 0) {
            // Start a new BIN part if current one is full
            if (currentPayloadBytes >= project.splitSize && currentPayloadBytes > 0) {
                writeHeader(part, partIndex, currentPayloadBytes);
                part.close();
                ++partIndex;
                currentPayloadBytes = 0;
                currentPartPath = project.outputFolder / ("data" + std::to_string(partIndex) + ".bin");
                part.open(currentPartPath, std::ios::binary | std::ios::out | std::ios::trunc);
                if (!part) return Result<ArchiveManifest>::failure("Could not create " + currentPartPath.string());
                part.write(reinterpret_cast<const char*>(&emptyHeader), sizeof(emptyHeader));
            }

            const uint64_t chunkSize = std::min(kCompressionChunkSize, remaining);
            auto rawChunk = readBlock(in, static_cast<size_t>(chunkSize));
            if (rawChunk.empty()) return Result<ArchiveManifest>::failure("Unexpected read failure: " + sourceFile.string());

            // Compress the chunk
            auto compressedChunk = codec->compress(rawChunk.data(), rawChunk.size());
            if (compressedChunk.empty() && !rawChunk.empty())
                return Result<ArchiveManifest>::failure("Compression failed for chunk of " + sourceFile.string());

            // Write chunk header: [compressedSize:uint32_t][originalSize:uint32_t]
            const uint32_t compSize32 = static_cast<uint32_t>(compressedChunk.size());
            const uint32_t origSize32 = static_cast<uint32_t>(rawChunk.size());
            part.write(reinterpret_cast<const char*>(&compSize32), sizeof(compSize32));
            part.write(reinterpret_cast<const char*>(&origSize32), sizeof(origSize32));

            // Write compressed payload
            part.write(reinterpret_cast<const char*>(compressedChunk.data()),
                       static_cast<std::streamsize>(compressedChunk.size()));

            const uint64_t bytesWritten = sizeof(compSize32) + sizeof(origSize32) + compressedChunk.size();

            remaining -= rawChunk.size();
            processed += rawChunk.size();           // track original bytes for progress
            currentPayloadBytes += bytesWritten;
            globalPayloadOffset += bytesWritten;

            ProgressInfo info;
            info.processedBytes = processed;
            info.totalBytes = manifest.installSize;
            info.percent = manifest.installSize ? (100.0 * static_cast<double>(processed) / static_cast<double>(manifest.installSize)) : 100.0;
            info.currentFile = entry.relativePath;
            info.message = codec->name() + std::string(" compress");
            progress.onProgress(info);
        }

        entry.storedSize = globalPayloadOffset - entry.archiveOffset;
        manifest.files.push_back(entry);
    }

    writeHeader(part, partIndex, currentPayloadBytes);
    part.close();

    for (uint32_t i = 1; i <= partIndex; ++i) {
        ArchivePart ap;
        ap.index = i;
        ap.fileName = "data" + std::to_string(i) + ".bin";
        const auto partPath = project.outputFolder / ap.fileName;
        ap.size = fs::file_size(partPath);
        ap.crc32 = integrity.crc32File(partPath);
        ap.hash64 = integrity.hash64File(partPath);
        manifest.compressedSize += ap.size;
        manifest.parts.push_back(ap);
    }

    writeTextFile(project.outputFolder / "manifest.rpk", serializeManifest(manifest));
    return Result<ArchiveManifest>::success(manifest);
}

Result<void> ArchiveManager::extractArchive(const fs::path& archiveFolder,
                                            const fs::path& targetFolder,
                                            const ArchiveManifest& manifest,
                                            IProgressSink& progress,
                                            bool verifyAfterExtract) {
    IntegrityManager integrity;
    auto partCheck = integrity.verifyArchiveParts(archiveFolder, manifest, false);
    if (!partCheck.ok()) return partCheck;
    fs::create_directories(targetFolder);

    ResumeState resume;
    const auto resumePath = targetFolder / ".repack-resume";
    resume.load(resumePath);

    uint64_t processed = 0;
    for (const auto& file : manifest.files) {
        processed += resume.isComplete(file.relativePath) ? file.originalSize : 0;
    }

    // Create decompression codec from manifest algorithm
    int level = 0;
    {
        // Infer default level from profile string
        const auto& p = manifest.profile;
        if (p == "fast")       level = 1;
        else if (p == "balanced") level = 5;
        else if (p == "maximum")  level = 9;
        else if (p == "extreme")  level = 9;
    }
    auto codec = createCodec(manifest.algorithm, level);

    size_t partCursor = 0;
    uint64_t partPayloadStart = 0;

    for (const auto& file : manifest.files) {
        if (resume.isComplete(file.relativePath)) continue;

        // Position the part cursor at the start of this file's compressed blob
        while (partCursor < manifest.parts.size()) {
            const uint64_t payloadSize = manifest.parts[partCursor].size - kHeaderSize;
            if (file.archiveOffset < partPayloadStart + payloadSize) break;
            partPayloadStart += payloadSize;
            ++partCursor;
        }
        if (partCursor >= manifest.parts.size())
            return Result<void>::failure("Archive offset outside available BIN parts.");

        auto outPath = targetFolder / fs::path(file.relativePath);
        fs::create_directories(outPath.parent_path());
        std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
        if (!out) return Result<void>::failure("Cannot write " + outPath.string());

        // Read the file's compressed payload into a buffer
        // (the compressed blob may span multiple parts)
        std::vector<unsigned char> compressedBuf;
        compressedBuf.reserve(static_cast<size_t>(file.storedSize));
        {
            uint64_t remaining = file.storedSize;
            uint64_t currentOffset = file.archiveOffset;
            size_t localPartCursor = partCursor;
            uint64_t localPartPayloadStart = partPayloadStart;

            while (remaining > 0) {
                const auto& partMeta = manifest.parts[localPartCursor];
                std::ifstream part(archiveFolder / partMeta.fileName, std::ios::binary);
                if (!part) return Result<void>::failure(partMeta.fileName + " is missing. Installation cannot continue.");
                auto header = readHeader(part);
                if (!header.ok()) return Result<void>::failure(partMeta.fileName + ": " + header.error());

                const uint64_t insidePart = currentOffset - localPartPayloadStart;
                const uint64_t available = header.value().payloadBytes - insidePart;
                const uint64_t toCopy = std::min<uint64_t>(remaining, available);
                part.seekg(static_cast<std::streamoff>(kHeaderSize + insidePart));

                uint64_t copied = 0;
                while (copied < toCopy) {
                    auto block = readBlock(part, static_cast<size_t>(std::min<uint64_t>(4ull * 1024ull * 1024ull, toCopy - copied)));
                    if (block.empty()) return Result<void>::failure("Unexpected end of " + partMeta.fileName);
                    compressedBuf.insert(compressedBuf.end(), block.begin(), block.end());
                    copied += block.size();
                    remaining -= block.size();
                    currentOffset += block.size();
                }

                if (remaining > 0) {
                    localPartPayloadStart += header.value().payloadBytes;
                    ++localPartCursor;
                    if (localPartCursor >= manifest.parts.size())
                        return Result<void>::failure("File spans beyond available BIN parts.");
                }
            }
        }

        // Process the compressed buffer chunk by chunk
        {
            uint64_t bufPos = 0;
            while (bufPos + 8 <= compressedBuf.size()) {
                // Read chunk header
                uint32_t compSize, origSize;
                std::memcpy(&compSize, compressedBuf.data() + bufPos, sizeof(compSize));
                std::memcpy(&origSize, compressedBuf.data() + bufPos + 4, sizeof(origSize));
                bufPos += 8;

                if (bufPos + compSize > compressedBuf.size())
                    return Result<void>::failure("Truncated chunk in " + file.relativePath);

                // Decompress
                auto decompressed = codec->decompress(
                    compressedBuf.data() + bufPos, compSize, origSize);
                bufPos += compSize;

                if (decompressed.empty())
                    return Result<void>::failure("Decompression failed for " + file.relativePath);

                out.write(reinterpret_cast<const char*>(decompressed.data()),
                          static_cast<std::streamsize>(decompressed.size()));

                // Track progress using original (decompressed) bytes
                processed += decompressed.size();
                ProgressInfo info;
                info.processedBytes = processed;
                info.totalBytes = manifest.installSize;
                info.percent = manifest.installSize ? (100.0 * static_cast<double>(processed) / static_cast<double>(manifest.installSize)) : 100.0;
                info.currentFile = file.relativePath;
                info.message = codec->name() + std::string(" decompress");
                progress.onProgress(info);
            }

            // Sanity check: we should have consumed all stored bytes
            if (bufPos != compressedBuf.size())
                return Result<void>::failure("Chunk size mismatch for " + file.relativePath);
        }

        out.close();

        if (verifyAfterExtract) {
            auto check = integrity.verifyFile(outPath, file);
            if (!check.ok()) return check;
        }
        resume.markComplete(file.relativePath);
        resume.save(resumePath);
    }

    fs::remove(resumePath);
    return Result<void>::success();
}

Result<void> InstallerGenerator::generate(const fs::path& templateExe,
                                          const fs::path& outputExe,
                                          const ArchiveManifest& manifest) {
    if (!fs::exists(templateExe)) return Result<void>::failure("Installer template not found: " + templateExe.string());
    fs::copy_file(templateExe, outputExe, fs::copy_options::overwrite_existing);
    writeTextFile(outputExe.string() + ".theme.rpk", "accent=" + manifest.accentColor + "\n"
                                               "title=" + manifest.installerTitle + "\n"
                                               "description=" + manifest.installerDescription + "\n");
    return Result<void>::success();
}

bool ResumeState::load(const fs::path& path) {
    completed_.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) completed_.insert(line);
    }
    return true;
}

bool ResumeState::save(const fs::path& path) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    for (const auto& item : completed_) out << item << "\n";
    return true;
}

void ResumeState::markComplete(const std::string& relativePath) {
    completed_.insert(relativePath);
}

bool ResumeState::isComplete(const std::string& relativePath) const {
    return completed_.contains(relativePath);
}

HardwareProfile HardwareProfiler::query() const {
    HardwareProfile p;
    p.logicalCores = std::max(1u, std::thread::hardware_concurrency());
#ifdef _WIN32
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        p.totalRam = status.ullTotalPhys;
        p.availableRam = status.ullAvailPhys;
    }
#endif
    return p;
}

uint64_t HardwareProfiler::estimateRamUsage(const std::string& mode) const {
    const auto p = query();
    if (mode == "lowram") return std::min<uint64_t>(p.availableRam / 4, 1024ull * 1024ull * 1024ull);
    if (mode == "highperf") return std::min<uint64_t>(p.availableRam / 2, 8ull * 1024ull * 1024ull * 1024ull);
    return std::min<uint64_t>(p.availableRam / 3, 2ull * 1024ull * 1024ull * 1024ull);
}

uint32_t HardwareProfiler::recommendThreadCount(const std::string& mode) const {
    const auto p = query();
    if (mode == "lowram") return std::max<uint32_t>(1, p.logicalCores / 2);
    if (mode == "highperf") return p.logicalCores;
    return std::max<uint32_t>(1, p.logicalCores - 1);
}

uint64_t DiskSpaceChecker::availableBytes(const fs::path& path) const {
    std::error_code ec;
    auto info = fs::space(path, ec);
    return ec ? 0 : info.available;
}

bool DiskSpaceChecker::hasEnoughSpace(const fs::path& path, uint64_t bytes) const {
    return availableBytes(path) >= bytes;
}

}

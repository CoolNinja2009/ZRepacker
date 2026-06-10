#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace RepackCore {

/// Abstract compression codec.
/// Each implementation handles one algorithm (store, zstd, lzma2).
class CompressionCodec {
public:
    virtual ~CompressionCodec() = default;

    /// Human-readable name (e.g. "store", "zstd", "lzma2").
    virtual const char* name() const noexcept = 0;

    /// Compress a single chunk of @p size bytes.
    /// Returns the compressed payload (may be empty on failure).
    virtual std::vector<unsigned char> compress(
        const unsigned char* data, size_t size) = 0;

    /// Decompress @p compressedSize bytes back to @p originalSize bytes.
    /// Returns the decompressed payload (may be empty on failure).
    virtual std::vector<unsigned char> decompress(
        const unsigned char* data, size_t compressedSize,
        size_t originalSize) = 0;
};

// -----------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------

/// Create a codec by algorithm name.
/// @param algorithm  One of "store", "zstd", "lzma2".
/// @param level      Compression level (ignored for store).
/// @return A heap-allocated codec instance.
std::unique_ptr<CompressionCodec> createCodec(
    const std::string& algorithm, int level);

} // namespace RepackCore

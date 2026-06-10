#include "RepackCore/Compression/CompressionCodec.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <zstd.h>
#include <lzma.h>

namespace RepackCore {

// ========================================================================
// StoreCodec – raw copy (no compression)
// ========================================================================

class StoreCodec final : public CompressionCodec {
public:
    explicit StoreCodec(int /*level*/) noexcept {}

    const char* name() const noexcept override { return "store"; }

    std::vector<unsigned char> compress(
        const unsigned char* data, size_t size) override
    {
        return std::vector<unsigned char>(data, data + size);
    }

    std::vector<unsigned char> decompress(
        const unsigned char* data, size_t /*compressedSize*/,
        size_t originalSize) override
    {
        return std::vector<unsigned char>(data, data + originalSize);
    }
};

// ========================================================================
// ZstdCodec – Zstandard compression
// ========================================================================

class ZstdCodec final : public CompressionCodec {
    int level_;
public:
    explicit ZstdCodec(int level) noexcept
        : level_(level)
    {
        if (level_ < 1)  level_ = 1;
        if (level_ > 22) level_ = 22;
    }

    const char* name() const noexcept override { return "zstd"; }

    std::vector<unsigned char> compress(
        const unsigned char* data, size_t size) override
    {
        if (size == 0) return {};

        const size_t bound = ZSTD_compressBound(size);
        std::vector<unsigned char> out(bound);
        const size_t rc = ZSTD_compress(out.data(), out.size(),
                                        data, size, level_);
        if (ZSTD_isError(rc))
            return {};
        out.resize(rc);
        return out;
    }

    std::vector<unsigned char> decompress(
        const unsigned char* data, size_t compressedSize,
        size_t originalSize) override
    {
        if (compressedSize == 0) return {};
        std::vector<unsigned char> out(originalSize);
        const size_t rc = ZSTD_decompress(out.data(), out.size(),
                                          data, compressedSize);
        if (ZSTD_isError(rc))
            return {};
        // rc == originalSize (or less for streaming, but one-shot is exact)
        out.resize(rc);
        return out;
    }
};

// ========================================================================
// Lzma2Codec – LZMA2 compression (via liblzma)
// ========================================================================

class Lzma2Codec final : public CompressionCodec {
    uint32_t preset_;
public:
    explicit Lzma2Codec(int level) noexcept
        : preset_(static_cast<uint32_t>(std::clamp(level, 0, 9)))
    {}

    const char* name() const noexcept override { return "lzma2"; }

    std::vector<unsigned char> compress(
        const unsigned char* data, size_t size) override
    {
        if (size == 0) return {};

        // lzma_easy_buffer_encode needs a worst-case output buffer.
        // lzma_stream_buffer_bound gives us that.
        const size_t bound = lzma_stream_buffer_bound(size);
        std::vector<unsigned char> out(bound);
        size_t outPos = 0;

        const lzma_ret ret = lzma_easy_buffer_encode(
            preset_, LZMA_CHECK_CRC32, /* allocator */ nullptr,
            data, size,
            out.data(), &outPos, out.size());

        if (ret != LZMA_OK)
            return {};
        out.resize(outPos);
        return out;
    }

    std::vector<unsigned char> decompress(
        const unsigned char* data, size_t compressedSize,
        size_t originalSize) override
    {
        if (compressedSize == 0) return {};
        std::vector<unsigned char> out(originalSize);
        size_t inPos = 0;
        size_t outPos = 0;
        uint64_t memlimit = UINT64_MAX;

        const lzma_ret ret = lzma_stream_buffer_decode(
            &memlimit, 0, /* allocator */ nullptr,
            data, &inPos, compressedSize,
            out.data(), &outPos, out.size());

        if (ret != LZMA_OK && ret != LZMA_STREAM_END)
            return {};
        out.resize(outPos);
        return out;
    }
};

// ========================================================================
// Factory
// ========================================================================

std::unique_ptr<CompressionCodec> createCodec(
    const std::string& algorithm, int level)
{
    std::string algo = algorithm;
    std::transform(algo.begin(), algo.end(), algo.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (algo == "store")
        return std::make_unique<StoreCodec>(level);
    if (algo == "zstd")
        return std::make_unique<ZstdCodec>(level);
    if (algo == "lzma2")
        return std::make_unique<Lzma2Codec>(level);

    // Fallback to store for unknown algorithms
    return std::make_unique<StoreCodec>(level);
}

} // namespace RepackCore

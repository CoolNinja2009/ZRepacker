#pragma once

#include <cstdint>
#include <string>

namespace RepackCore {

struct ProgressInfo {
    double percent = 0.0;
    uint64_t processedBytes = 0;
    uint64_t totalBytes = 0;
    uint64_t bytesPerSecond = 0;
    std::string currentFile;
    std::string message;
};

class IProgressSink {
public:
    virtual ~IProgressSink() = default;
    virtual void onProgress(const ProgressInfo& info) = 0;
};

class NullProgressSink final : public IProgressSink {
public:
    void onProgress(const ProgressInfo&) override {}
};

}


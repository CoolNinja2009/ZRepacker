#pragma once

#include <filesystem>
#include <set>
#include <string>

namespace RepackCore {

class ResumeState {
public:
    bool load(const std::filesystem::path& path);
    bool save(const std::filesystem::path& path) const;
    void markComplete(const std::string& relativePath);
    bool isComplete(const std::string& relativePath) const;

private:
    std::set<std::string> completed_;
};

}


#pragma once

#include <filesystem>
#include <string>
#include "RepackCore/Compression/CompressionProfile.h"

namespace RepackCore {

struct BrandingConfig {
    std::string logoPath;
    std::string bannerPath;
    std::string backgroundPath;
    std::string accentColor = "#4F8CFF";
    std::string installerTitle;
    std::string installerDescription;
    std::string publisherName;
    std::string version;
};

struct RepackProject {
    std::string gameName = "Untitled Game";
    std::string version = "1.0.0";
    std::string publisher = "Unknown Publisher";
    std::filesystem::path sourceFolder;
    std::filesystem::path outputFolder;
    uint64_t splitSize = 4ull * 1024ull * 1024ull * 1024ull;
    CompressionProfileSettings compression = defaultsForProfile(CompressionProfile::Balanced);
    BrandingConfig branding;
    bool integrityVerification = true;
};

}


#pragma once

#include <filesystem>
#include "RepackCore/Archive/ArchiveManifest.h"
#include "RepackCore/Common/Result.h"

namespace RepackCore {

class InstallerGenerator {
public:
    Result<void> generate(const std::filesystem::path& templateExe,
                          const std::filesystem::path& outputExe,
                          const ArchiveManifest& manifest);
};

}


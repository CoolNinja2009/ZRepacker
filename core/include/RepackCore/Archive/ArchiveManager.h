#pragma once

#include <filesystem>
#include "RepackCore/Archive/ArchiveManifest.h"
#include "RepackCore/Common/Progress.h"
#include "RepackCore/Common/Result.h"
#include "RepackCore/Project/RepackProject.h"

namespace RepackCore {

class ArchiveManager {
public:
    Result<ArchiveManifest> createArchive(const RepackProject& project, IProgressSink& progress);
    Result<void> extractArchive(const std::filesystem::path& archiveFolder,
                                const std::filesystem::path& targetFolder,
                                const ArchiveManifest& manifest,
                                IProgressSink& progress,
                                bool verifyAfterExtract = true);
};

}


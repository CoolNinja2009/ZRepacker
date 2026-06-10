#include "RepackCore/RepackCore.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

class ConsoleProgress final : public RepackCore::IProgressSink {
public:
    void onProgress(const RepackCore::ProgressInfo& info) override {
        const auto now = std::chrono::steady_clock::now();
        if (now - last_ < std::chrono::milliseconds(150) && info.percent < 100.0) return;
        last_ = now;
        std::cout << "\r" << info.message << " " << static_cast<int>(info.percent)
                  << "%  " << info.currentFile << "                          " << std::flush;
    }

private:
    std::chrono::steady_clock::time_point last_{};
};

static std::map<std::string, std::string> parseArgs(int argc, char** argv) {
    std::map<std::string, std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        if (key.rfind("--", 0) == 0) {
            if (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
                args[key.substr(2)] = argv[++i];
            } else {
                args[key.substr(2)] = "true";
            }
        }
    }
    return args;
}

static std::string readText(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void usage() {
    std::cout
        << "Premium Game Installer Runtime\n\n"
        << "Usage:\n"
        << "  setup.exe --install <folder> [--mode auto|normal|lowram|highperf]\n"
        << "  setup.exe --verify\n";
}

int main(int argc, char** argv) {
    const auto args = parseArgs(argc, argv);
    const bool interactive = argc == 1;
    if (args.contains("help")) {
        usage();
        return 0;
    }

    const auto exePath = fs::absolute(argv[0]);
    const auto baseDir = exePath.parent_path();
    const auto manifestPath = baseDir / "manifest.rpk";
    if (!fs::exists(manifestPath)) {
        std::cerr << "manifest.rpk is missing. Installation cannot continue.\n";
        return 2;
    }

    const auto manifest = RepackCore::parseManifest(readText(manifestPath));
    std::cout << "=============================================\n";
    std::cout << manifest.installerTitle << "\n";
    std::cout << "Game: " << manifest.gameName << " " << manifest.version << "\n";
    std::cout << "Publisher: " << manifest.publisher << "\n";
    std::cout << "Install size: " << manifest.installSize << " bytes\n";
    std::cout << "Required disk space: " << static_cast<uint64_t>(manifest.installSize * 1.05) << " bytes\n";
    if (!manifest.installerDescription.empty()) std::cout << manifest.installerDescription << "\n";
    std::cout << "=============================================\n\n";

    RepackCore::IntegrityManager integrity;

    if (args.contains("verify")) {
        std::cout << "Deep-verifying BIN files. Large repacks can take a while...\n";
        auto partCheck = integrity.verifyArchiveParts(baseDir, manifest, true);
        if (!partCheck.ok()) {
            std::cerr << partCheck.error() << "\n";
            return 3;
        }
        std::cout << "All BIN files verified successfully.\n";
        return 0;
    }

    fs::path target;
    if (args.contains("install")) {
        target = args.at("install");
    } else if (interactive) {
        const auto defaultTarget = baseDir / manifest.gameName;
        std::cout << "Install directory [" << defaultTarget.string() << "]: ";
        std::string input;
        std::getline(std::cin, input);
        target = input.empty() ? defaultTarget : fs::path(input);
    } else {
        usage();
        return 1;
    }

    std::cout << "Checking BIN files...\n";
    auto partCheck = integrity.verifyArchiveParts(baseDir, manifest, false);
    if (!partCheck.ok()) {
        std::cerr << partCheck.error() << "\n";
        if (interactive) {
            std::cout << "Press Enter to exit.";
            std::string ignored;
            std::getline(std::cin, ignored);
        }
        return 3;
    }

    RepackCore::DiskSpaceChecker disk;
    if (!disk.hasEnoughSpace(target.has_parent_path() ? target.parent_path() : fs::current_path(), manifest.installSize)) {
        std::cerr << "Not enough disk space for installation.\n";
        if (interactive) {
            std::cout << "Press Enter to exit.";
            std::string ignored;
            std::getline(std::cin, ignored);
        }
        return 4;
    }

    const std::string mode = args.contains("mode") ? args.at("mode") : "auto";
    RepackCore::HardwareProfiler hardware;
    const auto profile = hardware.query();
    std::cout << "Mode: " << mode << "\n";
    std::cout << "CPU threads: " << hardware.recommendThreadCount(mode) << " / " << profile.logicalCores << "\n";
    std::cout << "Estimated RAM usage: " << hardware.estimateRamUsage(mode) << " bytes\n\n";

    ConsoleProgress progress;
    RepackCore::ArchiveManager archive;
    auto result = archive.extractArchive(baseDir, target, manifest, progress, true);
    std::cout << "\n";
    if (!result.ok()) {
        std::cerr << "Installation failed: " << result.error() << "\n";
        if (interactive) {
            std::cout << "Press Enter to exit.";
            std::string ignored;
            std::getline(std::cin, ignored);
        }
        return 5;
    }

    std::cout << "Installation complete: " << fs::absolute(target).string() << "\n";
    if (interactive) {
        std::cout << "Press Enter to exit.";
        std::string ignored;
        std::getline(std::cin, ignored);
    }
    return 0;
}

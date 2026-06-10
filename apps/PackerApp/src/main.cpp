#include "RepackCore/RepackCore.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>

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

static void usage() {
    std::cout
        << "RepackSuite Developer Packer\n\n"
        << "Usage:\n"
        << "  packer.exe --source <folder> --output <folder> --game <name> [options]\n\n"
        << "Options:\n"
        << "  --version <version>\n"
        << "  --publisher <name>\n"
        << "  --profile fast|balanced|maximum|extreme\n"
        << "  --algorithm store|zstd|lzma2\n"
        << "  --split <bytes>\n"
        << "  --title <installer title>\n"
        << "  --description <installer description>\n"
        << "  --accent <#RRGGBB>\n";
}

int main(int argc, char** argv) {
    auto args = parseArgs(argc, argv);
    if (args.contains("help") || !args.contains("source") || !args.contains("output") || !args.contains("game")) {
        usage();
        return args.contains("help") ? 0 : 1;
    }

    RepackCore::RepackProject project;
    project.sourceFolder = args["source"];
    project.outputFolder = args["output"];
    project.gameName = args["game"];
    if (args.contains("version")) project.version = args["version"];
    if (args.contains("publisher")) project.publisher = args["publisher"];
    if (args.contains("split")) project.splitSize = std::stoull(args["split"]);
    if (args.contains("profile")) {
        project.compression = RepackCore::defaultsForProfile(RepackCore::parseCompressionProfile(args["profile"]));
    }
    if (args.contains("algorithm")) project.compression.algorithm = args["algorithm"];
    if (args.contains("title")) project.branding.installerTitle = args["title"];
    if (args.contains("description")) project.branding.installerDescription = args["description"];
    if (args.contains("accent")) project.branding.accentColor = args["accent"];

    std::cout << "RepackSuite Developer Packer\n";
    std::cout << "Game: " << project.gameName << " " << project.version << "\n";
    std::cout << "Source: " << fs::absolute(project.sourceFolder).string() << "\n";
    std::cout << "Output: " << fs::absolute(project.outputFolder).string() << "\n";
    std::cout << "Profile: " << RepackCore::toString(project.compression.profile)
              << " / " << project.compression.algorithm << "\n\n";

    ConsoleProgress progress;
    RepackCore::ArchiveManager archive;
    auto result = archive.createArchive(project, progress);
    std::cout << "\n";
    if (!result.ok()) {
        std::cerr << "Build failed: " << result.error() << "\n";
        return 2;
    }

    const auto templateExe = fs::current_path() / "setup-template.exe";
    const auto fallbackTemplate = fs::current_path() / "build" / "setup-template.exe";
    const auto selectedTemplate = fs::exists(templateExe) ? templateExe : fallbackTemplate;
    RepackCore::InstallerGenerator generator;
    auto gen = generator.generate(selectedTemplate, project.outputFolder / "setup.exe", result.value());
    if (!gen.ok()) {
        std::cerr << "Installer generation failed: " << gen.error() << "\n";
        return 3;
    }

    std::cout << "Build complete.\n";
    std::cout << "Installer: " << fs::absolute(project.outputFolder / "setup.exe").string() << "\n";
    std::cout << "Manifest:  " << fs::absolute(project.outputFolder / "manifest.rpk").string() << "\n";
    std::cout << "Parts:     " << result.value().parts.size() << "\n";
    std::cout << "Size:      " << result.value().compressedSize << " bytes\n";
    return 0;
}


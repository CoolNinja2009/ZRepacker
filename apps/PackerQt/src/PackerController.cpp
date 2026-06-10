#include "PackerController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <filesystem>

namespace fs = std::filesystem;

PackerController::PackerController(QObject* parent) : QObject(parent) {}

PackerController::~PackerController() {
    if (worker_) {
        worker_->quit();
        worker_->wait();
    }
}

void PackerController::setGameName(const QString& value) { if (gameName_ != value) { gameName_ = value; emit gameNameChanged(); } }
void PackerController::setVersion(const QString& value) { if (version_ != value) { version_ = value; emit versionChanged(); } }
void PackerController::setPublisher(const QString& value) { if (publisher_ != value) { publisher_ = value; emit publisherChanged(); } }
void PackerController::setSourceFolder(const QString& value) { if (sourceFolder_ != value) { sourceFolder_ = value; emit sourceFolderChanged(); refreshEstimate(); } }
void PackerController::setOutputFolder(const QString& value) { if (outputFolder_ != value) { outputFolder_ = value; emit outputFolderChanged(); } }
void PackerController::setProfile(const QString& value) { if (profile_ != value) { profile_ = value; emit profileChanged(); refreshEstimate(); } }
void PackerController::setAccentColor(const QString& value) { if (accentColor_ != value) { accentColor_ = value; emit accentColorChanged(); } }
void PackerController::setDescription(const QString& value) { if (description_ != value) { description_ = value; emit descriptionChanged(); } }

void PackerController::appendLog(const QString& line) {
    log_ += line + "\n";
    emit logChanged();
}

RepackCore::RepackProject PackerController::makeProject() const {
    RepackCore::RepackProject p;
    p.gameName = gameName_.toStdString();
    p.version = version_.toStdString();
    p.publisher = publisher_.toStdString();
    p.sourceFolder = sourceFolder_.toStdWString();
    p.outputFolder = outputFolder_.toStdWString();
    p.compression = RepackCore::defaultsForProfile(RepackCore::parseCompressionProfile(profile_.toStdString()));
    p.branding.installerTitle = (gameName_ + " Setup").toStdString();
    p.branding.installerDescription = description_.toStdString();
    p.branding.accentColor = accentColor_.toStdString();
    return p;
}

void PackerController::refreshEstimate() {
    if (sourceFolder_.isEmpty() || !QFileInfo::exists(sourceFolder_)) {
        estimatedSize_ = "Select source";
        emit metricsChanged();
        return;
    }

    uint64_t total = 0;
    for (const auto& entry : fs::recursive_directory_iterator(sourceFolder_.toStdWString())) {
        if (entry.is_regular_file()) total += entry.file_size();
    }
    const double gb = static_cast<double>(total) / 1024.0 / 1024.0 / 1024.0;
    estimatedSize_ = QString::number(gb, 'f', 2) + " GB source";
    emit metricsChanged();
}

void PackerController::startBuild() {
    if (building_) return;
    if (sourceFolder_.isEmpty() || outputFolder_.isEmpty()) {
        appendLog("Select source and output folders first.");
        return;
    }

    building_ = true;
    progress_ = 0.0;
    currentFile_.clear();
    log_.clear();
    emit buildingChanged();
    emit progressChanged();
    emit currentFileChanged();
    emit logChanged();
    appendLog("Build started.");

    const auto project = makeProject();
    worker_ = QThread::create([this, project]() {
        RepackCore::ArchiveManager archive;
        auto result = archive.createArchive(project, *this);
        if (!result.ok()) {
            QMetaObject::invokeMethod(this, [this, error = QString::fromStdString(result.error())]() {
                appendLog("Build failed: " + error);
                building_ = false;
                emit buildingChanged();
            }, Qt::QueuedConnection);
            return;
        }

        fs::path appDir = QCoreApplication::applicationDirPath().toStdWString();
        fs::path templateExe = appDir / "RepackInstallerGui.exe";
        if (!fs::exists(templateExe)) templateExe = appDir / "setup-template.exe";
        if (!fs::exists(templateExe)) templateExe = fs::current_path() / "RepackInstallerGui.exe";
        if (!fs::exists(templateExe)) templateExe = fs::current_path() / "setup-template.exe";
        if (!fs::exists(templateExe)) templateExe = fs::current_path() / "build" / "setup-template.exe";

        RepackCore::InstallerGenerator generator;
        auto gen = generator.generate(templateExe, project.outputFolder / "setup.exe", result.value());
        QMetaObject::invokeMethod(this, [this, genOk = gen.ok(), genError = QString::fromStdString(gen.error())]() {
            if (genOk) {
                progress_ = 100.0;
                appendLog("Build complete.");
            } else {
                appendLog("Installer generation failed: " + genError);
            }
            building_ = false;
            emit progressChanged();
            emit buildingChanged();
        }, Qt::QueuedConnection);
    });
    connect(worker_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(worker_, &QThread::finished, this, [this]() { worker_ = nullptr; });
    worker_->start();
}

void PackerController::onProgress(const RepackCore::ProgressInfo& info) {
    QMetaObject::invokeMethod(this, [this, info]() {
        progress_ = info.percent;
        currentFile_ = QString::fromStdString(info.currentFile);
        emit progressChanged();
        emit currentFileChanged();
    }, Qt::QueuedConnection);
}

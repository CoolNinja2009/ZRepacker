#include "InstallerController.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMetaObject>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

static std::string readText(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

InstallerController::InstallerController(QObject* parent) : QObject(parent) {
    loadManifest();
}

InstallerController::~InstallerController() {
    if (worker_) {
        worker_->quit();
        worker_->wait();
    }
}

void InstallerController::loadManifest() {
    baseDir_ = QCoreApplication::applicationDirPath();
    const fs::path manifestPath = fs::path(baseDir_.toStdWString()) / "manifest.rpk";
    if (!fs::exists(manifestPath)) {
        setStatus("manifest.rpk is missing. Installation cannot continue.");
        return;
    }

    manifest_ = RepackCore::parseManifest(readText(manifestPath));
    installFolder_ = QString::fromStdWString((fs::path(baseDir_.toStdWString()) / manifest_.gameName).wstring());
    setStatus("Ready");
    emit manifestChanged();
    emit installFolderChanged();
}

QString InstallerController::title() const {
    return QString::fromStdString(manifest_.installerTitle.empty() ? manifest_.gameName + " Setup" : manifest_.installerTitle);
}

QString InstallerController::gameName() const { return QString::fromStdString(manifest_.gameName); }
QString InstallerController::version() const { return QString::fromStdString(manifest_.version); }
QString InstallerController::publisher() const { return QString::fromStdString(manifest_.publisher); }
QString InstallerController::description() const { return QString::fromStdString(manifest_.installerDescription); }
QString InstallerController::installSize() const { return bytesToText(manifest_.installSize); }
QString InstallerController::requiredSpace() const { return bytesToText(static_cast<uint64_t>(manifest_.installSize * 1.05)); }

QString InstallerController::bytesToText(uint64_t bytes) const {
    const double gb = static_cast<double>(bytes) / 1024.0 / 1024.0 / 1024.0;
    if (gb >= 1.0) return QString::number(gb, 'f', 2) + " GB";
    const double mb = static_cast<double>(bytes) / 1024.0 / 1024.0;
    return QString::number(mb, 'f', 1) + " MB";
}

QString InstallerController::ramEstimate() const {
    RepackCore::HardwareProfiler profiler;
    return bytesToText(profiler.estimateRamUsage(performanceMode_.toStdString()));
}

void InstallerController::setInstallFolder(const QString& value) {
    if (installFolder_ != value) {
        installFolder_ = value;
        emit installFolderChanged();
    }
}

void InstallerController::setPerformanceMode(const QString& value) {
    if (performanceMode_ != value) {
        performanceMode_ = value;
        emit performanceModeChanged();
    }
}

void InstallerController::setStatus(const QString& value) {
    if (status_ != value) {
        status_ = value;
        emit statusChanged();
    }
}

void InstallerController::verifyBins() {
    if (installing_) return;
    setStatus("Deep-verifying BIN files...");
    worker_ = QThread::create([this]() {
        RepackCore::IntegrityManager integrity;
        auto result = integrity.verifyArchiveParts(baseDir_.toStdWString(), manifest_, true);
        QMetaObject::invokeMethod(this, [this, ok = result.ok(), error = QString::fromStdString(result.error())]() {
            setStatus(ok ? "All BIN files verified." : error);
        }, Qt::QueuedConnection);
    });
    connect(worker_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(worker_, &QThread::finished, this, [this]() { worker_ = nullptr; });
    worker_->start();
}

void InstallerController::startInstall() {
    if (installing_) return;
    if (installFolder_.isEmpty()) {
        setStatus("Choose an install folder.");
        return;
    }

    installing_ = true;
    finished_ = false;
    progress_ = 0.0;
    currentFile_.clear();
    setStatus("Checking BIN files...");
    emit installingChanged();
    emit finishedChanged();
    emit progressChanged();
    emit currentFileChanged();

    const auto target = installFolder_;
    worker_ = QThread::create([this, target]() {
        RepackCore::IntegrityManager integrity;
        auto partCheck = integrity.verifyArchiveParts(baseDir_.toStdWString(), manifest_, false);
        if (!partCheck.ok()) {
            QMetaObject::invokeMethod(this, [this, error = QString::fromStdString(partCheck.error())]() {
                setStatus(error);
                installing_ = false;
                emit installingChanged();
            }, Qt::QueuedConnection);
            return;
        }

        RepackCore::ArchiveManager archive;
        auto result = archive.extractArchive(baseDir_.toStdWString(), target.toStdWString(), manifest_, *this, true);
        QMetaObject::invokeMethod(this, [this, ok = result.ok(), error = QString::fromStdString(result.error())]() {
            installing_ = false;
            finished_ = ok;
            progress_ = ok ? 100.0 : progress_;
            setStatus(ok ? "Installation complete." : "Installation failed: " + error);
            emit installingChanged();
            emit finishedChanged();
            emit progressChanged();
        }, Qt::QueuedConnection);
    });
    connect(worker_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(worker_, &QThread::finished, this, [this]() { worker_ = nullptr; });
    worker_->start();
}

void InstallerController::onProgress(const RepackCore::ProgressInfo& info) {
    QMetaObject::invokeMethod(this, [this, info]() {
        progress_ = info.percent;
        currentFile_ = QString::fromStdString(info.currentFile);
        setStatus(QString::fromStdString(info.message));
        emit progressChanged();
        emit currentFileChanged();
    }, Qt::QueuedConnection);
}


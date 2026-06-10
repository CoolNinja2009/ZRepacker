#pragma once

#include <QObject>
#include <QThread>
#include "RepackCore/RepackCore.h"

class InstallerController : public QObject, public RepackCore::IProgressSink {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY manifestChanged)
    Q_PROPERTY(QString gameName READ gameName NOTIFY manifestChanged)
    Q_PROPERTY(QString version READ version NOTIFY manifestChanged)
    Q_PROPERTY(QString publisher READ publisher NOTIFY manifestChanged)
    Q_PROPERTY(QString description READ description NOTIFY manifestChanged)
    Q_PROPERTY(QString installSize READ installSize NOTIFY manifestChanged)
    Q_PROPERTY(QString requiredSpace READ requiredSpace NOTIFY manifestChanged)
    Q_PROPERTY(QString installFolder READ installFolder WRITE setInstallFolder NOTIFY installFolderChanged)
    Q_PROPERTY(QString performanceMode READ performanceMode WRITE setPerformanceMode NOTIFY performanceModeChanged)
    Q_PROPERTY(QString ramEstimate READ ramEstimate NOTIFY performanceModeChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool installing READ installing NOTIFY installingChanged)
    Q_PROPERTY(bool finished READ finished NOTIFY finishedChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit InstallerController(QObject* parent = nullptr);
    ~InstallerController() override;

    QString title() const;
    QString gameName() const;
    QString version() const;
    QString publisher() const;
    QString description() const;
    QString installSize() const;
    QString requiredSpace() const;
    QString installFolder() const { return installFolder_; }
    QString performanceMode() const { return performanceMode_; }
    QString ramEstimate() const;
    double progress() const { return progress_; }
    bool installing() const { return installing_; }
    bool finished() const { return finished_; }
    QString currentFile() const { return currentFile_; }
    QString status() const { return status_; }

    void setInstallFolder(const QString& value);
    void setPerformanceMode(const QString& value);

    Q_INVOKABLE void startInstall();
    Q_INVOKABLE void verifyBins();

    void onProgress(const RepackCore::ProgressInfo& info) override;

signals:
    void manifestChanged();
    void installFolderChanged();
    void performanceModeChanged();
    void progressChanged();
    void installingChanged();
    void finishedChanged();
    void currentFileChanged();
    void statusChanged();

private:
    QString bytesToText(uint64_t bytes) const;
    void setStatus(const QString& value);
    void loadManifest();

    RepackCore::ArchiveManifest manifest_;
    QString baseDir_;
    QString installFolder_;
    QString performanceMode_ = "auto";
    double progress_ = 0.0;
    bool installing_ = false;
    bool finished_ = false;
    QString currentFile_;
    QString status_;
    QThread* worker_ = nullptr;
};

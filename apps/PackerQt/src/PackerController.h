#pragma once

#include <QObject>
#include <QThread>
#include "RepackCore/RepackCore.h"

class PackerController : public QObject, public RepackCore::IProgressSink {
    Q_OBJECT
    Q_PROPERTY(QString gameName READ gameName WRITE setGameName NOTIFY gameNameChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString publisher READ publisher WRITE setPublisher NOTIFY publisherChanged)
    Q_PROPERTY(QString sourceFolder READ sourceFolder WRITE setSourceFolder NOTIFY sourceFolderChanged)
    Q_PROPERTY(QString outputFolder READ outputFolder WRITE setOutputFolder NOTIFY outputFolderChanged)
    Q_PROPERTY(QString profile READ profile WRITE setProfile NOTIFY profileChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool building READ building NOTIFY buildingChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged)
    Q_PROPERTY(QString log READ log NOTIFY logChanged)
    Q_PROPERTY(QString estimatedSize READ estimatedSize NOTIFY metricsChanged)

public:
    explicit PackerController(QObject* parent = nullptr);
    ~PackerController() override;

    QString gameName() const { return gameName_; }
    QString version() const { return version_; }
    QString publisher() const { return publisher_; }
    QString sourceFolder() const { return sourceFolder_; }
    QString outputFolder() const { return outputFolder_; }
    QString profile() const { return profile_; }
    QString accentColor() const { return accentColor_; }
    QString description() const { return description_; }
    double progress() const { return progress_; }
    bool building() const { return building_; }
    QString currentFile() const { return currentFile_; }
    QString log() const { return log_; }
    QString estimatedSize() const { return estimatedSize_; }

    void setGameName(const QString& value);
    void setVersion(const QString& value);
    void setPublisher(const QString& value);
    void setSourceFolder(const QString& value);
    void setOutputFolder(const QString& value);
    void setProfile(const QString& value);
    void setAccentColor(const QString& value);
    void setDescription(const QString& value);

    Q_INVOKABLE void startBuild();
    Q_INVOKABLE void refreshEstimate();

    void onProgress(const RepackCore::ProgressInfo& info) override;

signals:
    void gameNameChanged();
    void versionChanged();
    void publisherChanged();
    void sourceFolderChanged();
    void outputFolderChanged();
    void profileChanged();
    void accentColorChanged();
    void descriptionChanged();
    void progressChanged();
    void buildingChanged();
    void currentFileChanged();
    void logChanged();
    void metricsChanged();

private:
    void appendLog(const QString& line);
    RepackCore::RepackProject makeProject() const;

    QString gameName_ = "After 2025";
    QString version_ = "1.0.0";
    QString publisher_ = "Studio";
    QString sourceFolder_;
    QString outputFolder_;
    QString profile_ = "balanced";
    QString accentColor_ = "#4F8CFF";
    QString description_ = "Premium repack installer";
    double progress_ = 0.0;
    bool building_ = false;
    QString currentFile_;
    QString log_;
    QString estimatedSize_ = "Select source";
    QThread* worker_ = nullptr;
};

#pragma once

#include <QObject>
#include <QList>
#include <QAction>
#include "FilePluginInterface.h"

class CopyProgressDialog;

class CopyPlugin : public QObject, public FilePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePluginInterface_iid)
    Q_INTERFACES(FilePluginInterface)

public:
    QString name() const override { return "Copy Progress Plugin"; }
    QIcon icon() const override { return QIcon(); }

    void setApplicationAPI(ApplicationAPI *api) override { m_api = api; }

    bool showWidget() const override { return false; }
    bool backgroundPlugin() const override { return true; }

    QWidget* createWidget() override { return nullptr; }

    void initialize() override;
    void shutdown() override;

    void execute(const QStringList &files) override {} // не используется

private slots:
    void onCopyStarted(const QStringList &files, const QString &targetDir);
    void onCopyProgress(int fileIndex, qint64 copied, qint64 total, double speedMB);
    void onCopyFinished();
    void onCopyError(const QString &path);

private:
    ApplicationAPI *m_api = nullptr;
    CopyProgressDialog *m_dialog = nullptr;
};

#pragma once

#include <QObject>
#include "FilePluginInterface.h"

class QLabel;
class QPushButton;

class FileOperationsBtn : public QObject, public FilePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePluginInterface_iid)
    Q_INTERFACES(FilePluginInterface)

public:
    QString name() const override { return "FileOperationsBtn Plugin"; }
    void execute(const QStringList &files) override;

    QIcon icon() const override;
    void setApplicationAPI(ApplicationAPI *api) override { this->m_api = api; }
    bool showWidget() const override { return false; }
    bool backgroundPlugin() const override { return true; }

    QWidget* createWidget() override;

private slots:
    void onCopy();
    void onDelete();
    void onNewFolder();

private:
    ApplicationAPI *m_api = nullptr;

     QPushButton        *copyBtn;
     QPushButton        *deleteBtn;
     QPushButton        *newFolderBtn;
};

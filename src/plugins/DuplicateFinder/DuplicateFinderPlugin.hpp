#pragma once

#include <QObject>
#include <QAction>
#include "FilePluginInterface.h"

class DuplicateFinderDialog;

class DuplicateFinderPlugin : public QObject, public FilePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePluginInterface_iid)
    Q_INTERFACES(FilePluginInterface)

public:
    QString name() const override { return "Duplicate Finder"; }

    void execute(const QStringList &files) override;

    QIcon icon() const override { return QIcon(); }
    void setApplicationAPI(ApplicationAPI *api) override { m_api = api; }

    bool showWidget() const override { return false; }   // dock НЕ нужен
    bool backgroundPlugin() const override { return false; }

    QWidget* createWidget() override { return nullptr; } // dock не создаём

    void initialize() override;
    void shutdown() override;

private:
    ApplicationAPI* m_api = nullptr;
    QList<QAction*> m_actions;
};

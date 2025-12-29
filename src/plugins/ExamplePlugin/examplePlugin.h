#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include <QObject>
#include "FilePluginInterface.h"

class QLabel;
class ExamplePlugin : public QObject, public FilePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePluginInterface_iid)
    Q_INTERFACES(FilePluginInterface)

public:
    QString name() const override { return "Example Plugin"; }
    void execute(const QStringList &files) override;

    QIcon icon() const override;
    void setApplicationAPI(ApplicationAPI *api) override { this->m_api = api; }
    bool showWidget() const override { return true; }
    bool backgroundPlugin() const override { return false; }

    QWidget* createWidget() override;
    void initialize() override;
    void shutdown() override;

private:
    ApplicationAPI *m_api = nullptr;
    QLabel *m_widget = nullptr;
    QList<QAction*> m_actions;

};

#endif // EXAMPLEPLUGIN_H

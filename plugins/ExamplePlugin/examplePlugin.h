#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include <QObject>
#include "FilePluginInterface.h"

class ExamplePlugin : public QObject, public FilePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePluginInterface_iid)
    Q_INTERFACES(FilePluginInterface)

public:
    QString name() const override { return "Example Plugin"; }
    void execute(const QString &filePath) override;
};

#endif // EXAMPLEPLUGIN_H

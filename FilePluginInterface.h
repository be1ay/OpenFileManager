#ifndef FILEPLUGININTERFACE_H
#define FILEPLUGININTERFACE_H

#include <QtPlugin>
#include <QString>

class FilePluginInterface
{
public:
    virtual ~FilePluginInterface() {}


    virtual QString name() const = 0;
    virtual void execute(const QString &filePath) = 0;
};

#define FilePluginInterface_iid "belkin.alexey.FilePluginInterface"
Q_DECLARE_INTERFACE(FilePluginInterface, FilePluginInterface_iid)

#endif // FILEPLUGININTERFACE_H

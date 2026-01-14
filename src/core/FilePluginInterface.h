#ifndef FILEPLUGININTERFACE_H
#define FILEPLUGININTERFACE_H

#include <QtPlugin>
#include <QString>
#include <QIcon>
#include <QWidget>
#include "ApplicationAPI.h"

class BELKINCORE_EXPORT FilePluginInterface {
public:
    FilePluginInterface();
    virtual ~FilePluginInterface();

    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;

    // плагин говорит, нужен ли ему виджет (QDock)
    virtual bool showWidget() const = 0;

    virtual bool backgroundPlugin() const = 0;
    // должен вернуть QWidget* (parent == nullptr)
    virtual QWidget* createWidget() = 0;

    // основное действие
    virtual void execute(const QStringList &files) = 0;

    // передача API (MainWindow)
    virtual void setApplicationAPI(ApplicationAPI *api) = 0;

    virtual void initialize() = 0; // вызывается после setApplicationAPI() 
    virtual void shutdown() = 0;// вызывается перед выгрузкой плагина
};

#define FilePluginInterface_iid "belkin.alexey.FilePluginInterface"
Q_DECLARE_INTERFACE(FilePluginInterface, FilePluginInterface_iid)

#endif // FILEPLUGININTERFACE_H

#ifndef APPLICATIONAPI_H
#define APPLICATIONAPI_H

#include <QString>
#include <QWidget>

class FilePluginInterface;

class ApplicationAPI {
public:
    virtual ~ApplicationAPI() {}

    virtual QString currentFilePath() const = 0;
    virtual void showMessage(const QString &msg) = 0;
    virtual void showDockForPlugin(FilePluginInterface *plugin) = 0;
};

#endif // APPLICATIONAPI_H

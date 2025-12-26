#ifndef APPLICATIONAPI_H
#define APPLICATIONAPI_H

#include <QString>
#include <QWidget>

class FilePluginInterface;
class QHBoxLayout;
class QAction;

class ApplicationAPI {
public:
    virtual ~ApplicationAPI() {}

    virtual QString currentFilePath() const = 0;
    virtual void showMessage(const QString &msg) = 0;
    virtual void showDockForPlugin(FilePluginInterface *plugin) = 0;
    virtual QHBoxLayout* footerBtnPanel() const = 0;
    virtual QWidget* activeView()const = 0;
    virtual QWidget* passiveView() const = 0;
    virtual QStringList selectedFiles() const = 0;
    virtual void addContextMenuAction(QAction *action) = 0;

};

#endif // APPLICATIONAPI_H

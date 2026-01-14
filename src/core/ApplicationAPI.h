#ifndef APPLICATIONAPI_H
#define APPLICATIONAPI_H

#include <QString>
#include <QWidget>
#include "BelkinExport.h"

class FilePluginInterface;
class QHBoxLayout;
class QAction;
class CopySignals;

class BELKINCORE_EXPORT ApplicationAPI {
public:
    virtual ~ApplicationAPI() = default;
    virtual CopySignals* copySignals() = 0;

    virtual QString currentFilePath() const = 0;
    virtual void showMessage(const QString &msg) = 0;
    virtual void showDockForPlugin(FilePluginInterface *plugin) = 0;
    virtual QHBoxLayout* footerBtnPanel() const = 0;
    virtual QWidget* activeView()const = 0;
    virtual QWidget* passiveView() const = 0;
    virtual QStringList selectedFiles() const = 0;
    virtual void addContextMenuAction(QAction *action) = 0;
    virtual void performCopyOperation() = 0;
    virtual QWidget* mainWindow() const = 0;
};

#endif // APPLICATIONAPI_H

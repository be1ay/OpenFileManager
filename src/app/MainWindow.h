#pragma once

#include <QMainWindow>
#include <QVector>
#include <QModelIndex>
#include <QPluginLoader>
#include <QMap>
#include <QStringList>
#include "ApplicationAPI.h"
#include "CopySignals.h"

class QPushButton;
class FilePanel;
class FilePluginInterface;
class QToolBar;
class QDockWidget;
class QHBoxLayout;

class MainWindow : public QMainWindow, public ApplicationAPI
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    QString currentFilePath() const override;
    void showMessage(const QString &msg) override;
    //void addDockWidget(QWidget *widget, const QString &title) override;
    void addDockWidgetForPlugin(FilePluginInterface *plugin, QWidget *widget, const QString &title);
    void showDockForPlugin(FilePluginInterface *plugin) override;
    void removePluginDock(FilePluginInterface *plugin);
    QHBoxLayout* footerBtnPanel() const override;
    QWidget* activeView()  const override;
    QWidget* passiveView() const override;
    QStringList selectedFiles() const override;
    void addContextMenuAction(QAction *action) override;
    CopySignals* copySignals() override { return &m_copySignals; }
    void performCopyOperation() override;
    void performDeleteOperation(bool permanent = false) override;
    void performCreateFolder() override;
    void performRename() override;
    QWidget* mainWindow() const override { return const_cast<MainWindow*>(this); }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showContextMenu(const QPoint &globalPos);
    void onDeleteRequested(bool permanent);
    void onCopyFinished();
    void onCopyDropped(const QStringList &srcPaths, const QString &dstDir);
    void onRenameRequested();
    void onCreateFolderRequested();
    void onCopyToBuffer();
    void onPasteFromBuffer();

    private:
    void setupUi();
    void connectSignals();
    void loadPlugins();
    void unloadPlugins();
    void setActivePanel(QWidget *panelView);
    void updateActiveStyles();
    void createPluginToolbar();
    QList<QAction*> m_contextActions;

    FilePanel          *leftPanel;
    FilePanel          *rightPanel;
    QWidget            *currentActiveView;

    QHBoxLayout        *m_btnLayout;

    QVector<QPluginLoader*>       pluginLoaders;
    QVector<FilePluginInterface*> plugins;
    QToolBar *m_pluginToolBar;
    QMap<FilePluginInterface*, QDockWidget*> m_pluginDocks;
    CopySignals m_copySignals;
    void refreshPanelForPath(const QString &path);
    QStringList m_copyBuffer;
};

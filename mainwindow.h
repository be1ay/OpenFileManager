#pragma once

#include <QMainWindow>
#include <QVector>
#include <QModelIndex>
#include <QPluginLoader>
#include <QMap>
#include "ApplicationAPI.h"

class QPushButton;
class FilePanel;
class FilePluginInterface;
class QToolBar;
class QDockWidget;

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

protected:
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void onCopy();
    void onDelete();
    void onNewFolder();

private:
    void setupUi();
    void connectSignals();
    void loadPlugins();
    void unloadPlugins();
    void setActivePanel(QWidget *panelView);
    void updateActiveStyles();
    void createPluginToolbar();

    QWidget*            activeView()  const;
    QWidget*            passiveView() const;

    FilePanel          *leftPanel;
    FilePanel          *rightPanel;
    QWidget            *currentActiveView;

    QPushButton        *copyBtn;
    QPushButton        *deleteBtn;
    QPushButton        *newFolderBtn;

    QVector<QPluginLoader*>       pluginLoaders;
    QVector<FilePluginInterface*> plugins;
    QToolBar *m_pluginToolBar;
    QMap<FilePluginInterface*, QDockWidget*> m_pluginDocks;

};

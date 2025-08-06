#pragma once

#include <QMainWindow>
#include <QVector>
#include <QModelIndex>
#include <QPluginLoader>

class QPushButton;
class FilePanel;
class FilePluginInterface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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
};

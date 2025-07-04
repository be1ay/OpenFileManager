#pragma once

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void focusInEvent(QFocusEvent *event);

private:
    QFileSystemModel *leftModel;
    QFileSystemModel *rightModel;
    QTreeView *leftView;
    QTreeView *rightView;
    QLabel *leftPathLabel;
    QLabel *rightPathLabel;
    QPushButton *copyBtn;
    QPushButton *deleteBtn;
    QTreeView* activeView() const;
    QTreeView *currentActiveView = nullptr;
    QFileSystemModel* activeModel() const;
    QTreeView* passiveView() const;
    QFileSystemModel* passiveModel() const;

    void setupUi();
    void connectSignals();
    QPushButton *leftUpButton;
    QPushButton *rightUpButton;
    QPushButton *newFolderBtn;
    QComboBox *leftDriveBox;
    QComboBox *rightDriveBox;

private slots:
    void onCopy();
    void onDelete();
    void updateLeftPath(const QModelIndex &index);
    void updateRightPath(const QModelIndex &index);
    void updateActiveStyles();
    void setActivePanel(QTreeView *view);
    void onItemActivated(const QModelIndex &index);
    void onLeftUp();
    void onRightUp();
    bool copyDirectoryRecursively(const QString &srcPath, const QString &dstPath);
    void onNewFolder();
    void populateDriveBox(QComboBox *box);

};

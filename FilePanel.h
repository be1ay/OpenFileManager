#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>



class FilePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FilePanel(QWidget *parent = nullptr);

    // Доступ к внутренним элементам
    QTreeView*             view() const { return m_view; }
    QFileSystemModel*      model() const { return m_model; }
    QString                currentPath() const { return m_currentPath; }

signals:
    // Посылается при переходе на новую папку
    void pathChanged(const QString &newPath);
    void contextMenuRequested(const QPoint &globalPos);


private slots:
    void onUpClicked();
    void onDriveChanged(const QString &drive);
    void onItemActivated(const QModelIndex &index);

private:
    void populateDriveBox();

    QFileSystemModel *m_model;
    QTreeView        *m_view;
    QLabel           *m_pathLabel;
    QPushButton      *m_upButton;
    QComboBox        *m_driveBox;

    QString           m_currentPath;
};

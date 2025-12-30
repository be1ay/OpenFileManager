#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QPersistentModelIndex>


class FilePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FilePanel(QWidget *parent = nullptr);

    // Доступ к внутренним элементам
    QTreeView*             view() const { return m_view; }
    QFileSystemModel*      model() const { return m_model; }
    QString                currentPath() const { return m_currentPath; }
    QModelIndex lastIndex()const {return m_lastIndex; }
    void setPath(const QString &path);


protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    // Посылается при переходе на новую папку
    void pathChanged(const QString &newPath);
    void contextMenuRequested(const QPoint &globalPos);
    void deleteRequested(bool permanent);
    void copyRequested();
    void activated();

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
    QPersistentModelIndex     m_lastIndex;

};

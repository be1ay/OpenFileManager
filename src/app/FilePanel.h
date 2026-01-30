#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QPersistentModelIndex>
class QTreeView;

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
    void refresh();
    bool selectFile(const QString& filePath);

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
    void copyDropped(const QStringList &srcPaths, const QString &dstDir);
    void renameRequested();
    void copyToBufferRequested();
    void pasteFromBufferRequested();
    void moveRequested();



private slots:
    void onUpClicked();
    void onDriveChanged(const QString &drive);
    void onItemActivated(const QModelIndex &index);

private:
    void populateDriveBox();
    void updateDriveBoxSelection();

    QFileSystemModel *m_model;
    QTreeView        *m_view;
    QLabel           *m_pathLabel;
    QPushButton      *m_upButton;
    QComboBox        *m_driveBox;
    QString           m_currentPath;
    QPersistentModelIndex     m_lastIndex;

};
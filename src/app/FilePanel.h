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
    void refresh();


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

#include <QDrag>
#include <QPainter>
class FileView : public QTreeView
{
    Q_OBJECT
public:
    using QTreeView::QTreeView;

protected:
    void startDrag(Qt::DropActions supportedActions) override
    {
        QModelIndexList indexes = selectedIndexes();
        if (indexes.isEmpty())
            return;

        // MIME-данные как обычно
        QMimeData *mimeData = model()->mimeData(indexes);

        // Создаём drag
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // Имя файла
        QString fileName = model()->data(indexes.first(), Qt::DisplayRole).toString();

        // Рисуем маленькую иконку
        QFont font;
        font.setBold(true);

        QFontMetrics fm(font);
        int w = fm.horizontalAdvance(fileName) + 12;
        int h = fm.height() + 8;

        QPixmap pixmap(w, h);
        pixmap.fill(Qt::transparent);

        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);

        // Полупрозрачный тёмный фон
        p.setBrush(QColor(0, 0, 0, 160));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(pixmap.rect(), 4, 4);

        // Белый текст
        p.setPen(Qt::white);
        p.setFont(font);
        p.drawText(6, h - 4, fileName);

        p.end();

        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(6, h / 2));
        drag->exec(supportedActions);
    }
};


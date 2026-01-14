#pragma once

#include <QDrag>
#include <QPainter>
#include <QTreeView>

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
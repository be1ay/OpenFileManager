#include "DuplicateFinderPlugin.hpp"
#include "DuplicateFinderDialog.hpp"

#include <QAction>
#include <QApplication>
#include <QWidget>

void DuplicateFinderPlugin::initialize()
{
    QAction* act = new QAction("Поиск дубликатов");

    connect(act, &QAction::triggered, this, [this]() {
        // Создаём диалог
        DuplicateFinderDialog dlg;

        // Центрируем относительно главного окна приложения
        if (m_api && m_api->mainWindow()) {
            QWidget* mw = m_api->mainWindow();
            dlg.setParent(mw, Qt::Dialog);

            QRect mwRect = mw->geometry();
            QRect dlgRect = dlg.geometry();

            dlg.move(
                mwRect.center().x() - dlgRect.width() / 2,
                mwRect.center().y() - dlgRect.height() / 2
            );
        }

        dlg.exec();
    });

    m_api->addContextMenuAction(act);
    m_actions.append(act);
}

void DuplicateFinderPlugin::execute(const QStringList &files)
{
    // Можно игнорировать files — поиск не зависит от выделенных файлов
    // Просто показываем диалог
    DuplicateFinderDialog dlg;

    if (m_api && m_api->mainWindow()) {
        QWidget* mw = m_api->mainWindow();
        dlg.setParent(mw, Qt::Dialog);

        QRect mwRect = mw->geometry();
        QRect dlgRect = dlg.geometry();

        dlg.move(
            mwRect.center().x() - dlgRect.width() / 2,
            mwRect.center().y() - dlgRect.height() / 2
        );
    }

    if (dlg.exec() == QDialog::Accepted) {
        QString path = dlg.selectedPath();
        if (!path.isEmpty())
            m_api->navigateToFile(path);
    }

}

void DuplicateFinderPlugin::shutdown()
{
    for (QAction* a : m_actions)
        delete a;

    m_actions.clear();
}

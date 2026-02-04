#include "DuplicateFinderPlugin.h"
#include "DuplicateFinderDialog.h"

#include <QAction>
#include <QApplication>
#include <QWidget>
#include <QShortcut>
#include <QKeySequence>

void DuplicateFinderPlugin::initialize()
{
    QAction* act = new QAction("Поиск дубликатов",this);

    connect(act, &QAction::triggered, this, [this](){
        QString initialDir = m_api->currentDirectory();
        runDialog(initialDir); 
    });

    m_api->addContextMenuAction(act);
    
    m_api->registerShortcut( QKeySequence("Ctrl+Shift+D"), this, SLOT(onShortcutTriggered()) );
}

void DuplicateFinderPlugin::execute(const QStringList &files)
{
    runDialog("");
}


void DuplicateFinderPlugin::shutdown()
{
    m_api->unregisterShortcuts(this);
}

QIcon DuplicateFinderPlugin::icon() const
{
    return QIcon(":/duplicatefinder/icons/duplicate.png");
}

void DuplicateFinderPlugin::runDialog(const QString& initialDir)
{
    DuplicateFinderDialog dlg;

    if (!initialDir.isEmpty())
        dlg.setInitialDirectory(initialDir);

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

void DuplicateFinderPlugin::onShortcutTriggered(){
    execute({});
}


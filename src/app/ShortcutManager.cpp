#include "ShortcutManager.h"

ShortcutManager::ShortcutManager(QWidget* mainWindow)
    : m_mainWindow(mainWindow)
{
}

QShortcut* ShortcutManager::registerShortcut(const QKeySequence& seq,
                                             QObject* owner,
                                             const char* slot)
{
    QShortcut* sc = new QShortcut(seq, m_mainWindow);
    QObject::connect(sc, SIGNAL(activated()), owner, slot);
    m_owners.insert(sc, owner);
    return sc;
}

void ShortcutManager::unregisterShortcuts(QObject* owner)
{
    auto it = m_owners.begin();
    while (it != m_owners.end()) {
        if (it.value() == owner) {
            QShortcut* sc = it.key();
            delete sc;
            it = m_owners.erase(it);
        } else {
            ++it;
        }
    }
}

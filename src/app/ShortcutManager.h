#pragma once

#include <QObject>
#include <QShortcut>
#include <QKeySequence>
#include <QHash>
#include <QWidget>

class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutManager(QWidget* mainWindow);

    QShortcut* registerShortcut(const QKeySequence& seq,
                                QObject* owner,
                                const char* slot);

    void unregisterShortcuts(QObject* owner);

private:
    QWidget* m_mainWindow;
    QHash<QShortcut*, QObject*> m_owners;
};

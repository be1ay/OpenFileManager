#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QFileSystemModel>
#include "FileOperationsBtn.h"
#include "FileOperations.h"
#include "CopySignals.h"

void FileOperationsBtn::execute(const QStringList &files)
{
    Q_UNUSED(files)
    
    if (m_api) {
        auto btn_panel= m_api->footerBtnPanel();

        renameBtn_    = new QPushButton(tr("F2 Rename"));
        copyBtn_    = new QPushButton(tr("F5 Copy"));
        moveBtn_    = new QPushButton(tr("F6 Move"));
        newFolderBtn_ = new QPushButton(tr("F7 New Folder"));
        deleteBtn_    = new QPushButton(tr("F8 Delete"));

        btn_panel->addWidget(renameBtn_);
        btn_panel->addWidget(copyBtn_);
        btn_panel->addWidget(moveBtn_);
        btn_panel->addWidget(newFolderBtn_);
        btn_panel->addWidget(deleteBtn_);

        connect(renameBtn_,    &QPushButton::clicked, this, &FileOperationsBtn::onRename);
        connect(copyBtn_,      &QPushButton::clicked, this, &FileOperationsBtn::onCopy);
        connect(moveBtn_,      &QPushButton::clicked, this, &FileOperationsBtn::onMove);
        connect(deleteBtn_,    &QPushButton::clicked, this, &FileOperationsBtn::onDelete);
        connect(newFolderBtn_, &QPushButton::clicked, this, &FileOperationsBtn::onNewFolder);
    }
}

QWidget* FileOperationsBtn::createWidget()
{

    return nullptr;
}

QIcon FileOperationsBtn::icon() const
{
    return QIcon();
}

void FileOperationsBtn::onDelete()
{
    m_api->performDeleteOperation();
}

void FileOperationsBtn::onCopy()
{
    m_api->performCopyOperation();
}

void FileOperationsBtn::onMove()
{
    m_api->performMoveOperation();
}

void FileOperationsBtn::onNewFolder()
{
    m_api->performCreateFolder();
}

void FileOperationsBtn::initialize()
{
    m_api->registerShortcut(QKeySequence(Qt::Key_F2), this, SLOT(onRename()));
    m_api->registerShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onCopy()));
    m_api->registerShortcut(QKeySequence(Qt::Key_F6), this, SLOT(onMove()));
    m_api->registerShortcut(QKeySequence(Qt::Key_F7), this, SLOT(onNewFolder()));
    m_api->registerShortcut(QKeySequence(Qt::Key_F8), this, SLOT(onDelete()));
}

void FileOperationsBtn::shutdown()
{
    m_api->unregisterShortcuts(this);
}

void FileOperationsBtn::onRename()
{
    m_api->performRename();
}

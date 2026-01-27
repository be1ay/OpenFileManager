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

        copyBtn    = new QPushButton(tr("F5 Copy"));
        newFolderBtn = new QPushButton(tr("F7 New Folder"));
        deleteBtn    = new QPushButton(tr("F8 Delete"));

        btn_panel->addWidget(copyBtn);
        btn_panel->addWidget(newFolderBtn);
        btn_panel->addWidget(deleteBtn);

        connect(copyBtn,      &QPushButton::clicked, this, &FileOperationsBtn::onCopy);
        connect(deleteBtn,    &QPushButton::clicked, this, &FileOperationsBtn::onDelete);
        connect(newFolderBtn, &QPushButton::clicked, this, &FileOperationsBtn::onNewFolder);
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

void FileOperationsBtn::onNewFolder()
{
    m_api->performCreateFolder(); 
}

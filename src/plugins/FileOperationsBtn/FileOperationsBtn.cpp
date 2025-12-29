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

        copyBtn    = new QPushButton(tr("Copy"));
        deleteBtn    = new QPushButton(tr("Delete"));
        newFolderBtn = new QPushButton(tr("New Folder"));
        btn_panel->addWidget(copyBtn);
        btn_panel->addWidget(deleteBtn);
        btn_panel->addWidget(newFolderBtn);

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
    auto *view  = qobject_cast<QTreeView*>(m_api->activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());

    const auto sel = view->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        m_api->showMessage("No selection.");
        return;
    }

    QStringList paths;
    for (auto idx : sel)
        paths << model->filePath(idx);

    // Подтверждение
    QString msg = "Delete selected items?\n\n" + paths.join("\n");
    if (QMessageBox::question(nullptr, "Confirm delete", msg) != QMessageBox::Yes)
        return;

    int count = FileOperations::removePaths(paths);

    m_api->showMessage(QString("Deleted %1 items.").arg(count));

    // Обновляем панель
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
}

void FileOperationsBtn::onCopy()
{
    m_api->performCopyOperation();  // новый метод в ApplicationAPI
}


void FileOperationsBtn::onNewFolder()
{
    auto *view  = qobject_cast<QTreeView*>(m_api->activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());
    QString dir = model->filePath(view->rootIndex());

    bool ok;
    QString name = QInputDialog::getText(
        m_api->activeView(), "New Folder", "Enter name:", QLineEdit::Normal, "New Folder", &ok);

    if (!ok || name.isEmpty())
        return;

    if (QDir(dir).mkdir(name)) {
        m_api->showMessage("Created.");
        view->setRootIndex(model->index(dir));
    } else {
        m_api->showMessage("Could not create folder.");
    }
}

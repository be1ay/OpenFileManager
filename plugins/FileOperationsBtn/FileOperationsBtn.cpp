#include "FileOperationsBtn.h"
#include "FileOperations.h"
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

    int count = 0;
    for (auto idx : sel) {
        QString path = model->filePath(idx);
        if (FileOperations::removePath(path))
            ++count;
        else
            m_api->showMessage("Failed to delete: " + path);
    }

    m_api->showMessage( QString("Deleted %1 items.").arg(count));
    // Обновить текущее отображение
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
}

void FileOperationsBtn::onCopy()
{
    auto *srcView = qobject_cast<QTreeView*>(m_api->activeView());
    auto *dstView = qobject_cast<QTreeView*>(m_api->passiveView());

    auto *srcModel = qobject_cast<QFileSystemModel*>(srcView->model());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());

    QString dstDir = dstModel->filePath(dstView->rootIndex());
    if (dstDir.isEmpty()) {
        m_api->showMessage("No destination.");
        return;
    }

    const auto sel = srcView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        m_api->showMessage("No items selected.");
        return;
    }

    int count = 0;
    for (auto idx : sel) {
        QString srcPath = srcModel->filePath(idx);
        QFileInfo info(srcPath);
        QString dstPath = QDir(dstDir).filePath(info.fileName());


        bool ok = false;
        if (info.isDir()) {
            ok = FileOperations::copyDirectoryRecursively(srcPath, dstPath);
        } else {
            if (QFile::exists(dstPath)) {
                QFile::remove(dstPath); // перезаписать
            }
            ok = QFile::copy(srcPath, dstPath);
        }

        if (ok) ++count;
        else{
            qDebug() << "Failed to copy from" << srcPath << "to" << dstPath;
            qDebug() << "Exists:" << QFile::exists(srcPath);
            qDebug() << "Readable:" << QFileInfo(srcPath).isReadable();
            qDebug() << "Writable destination:" << QFileInfo(dstDir).isWritable();
            m_api->showMessage("Failed to copy: " + srcPath);

        }
    }

    m_api->showMessage(QString("Copied %1 items to:\n%2").arg(count).arg(dstDir));
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

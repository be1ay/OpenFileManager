#include "FileOperations.h"
#include <QDir>
#include <QFile>
#include <QFileInfoList>

bool FileOperations::copyDirectoryRecursively(const QString &srcPath,
                                              const QString &dstPath)
{
    QDir sourceDir(srcPath);
    if (!sourceDir.exists())
        return false;

    QDir targetDir(dstPath);
    if (!targetDir.exists()) {
        if (!QDir().mkdir(dstPath))
            return false;
    }

    const QFileInfoList entries =
        sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        QString srcFile = entry.absoluteFilePath();
        QString dstFile = dstPath + "/" + entry.fileName();

        if (entry.isDir()) {
            if (!copyDirectoryRecursively(srcFile, dstFile))
                return false;
        } else {
            if (QFile::exists(dstFile))
                QFile::remove(dstFile);
            if (!QFile::copy(srcFile, dstFile))
                return false;
        }
    }
    return true;
}

bool FileOperations::removePath(const QString &path)
{
    QFileInfo info(path);
    if (info.isDir()) {
        return QDir(path).removeRecursively();
    } else {
        return QFile::remove(path);
    }
}

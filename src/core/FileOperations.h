// FileOperations.h
#pragma once

#include <QStringList>
#include "BelkinExport.h"

class ApplicationAPI;

class BELKINCORE_EXPORT FileOperations
{
public:
    static bool copyDirectoryRecursively(const QString &srcPath,
                                         const QString &dstPath,
                                         ApplicationAPI *api,
                                         int &fileIndex);

    static bool copyFileWithProgress(const QString &srcFile,
                                     const QString &dstFile,
                                     int fileIndex,
                                     ApplicationAPI *api);

    static bool removePaths(const QStringList &paths, bool permanent);
    static bool removePath(const QString &path);
    static bool removeDirectoryRecursively(const QString &path);

    // новый фасад: асинхронное копирование через поток
    static void copyFilesAsync(const QStringList &srcFiles,
                               const QString &dstDir,
                               ApplicationAPI *api);

    // синхронный вариант (используется только внутри потока)
    static bool copyFilesSync(const QStringList &srcFiles,
                              const QString &dstDir,
                              ApplicationAPI *api);
    static bool renamePath(const QString &oldPath, const QString &newPath);

    static QString uniqueNameInDir(const QString &dir, const QString &baseName);


};

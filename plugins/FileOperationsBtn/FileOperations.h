#pragma once

#include <QString>

class FileOperations
{
public:
    // Рекурсивное копирование каталога
    static bool copyDirectoryRecursively(const QString &srcPath,
                                         const QString &dstPath);

    // Удаление файла или папки
    static bool removePath(const QString &path);
};

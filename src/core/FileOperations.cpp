#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QElapsedTimer>
#include <QThread>
#include "CopySignals.h"
#include "FileOperations.h"
#include "ApplicationAPI.h"
#include "CopyWorkerCore.h"


bool FileOperations::copyDirectoryRecursively(const QString &srcPath,
                                              const QString &dstPath,
                                              ApplicationAPI *api,
                                              int &fileIndex)
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

            if (!copyDirectoryRecursively(srcFile, dstFile, api, fileIndex))
                return false;

        } else {

            if (QFile::exists(dstFile))
                QFile::remove(dstFile);

            if (!copyFileWithProgress(srcFile, dstFile, fileIndex, api))
                return false;

            fileIndex++;
        }
    }

    return true;
}

bool FileOperations::copyFileWithProgress(const QString &srcFile,
                                          const QString &dstFile,
                                          int fileIndex,
                                          ApplicationAPI *api)
{
    QFile in(srcFile);
    QFile out(dstFile);

    if (!in.open(QIODevice::ReadOnly))
        return false;

    if (!out.open(QIODevice::WriteOnly))
        return false;

    qint64 total = in.size();
    qint64 copied = 0;

    const qint64 block = 1024 * 1024; // 1 MB
    QByteArray buffer;
    buffer.resize(block);

    QElapsedTimer timer;
    timer.start();

    while (true) {

    qint64 read = in.read(buffer.data(), block);

    if (read < 0) {
        // ошибка чтения
        return false;
    }

    if (read == 0) {
        // EOF
        break;
    }

    qint64 written = out.write(buffer.data(), read);
    if (written != read) {
        // ошибка записи
        return false;
    }

    copied += read;

    double seconds = timer.elapsed() / 1000.0;
    double speedMB = seconds > 0 ? (copied / (1024.0 * 1024.0)) / seconds : 0;

    auto *sig = api->copySignals();
    if (sig)
        emit sig->copyProgress(fileIndex, copied, total, speedMB);
    }

    out.flush();
    out.close();
    in.close();


    return true;
}

bool FileOperations::copyFilesSync(const QStringList &srcFiles,
                                   const QString &dstDir,
                                   ApplicationAPI *api)
{
    auto *sig = api->copySignals();
    if (sig)
        sig->copyStarted(srcFiles, dstDir);

    if (srcFiles.isEmpty()) {
        if (sig) sig->copyFinished();
        return true;
    }

    int fileIndex = 0;

    for (const QString &srcPath : srcFiles) {

        QFileInfo info(srcPath);
        QString dstPath = dstDir + "/" + info.fileName();

        bool ok = false;

        if (info.isDir()) {
            ok = copyDirectoryRecursively(srcPath, dstPath, api, fileIndex);
        } else {
            ok = copyFileWithProgress(srcPath, dstPath, fileIndex, api);
        }

        if (!ok) {
            if (sig) {
                sig->copyError(srcPath);
                sig->copyFinished();
            }
            return false;
        }

        ++fileIndex;
    }

    if (sig)
        sig->copyFinished();

    return true;
}

void FileOperations::copyFilesAsync(const QStringList &srcFiles,
                                    const QString &dstDir,
                                    ApplicationAPI *api)
{
    // если файлов нет — даже поток не создаём
    if (srcFiles.isEmpty()) {
        if (auto *sig = api->copySignals()) {
            sig->copyStarted(srcFiles, dstDir);
            sig->copyFinished();
        }
        return;
    }

    auto *worker = new CopyWorkerCore(srcFiles, dstDir, api);
    auto *thread = new QThread;

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started,
                     worker, &CopyWorkerCore::start);

    QObject::connect(worker, &CopyWorkerCore::finished,
                     thread,  &QThread::quit);
    QObject::connect(worker, &CopyWorkerCore::finished,
                     worker,  &CopyWorkerCore::deleteLater);
    QObject::connect(thread, &QThread::finished,
                     thread,  &QThread::deleteLater);

    thread->start();
}

int FileOperations::removePaths(const QStringList &paths)
{
    int count = 0;

    for (const QString &path : paths) {
        if (removePath(path))
            ++count;
    }

    return count;
}

bool FileOperations::removePath(const QString &path)
{
    QFileInfo info(path);

    if (info.isDir())
        return removeDirectoryRecursively(path);

    return QFile::remove(path);
}

bool FileOperations::removeDirectoryRecursively(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return false;

    QFileInfoList entries =
        dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            if (!removeDirectoryRecursively(entry.absoluteFilePath()))
                return false;
        } else {
            if (!QFile::remove(entry.absoluteFilePath()))
                return false;
        }
    }

    return dir.rmdir(path);
}

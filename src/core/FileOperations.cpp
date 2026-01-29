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
        //QString dstFile = dstPath + "/" + entry.fileName();
        QString baseName = entry.fileName();
        QString finalName = FileOperations::uniqueNameInDir(dstPath, baseName);
        QString dstFile = dstPath + "/" + finalName;


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
    if (!in.open(QIODevice::ReadOnly))
        return false;

    QString tmpFile = dstFile + ".tmp";
    QFile out(tmpFile);

    if (!out.open(QIODevice::WriteOnly))
        return false;

    qint64 total = in.size();
    qint64 copied = 0;

    const qint64 block = 1024 * 1024;
    QByteArray buffer(block, Qt::Uninitialized);

    QElapsedTimer timer;
    timer.start();

    while (true) {

        qint64 read = in.read(buffer.data(), block);
        if (read < 0)
            return false;

        if (read == 0)
            break;

        if (out.write(buffer.constData(), read) != read)
            return false;

        copied += read;

        double seconds = timer.elapsed() / 1000.0;
        double speedMB = seconds > 0
            ? (copied / (1024.0 * 1024.0)) / seconds
            : 0;

        if (auto *sig = api->copySignals())
            emit sig->copyProgress(fileIndex, copied, total, speedMB);
    }

    out.flush();
    out.close();
    in.close();

    QFile::remove(dstFile);               // если перезапись
    if (!QFile::rename(tmpFile, dstFile)) // ключевой момент
        return false;

    return true;
}


bool FileOperations::copyFilesSync(const QStringList &srcFiles,
                                   const QString &dstDir,
                                   ApplicationAPI *api)
{
    auto *sig = api->copySignals();
    if (sig)
        sig->copyStarted(srcFiles, dstDir, FileOpType::Copy);

    if (srcFiles.isEmpty()) {
        if (sig) sig->copyFinished();
        return true;
    }

    int fileIndex = 0;

    for (const QString &srcPath : srcFiles) {

        QFileInfo info(srcPath);

        QString baseName = info.fileName();
        QString finalName = FileOperations::uniqueNameInDir(dstDir, baseName);

        QString dstPath = dstDir + "/" + finalName;


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
            sig->copyStarted(srcFiles, dstDir,FileOpType::Copy);
            sig->copyFinished();
        }
        return;
    }

    auto *worker = new CopyWorkerCore(srcFiles, dstDir, api,FileOpType::Copy);
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

void FileOperations::moveFilesAsync(const QStringList &srcFiles,
                                    const QString &dstDir,
                                    ApplicationAPI *api)
{
    if (srcFiles.isEmpty()) {
        if (auto *sig = api->copySignals()) {
            sig->copyStarted(srcFiles, dstDir,FileOpType::Move);
            sig->copyFinished();
        }
        return;
    }

    auto *worker = new CopyWorkerCore(srcFiles, dstDir, api, FileOpType::Move);
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


#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>

static bool sendToTrash(const QStringList &paths)
{
    qDebug() << "sendToTrash called";

    QString joined;

    for (const QString &p : paths) {

        // Преобразуем путь в нативный формат Windows
        QString native = QDir::toNativeSeparators(p);

        joined += native;
        joined += QChar('\0');
    }

    joined += QChar('\0');

    SHFILEOPSTRUCTW op;
    memset(&op, 0, sizeof(op));

    op.wFunc = FO_DELETE;
    op.pFrom = (LPCWSTR)joined.utf16();
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    int result = SHFileOperationW(&op);

    return (result == 0);
}

#endif

#ifdef Q_OS_LINUX
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

static QString generateUniqueTrashName(const QString &trashFilesDir, const QString &baseName)
{
    QString candidate = baseName;
    int counter = 1;

    while (QFile::exists(trashFilesDir + candidate)) {
        candidate = baseName + QStringLiteral("_%1").arg(counter++);
    }

    return candidate;
}

static bool writeTrashInfo(const QString &trashInfoDir,
                           const QString &uniqueName,
                           const QString &originalPath)
{
    QString infoPath = trashInfoDir + uniqueName + ".trashinfo";

    QFile infoFile(infoPath);
    if (!infoFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&infoFile);
    out << "[Trash Info]\n";
    out << "Path=" << originalPath << "\n";
    out << "DeletionDate=" << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";

    return true;
}

static bool sendToTrash(const QStringList &paths)
{
    QString trashBase = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                        + "/.local/share/Trash/";

    QString trashFiles = trashBase + "files/";
    QString trashInfo  = trashBase + "info/";

    QDir().mkpath(trashFiles);
    QDir().mkpath(trashInfo);

    for (const QString &p : paths) {

        QFileInfo fi(p);
        QString baseName = fi.fileName();

        // 1. Генерируем уникальное имя
        QString uniqueName = generateUniqueTrashName(trashFiles, baseName);

        // 2. Создаём .trashinfo
        if (!writeTrashInfo(trashInfo, uniqueName, fi.absoluteFilePath()))
            return false;

        // 3. Перемещаем файл
        QString target = trashFiles + uniqueName;
        if (!QFile::rename(p, target))
            return false;
    }

    return true;
}
#endif

#ifdef Q_OS_MAC
#include <Foundation/Foundation.h>

static bool sendToTrash(const QStringList &paths)
{
    NSFileManager *fm = [NSFileManager defaultManager];

    for (const QString &p : paths) {

        const char *fsPath = p.toUtf8().constData();
        NSString *ns = [NSString stringWithUTF8String:fsPath];

        if (!ns)
            continue;

        NSURL *url = [NSURL fileURLWithPath:ns];

        NSError *error = nil;
        [fm trashItemAtURL:url resultingItemURL:nil error:&error];

        if (error)
            return false;
    }
    return true;
}
#endif

bool FileOperations::removePaths(const QStringList &paths, bool permanent)
{
    if (paths.isEmpty())
        return false;

    if (!permanent)
        return sendToTrash(paths);

    bool ok = true;
    for (const QString &path : paths)
        ok &= removePath(path);

    return ok;
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

bool FileOperations::renamePath(const QString &oldPath, const QString &newPath)
{
    return QFile::rename(oldPath, newPath);
}

QString FileOperations::uniqueNameInDir(const QString &dir, const QString &fileName)
{
    QDir d(dir);

    // Разбираем имя вручную, чтобы корректно обрабатывать много точек
    int lastDot = fileName.lastIndexOf('.');

    QString base;
    QString ext;

    if (lastDot > 0) {
        base = fileName.left(lastDot);       // всё до последней точки
        ext  = fileName.mid(lastDot + 1);    // всё после последней точки
    } else {
        base = fileName;
        ext  = "";
    }

    auto makeName = [&](const QString &core) -> QString {
        return ext.isEmpty() ? core : core + "." + ext;
    };

    QString copyWord = QObject::tr("Copy");

    // 1. Базовое имя
    QString candidate = makeName(base);
    if (!d.exists(candidate))
        return candidate;

    // 2. "base - Copy"
    candidate = makeName(base + " - " + copyWord);
    if (!d.exists(candidate))
        return candidate;

    // 3. "base - Copy (2)", "base - Copy (3)", ...
    int counter = 2;
    while (true) {
        candidate = makeName(QString("%1 - %2 (%3)")
                             .arg(base)
                             .arg(copyWord)
                             .arg(counter));
        if (!d.exists(candidate))
            return candidate;
        counter++;
    }
}

#ifdef _WIN32
#include <windows.h>

bool sameDevice(const QString &a, const QString &b)
{
    wchar_t volA[MAX_PATH], volB[MAX_PATH];

    if (!GetVolumePathNameW((LPCWSTR)a.utf16(), volA, MAX_PATH))
        return false;

    if (!GetVolumePathNameW((LPCWSTR)b.utf16(), volB, MAX_PATH))
        return false;

    wchar_t fsA[MAX_PATH], fsB[MAX_PATH];
    DWORD serialA = 0, serialB = 0;

    GetVolumeInformationW(volA, nullptr, 0, &serialA, nullptr, nullptr, fsA, MAX_PATH);
    GetVolumeInformationW(volB, nullptr, 0, &serialB, nullptr, nullptr, fsB, MAX_PATH);

    return serialA == serialB;
}
#else
#include <sys/stat.h>
#include <unistd.h>

bool sameDevice(const QString &pathA, const QString &pathB)
{
    struct stat stA{}, stB{};

    if (stat(pathA.toUtf8().constData(), &stA) != 0)
        return false;

    if (stat(pathB.toUtf8().constData(), &stB) != 0)
        return false;

    return stA.st_dev == stB.st_dev;
}
#endif
bool FileOperations::moveFilesSync(const QStringList &srcFiles,
                                   const QString &dstDir,
                                   ApplicationAPI *api)
{
    if (srcFiles.isEmpty())
        return true;

    auto *sig = api->copySignals();
    if (sig)
        sig->copyStarted(srcFiles, dstDir, FileOpType::Move);

    int fileIndex = 0;

    for (const QString &srcPath : srcFiles)
    {
        QFileInfo info(srcPath);
        const QString srcDir = info.absolutePath();
        const QString baseName = info.fileName();

        // 0. Перемещение в ту же директорию — бессмысленно, просто пропускаем
        if (QDir::cleanPath(srcDir) == QDir::cleanPath(dstDir)) {
            qDebug() << "Move in same dir, skip:" << srcPath;
            ++fileIndex;
            continue;
        }

        // Путь назначения без авто-ренейминга
        QString dstPathRaw = dstDir + "/" + baseName;

        // 1. Попытка быстрого перемещения (rename) на одном устройстве
        if (sameDevice(srcDir, dstDir)) {

            // src и dst разные директории, но имя то же — это нормальный move
            if (QFile::rename(srcPath, dstPathRaw)) {
                qDebug() << "Fast rename:" << srcPath << "->" << dstPathRaw;
                if (sig)
                    emit sig->copyProgress(fileIndex, 1, 1, 0);
                ++fileIndex;
                continue;
            }

            qDebug() << "Rename failed, fallback to copy:" << srcPath;
        } else {
            qDebug() << "Different FS, copy+delete:" << srcPath;
        }

        // 2. COPY + DELETE с уникальным именем (как при копировании)
        QString finalName = FileOperations::uniqueNameInDir(dstDir, baseName);
        QString dstPath   = dstDir + "/" + finalName;

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

        FileOperations::removePaths({srcPath}, false);
        ++fileIndex;
    }

    if (sig)
        sig->copyFinished();

    return true;
}







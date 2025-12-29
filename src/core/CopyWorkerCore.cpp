#include "CopyWorkerCore.h"
#include "FileOperations.h"
#include "ApplicationAPI.h"
#include "CopySignals.h"

CopyWorkerCore::CopyWorkerCore(const QStringList &files,
                               const QString &targetDir,
                               ApplicationAPI *api,
                               QObject *parent)
    : QObject(parent)
    , m_files(files)
    , m_targetDir(targetDir)
    , m_api(api)
{
}

void CopyWorkerCore::start()
{
    // здесь мы в рабочем потоке
    FileOperations::copyFilesSync(m_files, m_targetDir, m_api);
    emit finished();
}

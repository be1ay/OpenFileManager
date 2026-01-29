#include "ApplicationAPI.h"
#include "CopyWorkerCore.h"
#include "FileOperations.h"
#include "CopySignals.h"

CopyWorkerCore::CopyWorkerCore(const QStringList &files,
                               const QString &targetDir,
                               ApplicationAPI *api,
                               FileOpType opType,
                               QObject *parent)
    : QObject(parent)
    , m_files(files)
    , m_targetDir(targetDir)
    , m_opType(opType)
    , m_api(api)
{
}

void CopyWorkerCore::start()
{
    if (m_opType == FileOpType::Copy) {
        FileOperations::copyFilesSync(m_files, m_targetDir, m_api);
    } else {
        FileOperations::moveFilesSync(m_files, m_targetDir, m_api);
    }

    emit finished();
}


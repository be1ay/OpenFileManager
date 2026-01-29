#pragma once
#include <QObject>
#include <QStringList>
#include "FileOpType.h"

class ApplicationAPI;

class CopyWorkerCore : public QObject
{
    Q_OBJECT
public:
    CopyWorkerCore(const QStringList &files,
                   const QString &targetDir,
                   ApplicationAPI *api,
                   FileOpType opType,
                   QObject *parent = nullptr);

public slots:
    void start();

signals:
    void finished();

private:
    QStringList    m_files;
    QString        m_targetDir;
    FileOpType     m_opType;
    ApplicationAPI *m_api;
};

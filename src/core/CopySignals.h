#pragma once
#include <QObject>
#include <QStringList>
#include "BelkinExport.h"
#include "FileOpType.h"

class BELKINCORE_EXPORT CopySignals : public QObject
{
    Q_OBJECT
public:
    explicit CopySignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void copyStarted(const QStringList &files, const QString &targetDir, FileOpType opType);
    void copyProgress(int fileIndex, qint64 copied, qint64 totalBytes, double speedMB);
    void copyFinished();
    void copyError(const QString &path);
};

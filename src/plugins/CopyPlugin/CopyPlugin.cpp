#include "CopyPlugin.h"
#include "CopyProgressDialog.hpp"
#include "CopySignals.h"

#include <QDebug>

void CopyPlugin::initialize()
{
    auto *sig = m_api->copySignals();

    connect(sig, &CopySignals::copyStarted,
            this, &CopyPlugin::onCopyStarted);

    connect(sig, &CopySignals::copyProgress,
            this, &CopyPlugin::onCopyProgress);

    connect(sig, &CopySignals::copyFinished,
            this, &CopyPlugin::onCopyFinished);

    connect(sig, &CopySignals::copyError,
            this, &CopyPlugin::onCopyError);

    qDebug() << "[CopyPlugin] initialized";
}

void CopyPlugin::shutdown()
{
    if (m_dialog) {
        m_dialog->close();
        m_dialog = nullptr;
    }
}

void CopyPlugin::onCopyStarted(const QStringList &files, const QString &targetDir,FileOpType opType)
{
    if (m_dialog) {
        m_dialog->close();
        m_dialog = nullptr;
    }

    QWidget *mw = m_api->mainWindow();

    m_dialog = new CopyProgressDialog(files.size(), opType, mw);
    m_dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_dialog->setWindowModality(Qt::NonModal);
    m_dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    m_dialog->show();

    // Центрирование
    QRect pr = mw->geometry();
    QRect dr = m_dialog->geometry();
    m_dialog->move(
        pr.x() + (pr.width()  - dr.width())  / 2,
        pr.y() + (pr.height() - dr.height()) / 2
    );
}


void CopyPlugin::onCopyProgress(int fileIndex, qint64 copied, qint64 total, double speedMB)
{
    if (!m_dialog)
        return;

    m_dialog->updateProgress(fileIndex, copied, total, speedMB);
}

void CopyPlugin::onCopyFinished()
{
    qDebug() << "[CopyPlugin] Copy finished";

    if (m_dialog) {
        m_dialog->close();
        m_dialog = nullptr;
    }
}

void CopyPlugin::onCopyError(const QString &path)
{
    qDebug() << "[CopyPlugin] Copy error:" << path;

    if (m_dialog)
        m_dialog->showError("Failed to copy:\n" + path);
}

#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class CopyProgressDialog : public QDialog
{
    Q_OBJECT
public:
    CopyProgressDialog(int fileCount, FileOpType opType, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        if (opType == FileOpType::Copy)
            setWindowTitle(tr("Copying files..."));
        else
            setWindowTitle(tr("Moving files..."));

        setMinimumWidth(420);

        m_fileLabel = new QLabel("File 1 of " + QString::number(fileCount));
        m_speedLabel = new QLabel("Speed: 0 MB/s");

        m_progress = new QProgressBar;
        m_progress->setRange(0, 100);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(m_fileLabel);
        layout->addWidget(m_progress);
        layout->addWidget(m_speedLabel);

        setLayout(layout);
    }

    void updateProgress(int fileIndex, qint64 copied, qint64 total, double speedMB)
    {
        m_fileLabel->setText(QString("File %1").arg(fileIndex + 1));
        m_progress->setValue(int((double)copied / total * 100));
        m_speedLabel->setText(QString("Speed: %1 MB/s").arg(speedMB, 0, 'f', 2));
    }

    void showError(const QString &msg)
    {
        m_speedLabel->setText("<font color='red'>" + msg + "</font>");
    }

private:
    QLabel *m_fileLabel;
    QLabel *m_speedLabel;
    QProgressBar *m_progress;
};

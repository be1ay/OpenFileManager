#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QKeyEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QSignalBlocker>
#include "FilePanel.h"
#include "FileView.hpp"

FilePanel::FilePanel(QWidget *parent)
    : QWidget(parent)
    , m_model(new QFileSystemModel(this))
    , m_view(new FileView(this))
    , m_pathLabel(new QLabel(this))
    , m_upButton(new QPushButton("⬆ Up", this))
    , m_driveBox(new QComboBox(this))
{
    // Настройка модели и представления
    m_model->setRootPath(QDir::rootPath());
    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setColumnWidth(0, 250);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setItemsExpandable(false);
    m_view->setExpandsOnDoubleClick(false);
    
//Drag&Drop
    m_view->setDragEnabled(true);
    m_view->viewport()->setAcceptDrops(true);
    m_view->setAcceptDrops(true); 
    m_view->setDropIndicatorShown(true);
    m_view->setDragDropMode(QAbstractItemView::DragDrop);
    m_view->setDefaultDropAction(Qt::IgnoreAction);
    // Перехватываем события 
    m_view->viewport()->installEventFilter(this);

    // если есть хотя бы одна строка – запомним её как стартовую
    QModelIndex first = m_model->index(0, 0, m_view->rootIndex());
    if (first.isValid())
        m_lastIndex = first;

    // следим за текущей строкой
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &){
                if (current.isValid())
                    m_lastIndex = current.sibling(current.row(), 0); // нормализуем к колонке 0
            });


    // Список дисков/корневых точек
    populateDriveBox();
    //updateDriveBoxSelection();

    // Сборка интерфейса
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_upButton);
    topLayout->addWidget(m_driveBox);
    topLayout->addWidget(m_pathLabel);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_view);
    setLayout(mainLayout);

    // Сигналы-слоты
    connect(m_upButton, &QPushButton::clicked,       this, &FilePanel::onUpClicked);
    connect(m_driveBox, &QComboBox::currentTextChanged,
            this, &FilePanel::onDriveChanged);
    connect(m_view, &QTreeView::doubleClicked,
            this, &FilePanel::onItemActivated);

    connect(m_view, &QWidget::customContextMenuRequested,
        this, [this](const QPoint &pos) {
            emit contextMenuRequested(m_view->viewport()->mapToGlobal(pos));
        });
}

void FilePanel::populateDriveBox()
{
    m_driveBox->clear();
#ifdef Q_OS_WIN
    const auto& drives = QDir::drives();
    for (const QFileInfo &fi : drives) {
        m_driveBox->addItem(fi.absoluteFilePath());
    }
#else
    m_driveBox->addItem("/");
    m_driveBox->addItem(QDir::homePath());

    QDir media("/media");
    for (const QString &entry : media.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        m_driveBox->addItem("/media/" + entry);

    QDir mnt("/mnt");
    for (const QString &entry : mnt.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        m_driveBox->addItem("/mnt/" + entry);
#endif
}

void FilePanel::updateDriveBoxSelection()
{
#ifdef Q_OS_WIN
    QString root = QFileInfo(m_currentPath).absolutePath().left(3);
#else
    QString root = "/";

    if (m_currentPath.startsWith("/media/"))
        root = m_currentPath.section('/', 0, 2); // "/media/user"

    else if (m_currentPath.startsWith("/mnt/"))
        root = m_currentPath.section('/', 0, 2); // "/mnt/disk"

    else if (m_currentPath.startsWith(QDir::homePath()))
        root = QDir::homePath();
#endif

    int idx = m_driveBox->findText(root);
    if (idx >= 0){
        QSignalBlocker blocker(m_driveBox);
        m_driveBox->setCurrentIndex(idx);
    }
}


void FilePanel::onUpClicked()
{
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        m_currentPath = dir.absolutePath();
        m_view->setRootIndex(m_model->index(m_currentPath));
        m_pathLabel->setText(m_currentPath);
        updateDriveBoxSelection();
        emit pathChanged(m_currentPath);
    }
}

void FilePanel::onDriveChanged(const QString &drive)
{
    m_currentPath = drive;
    qDebug()<<m_currentPath;
    m_view->setRootIndex(m_model->index(drive));
    m_pathLabel->setText(drive);
    updateDriveBoxSelection();
    emit pathChanged(drive);
}

void FilePanel::onItemActivated(const QModelIndex &idx)
{
    QString path = m_model->fileInfo(idx).absoluteFilePath();
    if (QFileInfo(path).isDir()) {
        m_currentPath = path;
        m_view->setRootIndex(m_model->index(path));
        m_pathLabel->setText(path);
        updateDriveBoxSelection();
        emit pathChanged(path);
    }
}

void FilePanel::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        bool permanent = event->modifiers() & Qt::ShiftModifier;
        emit deleteRequested(permanent);
        return;
    }

    if (event->key() == Qt::Key_F8) {
        emit deleteRequested(false);
        return;
    }

    if (event->key() == Qt::Key_F5) {
        emit copyRequested();
        return;
    }

    if (event->key() == Qt::Key_F6) {
        emit moveRequested();
        return;
    }

    if (event->key() == Qt::Key_F2) {
        emit renameRequested();
        return;
    }

    if (event->matches(QKeySequence::Copy))
    { // Ctrl+C
        emit copyToBufferRequested();
        return;
    }

    if (event->matches(QKeySequence::Paste))
    { // Ctrl+V
        emit pasteFromBufferRequested();
        return;
    }

    QWidget::keyPressEvent(event);
}


bool FilePanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_view->viewport() && event->type() == QEvent::MouseButtonPress) {

        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QModelIndex idx = m_view->indexAt(me->pos());

        emit activated();

        if (!idx.isValid()) {
            return true;
        }
        return false;
    }
    if (obj == m_view->viewport())
    {
        if (event->type() == QEvent::DragEnter)
        {
            auto *e = static_cast<QDragEnterEvent *>(event);
            if (e->mimeData()->hasUrls())
                e->acceptProposedAction();
            return true;
        }
        if (event->type() == QEvent::DragMove)
        {
            auto *e = static_cast<QDragMoveEvent *>(event);
            e->acceptProposedAction();
            return true;
        }
        if (event->type() == QEvent::Drop)
        {
            auto *e = static_cast<QDropEvent *>(event);
            QStringList paths;
            for (const QUrl &url : e->mimeData()->urls())
                paths << url.toLocalFile();
            QString dstDir = m_model->filePath(m_view->rootIndex());
            emit copyDropped(paths, dstDir);
            e->acceptProposedAction();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void FilePanel::setPath(const QString &path)
{
    if (!QFileInfo(path).exists())
        return;

    m_currentPath = path;
    m_view->setRootIndex(m_model->index(path));
    m_pathLabel->setText(path);
    updateDriveBoxSelection();

    // обновляем lastIndex — ставим первую строку
    QModelIndex first = m_model->index(0, 0, m_view->rootIndex());
    if (first.isValid())
        m_lastIndex = first;
}

void FilePanel::refresh()
{
    // Сбрасываем lastIndex — директория могла измениться
    m_lastIndex = QPersistentModelIndex();

    // Перечитываем директорию
    QModelIndex idx = m_model->index(m_currentPath);
    m_view->setRootIndex(idx);
    updateDriveBoxSelection();
}

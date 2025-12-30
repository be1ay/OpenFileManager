#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QKeyEvent>
#include "FilePanel.h"

FilePanel::FilePanel(QWidget *parent)
    : QWidget(parent)
    , m_model(new QFileSystemModel(this))
    , m_view(new QTreeView(this))
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
    


    // фильтр на viewport – ловим клик по пустому месту
    m_view->viewport()->installEventFilter(this);

    // инициализация lastIndex после установки rootIndex
    m_currentPath = QDir::homePath();
    m_view->setRootIndex(m_model->index(m_currentPath));
    m_pathLabel->setText(m_currentPath);

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


    // Инициализация пути
    m_currentPath = QDir::homePath();
    m_view->setRootIndex(m_model->index(m_currentPath));
    m_pathLabel->setText(m_currentPath);

    // Список дисков/корневых точек
    populateDriveBox();
    QString d = QFileInfo(m_currentPath).absolutePath().left(3);
    int idx = m_driveBox->findText(d);
    if (idx >= 0) m_driveBox->setCurrentIndex(idx);

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
#endif
}

void FilePanel::onUpClicked()
{
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        m_currentPath = dir.absolutePath();
        m_view->setRootIndex(m_model->index(m_currentPath));
        m_pathLabel->setText(m_currentPath);
        emit pathChanged(m_currentPath);
    }
}

void FilePanel::onDriveChanged(const QString &drive)
{
    m_currentPath = drive;
    m_view->setRootIndex(m_model->index(drive));
    m_pathLabel->setText(drive);
    emit pathChanged(drive);
}

void FilePanel::onItemActivated(const QModelIndex &idx)
{
    QString path = m_model->fileInfo(idx).absoluteFilePath();
    if (QFileInfo(path).isDir()) {
        m_currentPath = path;
        m_view->setRootIndex(m_model->index(path));
        m_pathLabel->setText(path);
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

    if (event->key() == Qt::Key_F5) {
        emit copyRequested();
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

    return QWidget::eventFilter(obj, event);
}

void FilePanel::setPath(const QString &path)
{
    if (!QFileInfo(path).exists())
        return;

    m_currentPath = path;
    m_view->setRootIndex(m_model->index(path));
    m_pathLabel->setText(path);

    // обновляем lastIndex — ставим первую строку
    QModelIndex first = m_model->index(0, 0, m_view->rootIndex());
    if (first.isValid())
        m_lastIndex = first;
}


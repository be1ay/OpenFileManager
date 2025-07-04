#include "mainwindow.h"
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QComboBox>
#include <QPluginLoader>
#include "FilePluginInterface.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    currentActiveView = leftView; // По умолчанию
    leftView->setFocus();
    updateActiveStyles();

    connectSignals();
    loadPlugins();
}

MainWindow::~MainWindow()
{
    unloadPlugins();
}

void MainWindow::loadPlugins()
{
    QDir pluginsDir(qApp->applicationDirPath() + "/plugins");
    const QStringList entryList = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : entryList) {
        QString fullPath = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(fullPath, this);
        QObject *pluginObj = loader->instance();
        if (pluginObj) {
            auto iface = qobject_cast<FilePluginInterface *>(pluginObj);
            if (iface) {
                plugins << iface;
                pluginLoaders << loader;
                qDebug() << "Loaded plugin:" << iface->name();
            } else {
                delete loader;
            }
        } else {
            qDebug() << "Failed to load plugin:" << fileName << loader->errorString();
            delete loader;
        }
    }
}

void MainWindow::unloadPlugins()
{
    for (QPluginLoader *loader : std::as_const(pluginLoaders)) {
        qDebug() << "Unloading plugin:" << loader->fileName();
        loader->unload(); // выгрузить .dll/.so
        delete loader;    // удалить loader
    }
    pluginLoaders.clear();
    plugins.clear();
}



void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *panelsLayout = new QHBoxLayout;

    leftModel = new QFileSystemModel(this);
    leftModel->setRootPath(QDir::rootPath());

    rightModel = new QFileSystemModel(this);
    rightModel->setRootPath(QDir::rootPath());

    leftView = new QTreeView;
    leftView->setModel(leftModel);
    leftView->setRootIndex(leftModel->index(QDir::homePath()));
    leftView->setColumnWidth(0, 250);

    rightView = new QTreeView;
    rightView->setModel(rightModel);
    rightView->setRootIndex(rightModel->index(QDir::homePath()));
    rightView->setColumnWidth(0, 250);

    leftView->setRootIsDecorated(false);
    rightView->setRootIsDecorated(false);
    leftView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    rightView->setSelectionMode(QAbstractItemView::ExtendedSelection);


    leftPathLabel = new QLabel(QDir::homePath());
    rightPathLabel = new QLabel(QDir::homePath());

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftUpButton = new QPushButton("⬆ Up");
    leftDriveBox = new QComboBox;
    populateDriveBox(leftDriveBox);

    QHBoxLayout *leftTop = new QHBoxLayout;
    leftTop->addWidget(leftUpButton);
    leftTop->addWidget(leftDriveBox);
    leftTop->addWidget(leftPathLabel);

    leftLayout->addLayout(leftTop);
    leftLayout->addWidget(leftView);



    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightUpButton = new QPushButton("⬆ Up");
    rightDriveBox = new QComboBox;
    populateDriveBox(rightDriveBox);

    QHBoxLayout *rightTop = new QHBoxLayout;
    rightTop->addWidget(rightUpButton);
    rightTop->addWidget(rightDriveBox);
    rightTop->addWidget(rightPathLabel);

    rightLayout->addLayout(rightTop);
    rightLayout->addWidget(rightView);




    panelsLayout->addLayout(leftLayout);
    panelsLayout->addLayout(rightLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    copyBtn = new QPushButton(tr("Copy"));
    deleteBtn = new QPushButton(tr("Delete"));

    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(deleteBtn);

    newFolderBtn = new QPushButton(tr("New Folder"));


    btnLayout->addWidget(newFolderBtn);

    mainLayout->addLayout(panelsLayout);
    mainLayout->addLayout(btnLayout);

    QString defaultPath = QDir::homePath();
    leftDriveBox->setCurrentText(QFileInfo(defaultPath).absolutePath().left(3));  // e.g., "C:/"
    rightDriveBox->setCurrentText(QFileInfo(defaultPath).absolutePath().left(3));


    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle("Qt File Manager");
}

void MainWindow::connectSignals()
{
    connect(copyBtn, &QPushButton::clicked, this, &MainWindow::onCopy);

    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDelete);

    connect(leftView, &QTreeView::clicked, this, [this](const QModelIndex &) {
        setActivePanel(leftView);
    });

    connect(rightView, &QTreeView::clicked, this, [this](const QModelIndex &) {
        setActivePanel(rightView);
    });

    connect(leftView, &QTreeView::doubleClicked, this, &MainWindow::onItemActivated);
    connect(rightView, &QTreeView::doubleClicked, this, &MainWindow::onItemActivated);
    connect(leftUpButton, &QPushButton::clicked, this, &MainWindow::onLeftUp);
    connect(rightUpButton, &QPushButton::clicked, this, &MainWindow::onRightUp);

    connect(newFolderBtn, &QPushButton::clicked, this, &MainWindow::onNewFolder);
    connect(leftDriveBox, &QComboBox::currentTextChanged, this, [this](const QString &path) {
        leftView->setRootIndex(leftModel->index(path));
        leftPathLabel->setText(path);
    });

    connect(rightDriveBox, &QComboBox::currentTextChanged, this, [this](const QString &path) {
        rightView->setRootIndex(rightModel->index(path));
        rightPathLabel->setText(path);
    });


}

void MainWindow::onDelete()
{
    QTreeView *view = activeView();
    QFileSystemModel *model = activeModel();

    const QModelIndexList& selected = view->selectionModel()->selectedRows(); // только имя (0-я колонка)

    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete", "No files or folders selected.");
        return;
    }

    int deletedCount = 0;
    for (const QModelIndex &index : selected) {
        QString path = model->filePath(index);
        QFileInfo info(path);

        bool success = false;
        if (info.isDir()) {
            QDir dir(path);
            success = dir.removeRecursively();
        } else {
            success = QFile::remove(path);
        }

        if (success)
            deletedCount++;
        else
            QMessageBox::warning(this, "Error", "Could not delete: " + path);
    }

    QMessageBox::information(this, "Delete", QString("Deleted %1 items.").arg(deletedCount));

    // Обновить view вручную, если нужно
    QString currentPath = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(currentPath));
}


void MainWindow::updateLeftPath(const QModelIndex &index)
{
    leftPathLabel->setText(leftModel->filePath(index));
}

void MainWindow::updateRightPath(const QModelIndex &index)
{
    rightPathLabel->setText(rightModel->filePath(index));
}

void MainWindow::onCopy()
{
    QTreeView *fromView = activeView();
    QFileSystemModel *fromModel = activeModel();

    QTreeView *toView = passiveView();
    QFileSystemModel *toModel = passiveModel();

    QString destDir = toModel->filePath(toView->rootIndex());
    if (destDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "No destination selected.");
        return;
    }

    QItemSelectionModel *sel = fromView->selectionModel();
    const QModelIndexList& selected = sel->selectedRows();  // Только первая колонка (имя)

    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Error", "No files or folders selected.");
        return;
    }

    int copiedCount = 0;
    for (const QModelIndex &index : selected) {
        QString sourcePath = fromModel->filePath(index);
        QFileInfo info(sourcePath);
        QString destPath = destDir + "/" + info.fileName();

        if (info.isDir()) {
            if (copyDirectoryRecursively(sourcePath, destPath))
                copiedCount++;
        } else {
            if (QFile::exists(destPath)) QFile::remove(destPath); // перезаписать
            if (QFile::copy(sourcePath, destPath))
                copiedCount++;
        }
    }

    QMessageBox::information(this, "Copy done",
                             QString("Copied %1 items to:\n%2").arg(copiedCount).arg(destDir));
}

bool MainWindow::copyDirectoryRecursively(const QString &srcPath, const QString &dstPath)
{
    QDir sourceDir(srcPath);
    if (!sourceDir.exists())
        return false;

    QDir destDir(dstPath);
    if (!destDir.exists()) {
        if (!QDir().mkdir(dstPath))
            return false;
    }

    const QFileInfoList& entries = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        QString srcFilePath = entry.absoluteFilePath();
        QString dstFilePath = dstPath + "/" + entry.fileName();

        if (entry.isDir()) {
            if (!copyDirectoryRecursively(srcFilePath, dstFilePath))
                return false;
        } else {
            if (QFile::exists(dstFilePath)) QFile::remove(dstFilePath);
            if (!QFile::copy(srcFilePath, dstFilePath))
                return false;
        }
    }
    return true;
}



void MainWindow::focusInEvent(QFocusEvent *event)
{
    QMainWindow::focusInEvent(event);
    updateActiveStyles();
}

void MainWindow::updateActiveStyles()
{
    QString activeStyle = "border: 2px solid #007acc;";
    QString inactiveStyle = "border: 1px solid gray;";

    leftView->setStyleSheet(leftView->hasFocus() ? activeStyle : inactiveStyle);
    rightView->setStyleSheet(rightView->hasFocus() ? activeStyle : inactiveStyle);
}


QTreeView* MainWindow::activeView() const {
    return currentActiveView;
}

QTreeView* MainWindow::passiveView() const {
    return (currentActiveView == leftView) ? rightView : leftView;
}

QFileSystemModel* MainWindow::activeModel() const {
    return (currentActiveView == leftView) ? leftModel : rightModel;
}

QFileSystemModel* MainWindow::passiveModel() const {
    return (currentActiveView == leftView) ? rightModel : leftModel;
}


void MainWindow::setActivePanel(QTreeView *view)
{
    if (currentActiveView == view)
        return;

    // Снять выделение в предыдущей панели
    if (currentActiveView)
        currentActiveView->selectionModel()->clearSelection();

    // Установить новую активную панель
    currentActiveView = view;
    currentActiveView->setFocus();

    updateActiveStyles();
}

void MainWindow::onItemActivated(const QModelIndex &index)
{
    QTreeView *view = qobject_cast<QTreeView *>(sender());
    if (!view)
        return;

    QFileSystemModel *model = (view == leftView) ? leftModel : rightModel;
    QFileInfo info = model->fileInfo(index);

    if (info.isDir()) {
        view->setRootIndex(model->index(info.absoluteFilePath()));

        // Обновить путь в заголовке
        if (view == leftView)
            leftPathLabel->setText(info.absoluteFilePath());
        else
            rightPathLabel->setText(info.absoluteFilePath());
    }
}

void MainWindow::onLeftUp()
{
    QModelIndex currentRoot = leftView->rootIndex();
    QString currentPath = leftModel->filePath(currentRoot);
    QDir dir(currentPath);
    if (dir.cdUp()) {
        QString parentPath = dir.absolutePath();
        leftView->setRootIndex(leftModel->index(parentPath));
        leftPathLabel->setText(parentPath);
    }
}

void MainWindow::onRightUp()
{
    QModelIndex currentRoot = rightView->rootIndex();
    QString currentPath = rightModel->filePath(currentRoot);
    QDir dir(currentPath);
    if (dir.cdUp()) {
        QString parentPath = dir.absolutePath();
        rightView->setRootIndex(rightModel->index(parentPath));
        rightPathLabel->setText(parentPath);
    }
}

void MainWindow::onNewFolder()
{
    QTreeView *view = activeView();
    QFileSystemModel *model = activeModel();
    QString currentDir = model->filePath(view->rootIndex());

    bool ok;
    QString folderName = QInputDialog::getText(this, "New Folder",
                                               "Enter folder name:", QLineEdit::Normal, "New Folder", &ok);
    if (!ok || folderName.isEmpty())
        return;

    QDir dir(currentDir);
    if (dir.mkdir(folderName)) {
        QMessageBox::information(this, "Success", "Folder created.");
        view->setRootIndex(model->index(currentDir)); // обновим
    } else {
        QMessageBox::warning(this, "Failed", "Could not create folder.");
    }
}


void MainWindow::populateDriveBox(QComboBox *box)
{
    box->clear();
#ifdef Q_OS_WIN
    const QList<QFileInfo> &drives = QDir::drives();
    for (const QFileInfo &drive : drives) {
        box->addItem(drive.absoluteFilePath());
    }
#else
    // Для Linux/macOS можно добавить типовые монтируемые пути
    box->addItem("/home");
    box->addItem("/mnt");
    box->addItem("/media");
    box->addItem("/");
#endif
}

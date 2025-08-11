#include "MainWindow.h"
#include "FilePanel.h"
#include "FileOperations.h"
#include "FilePluginInterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QToolBar>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , leftPanel(nullptr)
    , rightPanel(nullptr)
    , currentActiveView(nullptr)
{

    createPluginToolbar();


    setupUi();
        loadPlugins();

    // По умолчанию – левая панель
    currentActiveView = leftPanel->view();
    currentActiveView->setFocus();
    updateActiveStyles();

    connectSignals();


}

MainWindow::~MainWindow()
{
    unloadPlugins();
}

QString MainWindow::currentFilePath() const {
    auto *view = qobject_cast<QTreeView*>(activeView());
    if (!view)
        return {};

    auto selection = view->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return {}; // ничего не выбрано — вернём пустую строку

    QModelIndex idx = selection.first();
    return static_cast<QFileSystemModel*>(view->model())->filePath(idx);
}


void MainWindow::showMessage(const QString &msg)
{
    QMessageBox::information(this, "Plugin Message", msg);
}

void MainWindow::addDockWidgetForPlugin(FilePluginInterface *plugin, QWidget *widget, const QString &title)
{
    if (m_pluginDocks.contains(plugin)) {
        QDockWidget *dock = m_pluginDocks.value(plugin);
        dock->show();
        dock->raise();
        return;
    }
    if(widget){
    QDockWidget *dock = new QDockWidget(title, this);
    dock->setObjectName(QStringLiteral("pluginDock_%1").arg(title));
    dock->setWidget(widget); // dock владеет widget'ом

        addDockWidget(Qt::RightDockWidgetArea, dock);
        m_pluginDocks.insert(plugin, dock);
    }
}

void MainWindow::setupUi()
{
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(m_pluginToolBar);
    // Панели файлового менеджера
    auto *panelsLayout = new QHBoxLayout;
    leftPanel  = new FilePanel(this);
    rightPanel = new FilePanel(this);
    panelsLayout->addWidget(leftPanel);
    panelsLayout->addWidget(rightPanel);

    // Кнопки действий
    auto *btnLayout = new QHBoxLayout;
    copyBtn      = new QPushButton(tr("Copy"), this);
    deleteBtn    = new QPushButton(tr("Delete"), this);
    newFolderBtn = new QPushButton(tr("New Folder"), this);
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(newFolderBtn);

    mainLayout->addLayout(panelsLayout);
    mainLayout->addLayout(btnLayout);

    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle("Qt File Manager");
}

void MainWindow::connectSignals()
{
    connect(copyBtn,      &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(deleteBtn,    &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(newFolderBtn, &QPushButton::clicked, this, &MainWindow::onNewFolder);

    // Выбор активной панели по клику
    connect(leftPanel->view(), &QTreeView::clicked, this, [this](const QModelIndex&){
        setActivePanel(leftPanel->view());
    });
    connect(rightPanel->view(), &QTreeView::clicked, this, [this](const QModelIndex&){
        setActivePanel(rightPanel->view());
    });

    // Активная панель по двойному клику
    connect(leftPanel->view(), &QTreeView::doubleClicked, this, [this](const QModelIndex&){
        setActivePanel(leftPanel->view());
    });
    connect(rightPanel->view(), &QTreeView::doubleClicked, this, [this](const QModelIndex&){
        setActivePanel(rightPanel->view());
    });
}

void MainWindow::loadPlugins()
{
    QDir pluginsDir(qApp->applicationDirPath() + "/plugins");
    const QStringList files = pluginsDir.entryList(QDir::Files);
    QStringList allowedExtensions = { ".dll", ".so", ".dylib" };

    for (const QString &fileName : files) {
        QFileInfo info(fileName);
        QString ext = "." + info.suffix().toLower();
        if (!allowedExtensions.contains(ext))
            continue;

        QString path = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(path, this);
        QObject *obj = loader->instance();
        if (!obj) {
            qDebug() << "Failed to load plugin:" << fileName << loader->errorString();
            delete loader;
            continue;
        }

        FilePluginInterface *iface = qobject_cast<FilePluginInterface*>(obj);
        if (!iface) {
            delete loader;
            continue;
        }

        // важно: сначала дать API, чтобы createWidget() мог им пользоваться
        iface->setApplicationAPI(this);

        // сохранить
        plugins.append(iface);
        pluginLoaders.append(loader);

        // создать док один раз при загрузке (если плагин этого требует)
        if (iface->showWidget()) {
            QWidget *w = iface->createWidget(); // должен вернуть widget с parent==nullptr
            if (w) addDockWidgetForPlugin(iface, w, iface->name());
        }

        // добавить кнопку на тулбар — при клике вызываем execute()
        QAction *act = m_pluginToolBar->addAction(iface->icon().isNull() ? QIcon(":/icons/default_plugin.png") : iface->icon(), iface->name());
        connect(act, &QAction::triggered, this, [this, iface]() {
            QString path = currentFilePath();
            iface->execute(path); // плагин при showWidget==true должен вызвать api->showDockForPlugin(this)
        });

        qDebug() << "Loaded plugin:" << iface->name();
    }
}

void MainWindow::unloadPlugins()
{
    for (int i = 0; i < plugins.size(); ++i) {
        FilePluginInterface *p = plugins[i];
        // удалить док, если есть
        removePluginDock(p);

        QPluginLoader *loader = pluginLoaders[i];
        qDebug() << "Unloading plugin:" << loader->fileName();
        loader->unload();
        delete loader;
    }
    plugins.clear();
    pluginLoaders.clear();
    m_pluginDocks.clear();
}


QWidget* MainWindow::activeView() const
{
    return currentActiveView;
}

QWidget* MainWindow::passiveView() const
{
    return (currentActiveView == leftPanel->view()
            ? rightPanel->view()
            : leftPanel->view());
}

void MainWindow::setActivePanel(QWidget *view)
{
    if (currentActiveView == view)
        return;

    // Сброс селекции старой панели
    if (currentActiveView) {
        auto *oldView = qobject_cast<QTreeView*>(currentActiveView);
        if (oldView)
            oldView->selectionModel()->clearSelection();
    }

    currentActiveView = view;
    view->setFocus();
    updateActiveStyles();
}

void MainWindow::updateActiveStyles()
{
    QString activeStyle   = "border: 1px solid #007acc;";
    QString inactiveStyle = "border: 1px solid gray;";

    auto *lview = leftPanel->view();
    auto *rview = rightPanel->view();
    lview->setStyleSheet(lview->hasFocus() ? activeStyle : inactiveStyle);
    rview->setStyleSheet(rview->hasFocus() ? activeStyle : inactiveStyle);
}

void MainWindow::createPluginToolbar()
{
    m_pluginToolBar = new QToolBar("Plugins", this);
}

void MainWindow::showDockForPlugin(FilePluginInterface *plugin)
{
    if (!m_pluginDocks.contains(plugin)) return;
    QDockWidget *dock = m_pluginDocks.value(plugin);
    if (dock->isHidden()) dock->show();
    dock->raise();
}

void MainWindow::removePluginDock(FilePluginInterface *plugin)
{
    auto it = m_pluginDocks.find(plugin);
    if (it == m_pluginDocks.end()) return;
    QDockWidget *dock = it.value();
    removeDockWidget(dock);
    delete dock;
    m_pluginDocks.remove(plugin);
}


void MainWindow::focusInEvent(QFocusEvent *ev)
{
    QMainWindow::focusInEvent(ev);
    updateActiveStyles();
}

void MainWindow::onDelete()
{
    auto *view  = qobject_cast<QTreeView*>(activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());
    const auto sel = view->selectionModel()->selectedRows();

    if (sel.isEmpty()) {
        QMessageBox::information(this, "Delete", "No selection.");
        return;
    }

    int count = 0;
    for (auto idx : sel) {
        QString path = model->filePath(idx);
        if (FileOperations::removePath(path))
            ++count;
        else
            QMessageBox::warning(this, "Error", "Failed to delete: " + path);
    }

    QMessageBox::information(this, "Deleted", QString("Deleted %1 items.").arg(count));
    // Обновить текущее отображение
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
}

void MainWindow::onCopy()
{
    auto *srcView = qobject_cast<QTreeView*>(activeView());
    auto *dstView = qobject_cast<QTreeView*>(passiveView());

    auto *srcModel = qobject_cast<QFileSystemModel*>(srcView->model());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());

    QString dstDir = dstModel->filePath(dstView->rootIndex());
    if (dstDir.isEmpty()) {
        QMessageBox::warning(this, "Copy", "No destination.");
        return;
    }

    const auto sel = srcView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::warning(this, "Copy", "No items selected.");
        return;
    }

    int count = 0;
    for (auto idx : sel) {
        QString srcPath = srcModel->filePath(idx);
        QFileInfo info(srcPath);
        QString dstPath = dstDir + "/" + info.fileName();

        bool ok = false;
        if (info.isDir()) {
            ok = FileOperations::copyDirectoryRecursively(srcPath, dstPath);
        } else {
            if (QFile::exists(dstPath)) {
                QFile::remove(dstPath); // перезаписать
            }
            ok = QFile::copy(srcPath, dstPath);
        }

        if (ok) ++count;
        else QMessageBox::warning(this, "Error", "Failed to copy: " + srcPath);
    }

    QMessageBox::information(
        this, "Copy", QString("Copied %1 items to:\n%2").arg(count).arg(dstDir));
}


void MainWindow::onNewFolder()
{
    auto *view  = qobject_cast<QTreeView*>(activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());
    QString dir = model->filePath(view->rootIndex());

    bool ok;
    QString name = QInputDialog::getText(
        this, "New Folder", "Enter name:", QLineEdit::Normal, "New Folder", &ok);

    if (!ok || name.isEmpty())
        return;

    if (QDir(dir).mkdir(name)) {
        QMessageBox::information(this, "Folder", "Created.");
        view->setRootIndex(model->index(dir));
    } else {
        QMessageBox::warning(this, "Folder", "Could not create folder.");
    }
}

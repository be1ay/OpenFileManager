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
#include <QMenu>
#include <QResource>
#include <QSettings>
#include <QTreeView>
#include "MainWindow.h"
#include "FilePanel.h"
#include "FilePluginInterface.h"
#include "FileOperations.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , leftPanel(nullptr)
    , rightPanel(nullptr)
    , currentActiveView(nullptr)
{

    createPluginToolbar();
    setupUi();
    loadPlugins();



    QSettings settings("BelkinSoft", "BelkinCommander");

    QString leftPath  = settings.value("Panels/LeftPath",  QDir::homePath()).toString();
    QString rightPath = settings.value("Panels/RightPath", QDir::homePath()).toString();

    // восстановить пути
    leftPanel->setPath(leftPath);
    rightPanel->setPath(rightPath);
    //Restore geometry
    if (settings.contains("MainWindow/geometry"))
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    if (settings.contains("MainWindow/state"))
        restoreState(settings.value("MainWindow/state").toByteArray());

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
    m_btnLayout  = new QHBoxLayout();
    mainLayout->addLayout(panelsLayout);
    mainLayout->addLayout(m_btnLayout);

    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle("Belkin Commander");
}

void MainWindow::connectSignals()
{
    // Выбор активной панели по клику
    connect(leftPanel,  &FilePanel::activated,
        this,       [this]() { setActivePanel(leftPanel->view()); });

    connect(rightPanel, &FilePanel::activated,
        this,       [this]() { setActivePanel(rightPanel->view()); });

    // Контекстное меню
    connect(leftPanel,  &FilePanel::contextMenuRequested,
        this,       &MainWindow::showContextMenu);

    connect(rightPanel, &FilePanel::contextMenuRequested,
        this,       &MainWindow::showContextMenu);

    connect(leftPanel,  &FilePanel::deleteRequested,
        this,       &MainWindow::onDeleteRequested);

    connect(rightPanel, &FilePanel::deleteRequested,
        this,       &MainWindow::onDeleteRequested);
    
    connect(leftPanel,  &FilePanel::copyRequested,
        this,       &MainWindow::performCopyOperation);

    connect(rightPanel, &FilePanel::copyRequested,
        this,       &MainWindow::performCopyOperation);

    connect(copySignals(), &CopySignals::copyFinished,
         this,      &MainWindow::onCopyFinished);
    
    connect(leftPanel,  &FilePanel::copyDropped,
            this,       &MainWindow::onCopyDropped);

    connect(rightPanel, &FilePanel::copyDropped,
            this, &MainWindow::onCopyDropped);

    connect(leftPanel, &FilePanel::renameRequested,
            this, &MainWindow::onRenameRequested);

    connect(rightPanel, &FilePanel::renameRequested,
            this, &MainWindow::onRenameRequested);

    connect(leftPanel, &FilePanel::copyToBufferRequested,
            this, &MainWindow::onCopyToBuffer);

    connect(rightPanel, &FilePanel::copyToBufferRequested,
            this, &MainWindow::onCopyToBuffer);

    connect(leftPanel, &FilePanel::pasteFromBufferRequested,
            this, &MainWindow::onPasteFromBuffer);

    connect(rightPanel, &FilePanel::pasteFromBufferRequested,
            this, &MainWindow::onPasteFromBuffer);

    connect(leftPanel, &FilePanel::moveRequested,
            this, &MainWindow::performMoveOperation);

    connect(rightPanel, &FilePanel::moveRequested,
            this, &MainWindow::performMoveOperation);
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
        iface->initialize();


        // сохранить
        plugins.append(iface);
        pluginLoaders.append(loader);

        // создать док один раз при загрузке (если плагин этого требует)
        if (iface->showWidget()) {
            QWidget *w = iface->createWidget(); // должен вернуть widget с parent==nullptr
            if (w) addDockWidgetForPlugin(iface, w, iface->name());
        }

        if(!iface->backgroundPlugin()){
            qDebug() << QFile::exists(":/icons/default_plugin.png");

            // добавить кнопку на тулбар — при клике вызываем execute()
            QAction *act = m_pluginToolBar->addAction(iface->icon().isNull() ? QIcon(":/icons/default_plugin.png") : iface->icon(), iface->name());
            connect(act, &QAction::triggered, this, [this, iface]() {
                QStringList paths = selectedFiles();
                iface->execute(paths);

            });
        }
        else{
            QStringList paths = selectedFiles();
            iface->execute(paths);
        }

        qDebug() << "Loaded plugin:" << iface->name();
    }
}

void MainWindow::unloadPlugins()
{
    for (int i = 0; i < plugins.size(); ++i) {
        FilePluginInterface *p = plugins[i];
        // удалить док, если есть
        removePluginDock(p);
        p->shutdown();

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

    // 1. Снимаем выделение со старой активной панели (если была)
    if (currentActiveView) {
        if (auto *oldView = qobject_cast<QTreeView*>(currentActiveView)) {
            if (auto *oldSel = oldView->selectionModel()) {
                oldSel->clearSelection();
            }
        }
    }

    // 2. Запоминаем новую активную
    currentActiveView = view;

    auto *panel = qobject_cast<FilePanel*>(view->parentWidget());
    auto *tree  = qobject_cast<QTreeView*>(view);

    if (!panel || !tree)
        return;

    tree->setFocus();

    QModelIndex idx = panel->lastIndex();

    // Если нет lastIndex — берём первую строку
    if (!idx.isValid()|| idx.model() != tree->model())
        idx = tree->model()->index(0, 0, tree->rootIndex());

    if (idx.isValid()) {
        // На всякий случай нормализуем к колонке 0
        idx = idx.sibling(idx.row(), 0);

        if (auto *sel = tree->selectionModel()) {
            sel->setCurrentIndex(
                idx,
                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
            );
        }
        tree->scrollTo(idx);
    }

    updateActiveStyles();
}

void MainWindow::updateActiveStyles()
{
    QString activeStyle   = "border: 1px solid #007acc;";
    QString inactiveStyle = "border: 1px solid gray;";

    auto *lview = leftPanel->view();
    auto *rview = rightPanel->view();

    lview->setStyleSheet(currentActiveView == lview ? activeStyle : inactiveStyle);
    rview->setStyleSheet(currentActiveView == rview ? activeStyle : inactiveStyle);
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

QHBoxLayout* MainWindow::footerBtnPanel() const
{
    return m_btnLayout;
}

/*
void MainWindow::focusInEvent(QFocusEvent *ev)
{
    QMainWindow::focusInEvent(ev);
    updateActiveStyles();
}*/

QStringList MainWindow::selectedFiles() const
{
    QStringList result;

    auto *view = qobject_cast<QTreeView*>(activeView());
    if (!view)
        return result;

    auto *model = qobject_cast<QFileSystemModel*>(view->model());
    if (!model)
        return result;

    QModelIndexList rows = view->selectionModel()->selectedRows();

    for (const QModelIndex &idx : rows) {
        result << model->filePath(idx);
    }

    return result;
}

void MainWindow::addContextMenuAction(QAction *action)
{
    if (!action)
        return;

    m_contextActions.append(action);
}

void MainWindow::showContextMenu(const QPoint &globalPos)
{
    QMenu menu;

    // системные действия (если нужны)
    // menu.addAction("Открыть", ...);

    // действия плагинов
    for (QAction *act : m_contextActions) {
        menu.addAction(act);
    }

    menu.exec(globalPos);
}

void MainWindow::onDeleteRequested(bool permanent)
{
    auto *view  = qobject_cast<QTreeView*>(activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());

    const auto sel = view->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        showMessage("No selection.");
        return;
    }

    // Собираем список путей
    QStringList paths;
    for (auto idx : sel)
        paths << model->filePath(idx);

    // Подтверждение
    QString msg = "Delete selected items?\n\n" + paths.join("\n");
    if (QMessageBox::question(this, "Confirm delete", msg) != QMessageBox::Yes)
        return;

    // Удаляем через Core
    int count = FileOperations::removePaths(paths, permanent);

    //showMessage(QString("Deleted %1 items.").arg(count));

    // Обновляем панель
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
}

void MainWindow::onRenameRequested()
{
    auto *view  = qobject_cast<QTreeView*>(activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());

    QModelIndex idx = view->currentIndex();
    if (!idx.isValid())
        return;

    QString oldPath = model->filePath(idx);
    QString oldName = QFileInfo(oldPath).fileName();

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "Rename",
        "New name:",
        QLineEdit::Normal,
        oldName,
        &ok
    );

    if (!ok || newName.isEmpty() || newName == oldName)
        return;

    QString newPath = QFileInfo(oldPath).absoluteDir().absoluteFilePath(newName);

    if (!FileOperations::renamePath(oldPath, newPath)) {
        QMessageBox::warning(this, "Error", "Failed to rename.");
        return;
    }

    // обновляем панель
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
}

void MainWindow::onCreateFolderRequested()
{
    auto *view  = qobject_cast<QTreeView*>(activeView());
    auto *model = qobject_cast<QFileSystemModel*>(view->model());

    // Текущая директория активной панели
    QString root = model->filePath(view->rootIndex());
    if (root.isEmpty())
        return;

    // Запрос имени новой папки
    bool ok = false;
    QString name = QInputDialog::getText(
        this,
        "Create folder",
        "Folder name:",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok || name.trimmed().isEmpty())
        return;

    QString newPath = root + "/" + name.trimmed();

    // Проверка существования
    if (QFileInfo::exists(newPath)) {
        QMessageBox::warning(this, "Error", "A file or folder with this name already exists.");
        return;
    }

    // Создание папки
    if (!QDir().mkdir(newPath)) {
        QMessageBox::warning(this, "Error", "Failed to create folder.");
        return;
    }

    // Обновляем панель
    view->setRootIndex(model->index(root));

    // Выделяем созданную папку
    QModelIndex idx = model->index(newPath);
    if (idx.isValid()) {
        view->selectionModel()->setCurrentIndex(
            idx,
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
        );
        view->scrollTo(idx);
    }
    view->setFocus();
}

void MainWindow::performCopyOperation()
{
    auto *srcView  = qobject_cast<QTreeView*>(activeView());
    auto *dstView  = qobject_cast<QTreeView*>(passiveView());

    auto *srcModel = qobject_cast<QFileSystemModel*>(srcView->model());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());

    const auto sel = srcView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        showMessage("No selection.");
        return;
    }

    QStringList files;
    for (auto idx : sel)
        files << srcModel->filePath(idx);

    const QString dstDir = dstModel->filePath(dstView->rootIndex());

    FileOperations::copyFilesAsync(files, dstDir, this);

    // обновить пассивную панель
    dstView->setRootIndex(dstModel->index(dstDir));
}

void MainWindow::onCopyFinished()
{

}

void MainWindow::performDeleteOperation(bool permanent)
{
    onDeleteRequested(permanent);
}

void MainWindow::performRename()
{
    onRenameRequested();
}

void MainWindow::performCreateFolder()
{
    onCreateFolderRequested();
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {

        if (obj == leftPanel->view())
            setActivePanel(leftPanel->view());

        if (obj == rightPanel->view())
            setActivePanel(rightPanel->view());
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("BelkinSoft", "BelkinCommander");

    // сохраняем геометрию окна
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());

    // сохраняем пути панелей
    settings.setValue("Panels/LeftPath",  leftPanel->currentPath());
    settings.setValue("Panels/RightPath", rightPanel->currentPath());

    QMainWindow::closeEvent(event);
}

void MainWindow::onCopyDropped(const QStringList &srcPaths, const QString &dstDir)
{
    QStringList filtered;

    for (const QString &src : srcPaths)
    {
        QFileInfo fi(src);

        // Если копируем в ту же папку — пропускаем
        if (fi.absolutePath() == dstDir)
            continue;

        filtered << src;
    }

    if (filtered.isEmpty())
        return; // нечего копировать

    FileOperations::copyFilesAsync(filtered, dstDir, this);

    auto *dstView  = qobject_cast<QTreeView*>(passiveView());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());
    dstView->setRootIndex(dstModel->index(dstDir));
}

void MainWindow::onCopyToBuffer()
{
    m_copyBuffer = selectedFiles();
    if (m_copyBuffer.isEmpty()) {
        showMessage("Nothing to copy.");
        return;
    }

    //showMessage(QString("Copied %1 item(s) to buffer.").arg(m_copyBuffer.size()));
}

void MainWindow::onPasteFromBuffer()
{
    if (m_copyBuffer.isEmpty()) {
        showMessage("Copy buffer is empty.");
        return;
    }

    auto *dstView  = qobject_cast<QTreeView*>(activeView());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());

    QString dstDir = dstModel->filePath(dstView->rootIndex());

    FileOperations::copyFilesAsync(m_copyBuffer, dstDir, this);

    // обновить панель
    dstView->setRootIndex(dstModel->index(dstDir));
}
void MainWindow::performMoveOperation()
{
    auto *srcView  = qobject_cast<QTreeView*>(activeView());
    auto *dstView  = qobject_cast<QTreeView*>(passiveView());

    auto *srcModel = qobject_cast<QFileSystemModel*>(srcView->model());
    auto *dstModel = qobject_cast<QFileSystemModel*>(dstView->model());

    const auto sel = srcView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        showMessage("No selection.");
        return;
    }

    QStringList files;
    for (auto idx : sel)
        files << srcModel->filePath(idx);

    const QString dstDir = dstModel->filePath(dstView->rootIndex());

    //FileOperations::moveFilesSync(files, dstDir, this);
    FileOperations::moveFilesAsync(files, dstDir, this);


    // обновить панели
    srcView->setRootIndex(srcModel->index(srcModel->filePath(srcView->rootIndex())));
    dstView->setRootIndex(dstModel->index(dstDir));
}
void MainWindow::refreshPanelForPath(const QString &path)
{
    // Определяем, какая панель является целевой
    FilePanel *panel = nullptr;

    if (leftPanel->currentPath() == path)
        panel = leftPanel;
    else if (rightPanel->currentPath() == path)
        panel = rightPanel;

    if (!panel)
        return;

    // Обновляем rootIndex (это заставляет модель перечитать директорию)
    panel->refresh();
}











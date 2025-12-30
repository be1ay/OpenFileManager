#include "MainWindow.h"
#include "FilePanel.h"

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
#include <QMenu>
#include <QResource>
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
    if (!idx.isValid())
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

void MainWindow::onDeleteRequested()
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
    int count = FileOperations::removePaths(paths);

    //showMessage(QString("Deleted %1 items.").arg(count));

    // Обновляем панель
    QString root = model->filePath(view->rootIndex());
    view->setRootIndex(model->index(root));
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
    auto *dstView  = qobject_cast<QTreeView*>(passiveView());
    auto *oldModel = qobject_cast<QFileSystemModel*>(dstView->model());

    QString dstDir = oldModel->filePath(dstView->rootIndex());

    // Создаём новую модель
    auto *newModel = new QFileSystemModel(dstView);
    newModel->setFilter(oldModel->filter());
    newModel->setRootPath(dstDir);

    dstView->setModel(newModel);
    dstView->setRootIndex(newModel->index(dstDir));

    // старую модель можно удалить
    oldModel->deleteLater();
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







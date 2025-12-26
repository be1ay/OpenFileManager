#include "mainwindow.h"
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
    /*copyBtn    = new QPushButton(tr("Copy"), this);
    deleteBtn    = new QPushButton(tr("Delete"), this);
    newFolderBtn = new QPushButton(tr("New Folder"), this);
    m_btnLayout->addWidget(copyBtn);
    m_btnLayout->addWidget(deleteBtn);
    m_btnLayout->addWidget(newFolderBtn);*/

    mainLayout->addLayout(panelsLayout);
    mainLayout->addLayout(m_btnLayout);

    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle("Qt File Manager");
}

void MainWindow::connectSignals()
{


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

        if(!iface->backgroundPlugin()){
            // добавить кнопку на тулбар — при клике вызываем execute()
            QAction *act = m_pluginToolBar->addAction(iface->icon().isNull() ? QIcon(":/icons/default_plugin.png") : iface->icon(), iface->name());
            connect(act, &QAction::triggered, this, [this, iface]() {
                QString path = currentFilePath();
                iface->execute(path); // плагин при showWidget==true должен вызвать api->showDockForPlugin(this)
            });
        }
        else{
            iface->execute(currentFilePath());
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

QHBoxLayout* MainWindow::footerBtnPanel() const
{
    return m_btnLayout;
}


void MainWindow::focusInEvent(QFocusEvent *ev)
{
    QMainWindow::focusInEvent(ev);
    updateActiveStyles();
}

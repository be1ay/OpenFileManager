#include "examplePlugin.h"
#include <QMessageBox>
#include <QLabel>

void ExamplePlugin::execute(const QStringList &files)
{
    if (m_api) {

        m_api->showMessage("Plugin got files:\n" + files.join("\n"));
        m_api->showDockForPlugin(this); // попросить MainWindow показать док
    }
}

QWidget* ExamplePlugin::createWidget()
{
    if (!m_widget) {
        m_widget = new QLabel("Example plugin\n(создано при загрузке)", nullptr);
        m_widget->setAlignment(Qt::AlignCenter);
    }
    return m_widget;
}

QIcon ExamplePlugin::icon() const
{
    return QIcon();
}

void ExamplePlugin::initialize()
{
    QAction *act = new QAction("Example Plugin Action");

    connect(act, &QAction::triggered, this, [this]() {
        QStringList files = m_api->selectedFiles();
        m_api->showMessage("Plugin action:\n" + files.join("\n"));
    });

    m_api->addContextMenuAction(act);
    m_actions.append(act);
}

void ExamplePlugin::shutdown()
{
    // удалить свои действия из меню
    for (QAction *a : m_actions)
        delete a;

    m_actions.clear();
}


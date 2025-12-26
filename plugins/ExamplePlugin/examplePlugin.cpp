#include "examplePlugin.h"
#include <QMessageBox>
#include <QLabel>

void ExamplePlugin::execute(const QString &filePath)
{
    if (m_api) {
        m_api->showMessage("Plugin got file: " + filePath);
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

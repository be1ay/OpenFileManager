#include "examplePlugin.h"
#include <QMessageBox>

void ExamplePlugin::execute(const QString &filePath)
{
    QMessageBox::information(nullptr, "Plugin executed", "Selected file: " + filePath);
}

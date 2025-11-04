#ifndef UICONSTANTS_H
#define UICONSTANTS_H

#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <QIcon>
#include <QString>

namespace Frontend::Constants
{

namespace Settings
{

const QString skLanguage = "language";
const QString skGeometry = "geometry";
const QString skState = "state";
const QString skDockingState = "dockingState";
const QString skRecent = "recent";
const QString skFileName = "Settings.ini";
const QString skMainWindow = "mainWindow";
const QString skLastPathFile = "lastPathFile";
}

namespace Colors
{

vtkNew<vtkNamedColors> const vtkColors;
}

namespace Size
{

const QSize skToolBarIcon = QSize(25, 25);
const uint skMaxRecentProjects = 5;

}
}

#endif // UICONSTANTS_H

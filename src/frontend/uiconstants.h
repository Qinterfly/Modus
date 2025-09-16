#ifndef UICONSTANTS_H
#define UICONSTANTS_H

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

}

namespace Size
{

const QSize skToolBarIcon = QSize(25, 25);
const uint skMaxRecentProjects = 5;

}

namespace Role
{

const int skParent = Qt::UserRole + 1;

}
}

#endif // UICONSTANTS_H

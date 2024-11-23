#include "CorePlugin.h"
#include <QDebug>

CorePlugin::Info CorePlugin::info()
{
	CorePlugin::Info i;
	i.icon = QUrl("qrc:/Resources/delete.png");
	i.name = "CorePlugin";
	i.author = "italink";
	i.description = "this is a custom plugin";
	i.link = "github.com";
	return i;
}

void CorePlugin::startup() {
	qDebug() << "CorePlugin::startup";
}

void CorePlugin::shutdown() {
	qDebug() << "CorePlugin::shutdown";
}

QENGINE_IMPLEMENT_PLUGIN(CorePlugin,Core)
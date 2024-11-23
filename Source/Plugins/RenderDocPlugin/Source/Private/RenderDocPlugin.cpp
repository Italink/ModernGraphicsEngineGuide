#include "RenderDocPlugin.h"
#include <QDebug>
#include "renderdoc_app.h"
#include "QEngineCoreSignals.h"

RENDERDOC_API_1_1_2* rdoc_api = nullptr;

RenderDocPlugin::Info RenderDocPlugin::info()
{
	RenderDocPlugin::Info i;
	i.icon = QUrl("qrc:/Resources/delete.png");
	i.name = "CorePlugin";
	i.author = "italink";
	i.description = "this is a custom plugin";
	i.link = "github.com";
	return i;
}


void RenderDocPlugin::startup() {
	if (renderdoc.load()) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)renderdoc.resolve("RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
		if (ret != 1) {
			qWarning() << "RenderDoc Plugin Fail to Get API";
			return;
		}
		rdoc_api->MaskOverlayBits(eRENDERDOC_Overlay_None, 0);
		rdoc_api->SetCaptureOptionU32(RENDERDOC_CaptureOption::eRENDERDOC_Option_AllowVSync, 0);
		rdoc_api->SetCaptureOptionU32(RENDERDOC_CaptureOption::eRENDERDOC_Option_AllowFullscreen, 1);
		rdoc_api->SetCaptureOptionU32(RENDERDOC_CaptureOption::eRENDERDOC_Option_DebugOutputMute, 0);
		QObject::connect(QEngineCoreSignals::Instance(), &QEngineCoreSignals::asViewportKeyPressEvent, [this](QKeyEvent* event) {
			if (event->key() == Qt::Key_C) {
				rdoc_api->TriggerCapture();
				if (!rdoc_api->IsTargetControlConnected())
					rdoc_api->LaunchReplayUI(1, nullptr);
			}
		});
	}
	else {
		qWarning() << "Render Doc Load Failed: " << renderdoc.errorString();
	}
}

void RenderDocPlugin::shutdown() {

}

QENGINE_IMPLEMENT_PLUGIN(RenderDocPlugin, RenderDocPlugin)
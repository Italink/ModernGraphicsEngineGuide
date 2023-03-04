#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
#include "Render/RenderComponent/QStaticMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent((new QStaticMeshRenderComponent)
				->setupStaticMeshPath(RESOURCE_DIR"/Genji/Genji.FBX")
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


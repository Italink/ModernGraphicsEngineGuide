#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Component/QStaticMeshRenderComponent.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("StaticMesh", (new QSceneForwardRenderPass())
			->addRenderComponent((new QStaticMeshRenderComponent)
				->setupStaticMeshPath(RESOURCE_DIR"/Genji/Genji.FBX")
			)
		)
		->end("StaticMesh", QSceneForwardRenderPass::BaseColor)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


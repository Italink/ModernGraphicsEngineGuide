#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Component/QSkyboxRenderComponent.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	//initParams.backend = QRhi::Implementation::D3D11;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Skybox", (new QSceneForwardRenderPass())
			->addRenderComponent((new QSkyboxRenderComponent())
				->setupSkyBoxImage(QImage(RESOURCE_DIR"/Skybox.jpeg"))
			)
		)
		->end("Skybox", QSceneForwardRenderPass::BaseColor)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

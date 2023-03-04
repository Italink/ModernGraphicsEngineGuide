#include <QApplication>
#include "QRenderWidget.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
#include "Render/RenderComponent/QParticlesRenderComponent.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent((new QParticlesRenderComponent())
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

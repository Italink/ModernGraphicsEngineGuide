#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Component/QParticlesRenderComponent.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera();

	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Particles", (new QSceneForwardRenderPass())
			->addRenderComponent(
				(new QParticlesRenderComponent())
			)
		)
		->end("Particles",QSceneForwardRenderPass::BaseColor)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

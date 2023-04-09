#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QParticlesRenderComponent.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera();

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QParticlesRenderComponent::Create("Particles")
			)
		)
		.end("BasePass",QBasePassForward::BaseColor)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

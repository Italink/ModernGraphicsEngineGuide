#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/Derived/QSpectrumRenderComponent.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	QRenderWidget widget(initParams);

	widget.setupCamera();

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QSpectrumRenderComponent::Create("Spectrum")
				.setAudio(RESOURCE_DIR"/Audio/MySunset.mp3")
				.setBarCount(1000)
				.setTranslate(QVector3D(0,-0.5,0))
			)
		)
		.end("BasePass", QBasePassForward::Out::BaseColor)
	);

	widget.showMaximized();
	return app.exec();
}

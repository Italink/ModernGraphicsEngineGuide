#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QSkyRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass
		(
			QSkyRenderPass::Create("Sky")
			.setSkyBoxImagePath(RESOURCE_DIR"/Image/environment.hdr")
		)
		.end("Sky", QSkyRenderPass::Out::SkyTexture)
	);

	widget.showMaximized();
	return app.exec();
}

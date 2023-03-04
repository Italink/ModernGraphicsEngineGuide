#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QImGUIRenderPass.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Scene", (new QImGUIRenderPass())
			->setupPaintFunctor([]() {
				ImGui::ShowFontSelector("Font");
				ImGui::ShowStyleSelector("Style");
				ImGui::ShowDemoWindow();
			})
		)
		->end("Scene", QImGUIRenderPass::OutSlot::UiLayer)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QImGUIRenderPass.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QImGUIRenderPass::Create("ImGui")
			.setPaintFunctor([](ImGuiContext* Ctx) {
				ImGui::SetCurrentContext(Ctx);
				ImGui::ShowFontSelector("Font");
				ImGui::ShowStyleSelector("Style");
				ImGui::ShowDemoWindow();
			})
		)
		.end()
	);

	widget.showMaximized();
	return app.exec();
}


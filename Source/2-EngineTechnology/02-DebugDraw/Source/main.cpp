#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/IRenderer.h"
#include "Render/RenderGraph/PassBuilder/QImGUIPassBuilder.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"

class MyRenderer : public IRenderer {
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QImGUIPassBuilder::Output imguiOut = graphBuilder.addPassBuilder<QImGUIPassBuilder>("MeshPass").
			setPaintFunctor([](ImGuiContext* Ctx) {
				ImGui::SetCurrentContext(Ctx);
				ImGui::ShowStyleSelector("Style");
				ImGui::ShowDemoWindow();
			});

		QOutputPassBuilder::Output ret = graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(imguiOut.ImGuiTexture);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}


#include <QApplication>
#include "QRenderWidget.h"
#include "Render/RenderGraph/PassBuilder/QSkyPassBuilder.h"
#include "QEngineApplication.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QSharedPointer<QSkyPassBuilder> mSkyPass{ new QSkyPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mSkyPass->setSkyBoxImageByPath("Resources/Image/environment.hdr");
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QSkyPassBuilder::Output skyOut
			= graphBuilder.addPassBuilder("SkyPass", mSkyPass);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(skyOut.SkyTexture);

	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}
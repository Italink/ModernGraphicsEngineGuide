#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/Component/Derived/QSpectrumRenderComponent.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QSpectrumRenderComponent mSpectrumComp;
	QSharedPointer<QMeshPassBuilder> mMeshPass{ new QMeshPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mSpectrumComp.setAudio("Resources/Audio/MySunset.mp3");
		mSpectrumComp.setBarCount(1000);
		mSpectrumComp.setTranslate(QVector3D(0, -0.5, 0));

		addComponent(&mSpectrumComp);
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {

		QMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder("MeshPass", mMeshPass);


		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(meshOut.BaseColor);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}

#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QTextureCopyPassBuilder.h"
#include "QtConcurrent/qtconcurrentrun.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent mComp;
	IMeshPassBuilder mMeshPass;
	QTexutreCopyPassBuilder TextureCopyPass;
public:
	MyRenderer()
		: IRenderer(
			QRhiHelper::InitParamsBuilder()
			.backend(QRhi::Implementation::Vulkan)
		) {

		QtConcurrent::run([this]() {
			mComp.setStaticMesh(QStaticMesh::CreateFromFile(R"(E:\ModernGraphicsEngineGuide\Source\Resources\Model\mandalorian\scene.gltf)"));
			});
	}
public:
protected:
	void setupGraph(QRGBuilder& graphBuilder) override {
		IMeshPassBuilder::InputParams ia;
		ia.components = { &mComp };
		auto oa = graphBuilder.addPassBuilder(&mMeshPass, ia);
		QTexutreCopyPassBuilder::InputParams ib;
		ib.SrcTexture = oa.baseColor;
		ib.DstRenderTarget = graphBuilder.mainRenderTarget();
		graphBuilder.addPassBuilder(&TextureCopyPass, ib);
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;

	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}


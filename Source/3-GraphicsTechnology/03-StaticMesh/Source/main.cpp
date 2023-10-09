#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent mStaticComp;
	QSharedPointer<QMeshPassBuilder> mMeshPass{ new QMeshPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		QtConcurrent::run([this]() {
			mStaticComp.setStaticMesh(QStaticMesh::CreateFromFile("Resources/Model/mandalorian_ship/scene.gltf"));
		});

		addComponent(&mStaticComp);
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

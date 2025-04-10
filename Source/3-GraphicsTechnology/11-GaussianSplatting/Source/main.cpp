#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/Component/QGaussianSplattingPointCloudRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QGaussianSplattingPointCloudRenderComponent m3DGSComp;
	QSharedPointer<QGaussianSplattingPointCloud> GaussianSplattingPointCloudAsyncLoader;
	QSharedPointer<QMeshPassBuilder> mMeshPass{ new QMeshPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		GaussianSplattingPointCloudAsyncLoader = QGaussianSplattingPointCloud::CreateFromFile("Resources/Model/ProjectTitan.ply");
		m3DGSComp.setGaussianSplattingPointCloud(GaussianSplattingPointCloudAsyncLoader);

		addComponent(&m3DGSComp);

		m3DGSComp.setRotation(QVector3D(-180, 0, 0));

		setCurrentObject(&m3DGSComp);

		getCamera()->setPosition(QVector3D(20, 15, 12));
		getCamera()->setRotation(QVector3D(-30, 145, 0));
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

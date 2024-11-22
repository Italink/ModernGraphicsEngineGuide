#include <QApplication>
#include "QtConcurrent/qtconcurrentrun.h"
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/RenderGraph/PassBuilder/PBR/QPbrMeshPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/PBR/QPbrLightingPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QSkyPassBuilder.h"
#include "QEngineApplication.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent mStaticComp;
	QSharedPointer<QPbrMeshPassBuilder> mMeshPass{ new QPbrMeshPassBuilder };
	QSharedPointer<QPbrLightingPassBuilder> mLightingPass{ new QPbrLightingPassBuilder };
	QSharedPointer<QSkyPassBuilder> mSkyPass{ new QSkyPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mStaticComp.setStaticMesh(QStaticMesh::CreateFromFile("Resources/Model/mandalorian_ship/scene.gltf"));
		mStaticComp.setRotation(QVector3D(-90, 0, 0));

		mSkyPass->setSkyBoxImageByPath("Resources/Image/environment.hdr");

		addComponent(&mStaticComp);

		setCurrentObject(&mStaticComp);

		getCamera()->setPosition(QVector3D(20, 15, 12));
		getCamera()->setRotation(QVector3D(-30, 145, 0));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QSkyPassBuilder::Output skyOut
			= graphBuilder.addPassBuilder("SkyPass", mSkyPass);

		QPbrMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder("PbrMeshPass",mMeshPass);

		QPbrLightingPassBuilder::Output lightingOut
			= graphBuilder.addPassBuilder("LightingPass", mLightingPass)
			.setBaseColor(meshOut.BaseColor)
			.setMetallic(meshOut.Metallic)
			.setNormal(meshOut.Normal)
			.setPosition(meshOut.Position)
			.setRoughness(meshOut.Roughness)
			.setSkyCube(skyOut.SkyCube)
			.setSkyTexture(skyOut.SkyTexture);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(lightingOut.LightingResult);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}
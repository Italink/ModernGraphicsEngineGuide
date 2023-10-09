#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QBlurPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QPixelFilterPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QBloomPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QToneMappingPassBuilder.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/Component/QStaticMeshRenderComponent.h"

#define Q_PROPERTY_VAR(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
    } \
    Type Name

class MyRenderer : public IRenderer {
	Q_OBJECT
	Q_PROPERTY_VAR(int, BlurIterations) = 2;
	Q_PROPERTY_VAR(int, BlurSize) = 20;
	Q_PROPERTY_VAR(int, DownSampleCount) = 4;

	Q_PROPERTY_VAR(float, Gamma) = 2.2f;
	Q_PROPERTY_VAR(float, Exposure) = 1.f;
	Q_PROPERTY_VAR(float, PureWhite) = 1.f;

	Q_CLASSINFO("BlurIterations", "Min=1,Max=8")
	Q_CLASSINFO("BlurSize", "Min=1,Max=80")
	Q_CLASSINFO("DownSampleCount", "Min=1,Max=16")
private:
	QStaticMeshRenderComponent mStaticComp;
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mStaticComp.setStaticMesh(QStaticMesh::CreateFromFile("Resources/Model/mandalorian_ship/scene.gltf"));
		mStaticComp.setRotation(QVector3D(-90, 0, 0));

		addComponent(&mStaticComp);

		getCamera()->setPosition(QVector3D(20, 15, 12));
		getCamera()->setRotation(QVector3D(-30, 145, 0));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder<QMeshPassBuilder>("MeshPass");

		QPixelFilterPassBuilder::Output filterOut = graphBuilder.addPassBuilder<QPixelFilterPassBuilder>("FilterPass")
			.setBaseColorTexture(meshOut.BaseColor)
			.setFilterCode(R"(
				const float threshold = 0.5f;
				void main() {
					vec4 color = texture(uTexture, vUV);
					float value = max(max(color.r,color.g),color.b);
					outFragColor = (1-step(value, threshold)) * color;
				}
			)");

		QBlurPassBuilder::Output blurOut = graphBuilder.addPassBuilder<QBlurPassBuilder>("BlurPass")
			.setBaseColorTexture(filterOut.FilterResult)
			.setBlurIterations(BlurIterations)
			.setBlurSize(BlurSize)
			.setDownSampleCount(DownSampleCount);

		QBloomPassBuilder::Output bloomOut = graphBuilder.addPassBuilder<QBloomPassBuilder>("BloomPass")
			.setBaseColorTexture(meshOut.BaseColor)
			.setBlurTexture(blurOut.BlurResult);

		QToneMappingPassBuilder::Output tonemappingOut = graphBuilder.addPassBuilder<QToneMappingPassBuilder>("ToneMappingPass")
			.setBaseColorTexture(bloomOut.BloomResult)
			.setExposure(Exposure)
			.setGamma(Gamma)
			.setPureWhite(PureWhite);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(tonemappingOut.ToneMappingReslut);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}

#include "main.moc"
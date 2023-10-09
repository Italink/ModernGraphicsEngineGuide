#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/PBR/QPbrMeshPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QOutliningPassBuilder.h"

#define Q_PROPERTY_VAR(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
    } \
    Type Name

class MyRenderer : public IRenderer {
	Q_OBJECT
	Q_PROPERTY_VAR(int, Radius) = 2;
	Q_PROPERTY_VAR(QColor4D, ColorModifier) = QColor4D(0.324f, 0.063f, 0.099f, 1.0f);
	Q_PROPERTY_VAR(float, MinSeparation) = 1.0f;
	Q_PROPERTY_VAR(float, MaxSeparation) = 3.0f;
	Q_PROPERTY_VAR(float, MinDistance) = 0.5f;
	Q_PROPERTY_VAR(float, MaxDistance) = 2.0f;
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
		QPbrMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder<QPbrMeshPassBuilder>("MeshPass");

		QOutliningPassBuilder::Output outliningOut = graphBuilder.addPassBuilder<QOutliningPassBuilder>("OutliningPass")
			.setBaseColorTexture(meshOut.BaseColor)
			.setPositionTexture(meshOut.Position)
			.setRadius(Radius)
			.setColorModifier(ColorModifier)
			.setMinDistance(MinDistance)
			.setMaxDistance(MaxDistance)
			.setMinSeparation(MinSeparation)
			.setMaxSeparation(MaxSeparation)
			;

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(outliningOut.OutliningReslut);
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
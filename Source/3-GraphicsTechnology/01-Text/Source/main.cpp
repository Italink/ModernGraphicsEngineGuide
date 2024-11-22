#include "QEngineApplication.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent mTextTextureComp;
	QStaticMeshRenderComponent mTextMeshComp;
	QSharedPointer<QMeshPassBuilder> mMeshPass{ new QMeshPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mTextMeshComp.setStaticMesh(QStaticMesh::CreateFromText("TextMesh", QFont("微软雅黑", 64), Qt::white, Qt::Horizontal, 2, false));
		addComponent(&mTextMeshComp);
		
		mTextTextureComp.setStaticMesh(QStaticMesh::CreateFromText("TextTexture", QFont("微软雅黑", 64), Qt::white, Qt::Horizontal, 2, true));
		mTextTextureComp.setTranslate(QVector3D(0.0f, -100.0f, 0.0f));
		addComponent(&mTextTextureComp);

		setCurrentObject(&mTextMeshComp);

		getCamera()->setRotation(QVector3D(0, 90, 0));
		getCamera()->setPosition(QVector3D(0, 0, 500));
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

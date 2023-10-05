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
		//QtConcurrent::run([this]() {

		//});
		mTextTextureComp.setStaticMesh(QStaticMesh::CreateFromText("TextTexture", QFont("微软雅黑", 64), Qt::white, Qt::Horizontal, 2, true));
		mTextMeshComp.setStaticMesh(QStaticMesh::CreateFromText("TextMesh", QFont("微软雅黑", 64), Qt::white, Qt::Horizontal, 2, false));
		addComponent(&mTextTextureComp);
		addComponent(&mTextMeshComp);
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

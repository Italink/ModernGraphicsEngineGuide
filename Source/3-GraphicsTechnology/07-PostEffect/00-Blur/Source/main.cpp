#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/QBlurRenderPass.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera()
		->setPosition(QVector3D(0,0,25));
	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian/scene.gltf"))
				.setTranslate(QVector3D(0,-5,0))
				.setRotation(QVector3D(-90,0,0))
			)
		)
		.addPass(
			QBlurRenderPass::Create("Blur")
			.setBlurIterations(2)
			.setBlurSize(10)
			.setDownSampleCount(4)
			.setTextureIn_Src( "BasePass", QBasePassForward::Out::BaseColor)
		)
		.end("Blur", QBlurRenderPass::Result)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


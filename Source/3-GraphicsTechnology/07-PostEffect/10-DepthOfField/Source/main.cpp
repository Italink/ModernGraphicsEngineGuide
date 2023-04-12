#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/QDilationRenderPass.h"
#include "Render/Pass/QDepthOfFieldRenderPass.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera()
		->setPosition(QVector3D(0, 0, 25));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QPbrBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian/scene.gltf"))
				.setTranslate(QVector3D(0, -5, 0))
				.setRotation(QVector3D(-90, 0, 0))
			)
		)
		.addPass(
			QDilationRenderPass::Create("Dilation")
			.setTextureIn_Src("BasePass", QPbrBasePassDeferred::Out::BaseColor)
		)
		.addPass(
			QDepthOfFieldRenderPass::Create("DepthOfField")
			.setTextureIn_Focus("BasePass", QPbrBasePassDeferred::Out::BaseColor)
			.setTextureIn_LoseFocus("Dilation", QDilationRenderPass::Out::Result)
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
			.setNear(1)
			.setFar(2)
		)
		.end("DepthOfField", QDepthOfFieldRenderPass::Result)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


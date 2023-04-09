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
	widget.setupCamera();

	auto StaticMesh = QStaticMesh::CreateFromFile(RESOURCE_DIR"/cerberus.fbx");

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QPbrBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(StaticMesh)
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
		)
		.end("DepthOfField", QDepthOfFieldRenderPass::Result)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


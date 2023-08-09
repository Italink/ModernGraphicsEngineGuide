#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"
#include "Render/Pass/QSsaoRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"

int main(int argc, char **argv){
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(20, 15, 12));
	camera->setRotation(QVector3D(-30, 145, 0));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QPbrBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian_ship/scene.gltf"))
				.setRotation(QVector3D(-90, 0, 0))
			)
		)
		.addPass(
			QSsaoRenderPass::Create("SSAO")
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
			.setTextureIn_Normal("BasePass", QPbrBasePassDeferred::Out::Normal)
		)
		.addPass(
			QBlurRenderPass::Create("Blur")
			.setBlurIterations(1)
			.setBlurSize(4)
			.setDownSampleCount(2)
			.setTextureIn_Src( "SSAO", QSsaoRenderPass::Out::Result)
		)
		.end("Blur", QBlurRenderPass::Result)
	);

	widget.showMaximized();
	return app.exec();
}


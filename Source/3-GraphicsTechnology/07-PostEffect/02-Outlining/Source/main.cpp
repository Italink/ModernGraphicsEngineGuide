#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/QOutliningRenderPass.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"

int main(int argc, char** argv) {
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
			QOutliningRenderPass::Create("Outlining")
			.setTextureIn_BaseColor("BasePass", QPbrBasePassDeferred::Out::BaseColor)
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
		)
		.end("Outlining", QOutliningRenderPass::Result)
	);

	widget.showMaximized();
	return app.exec();
}


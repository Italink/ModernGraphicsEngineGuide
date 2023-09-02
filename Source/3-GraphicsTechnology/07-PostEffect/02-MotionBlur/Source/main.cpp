#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/QMotionBlurRenderPass.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);
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
			QMotionBlurRenderPass::Create("MotionBlur")
			.setTextureIn_BaseColor("BasePass", QPbrBasePassDeferred::Out::BaseColor)
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
		)
		.end("MotionBlur", QMotionBlurRenderPass::Result)
	);

	widget.showMaximized();
	return app.exec();
}


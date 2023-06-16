#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "QEngineApplication.h"

int main(int argc, char **argv){
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(30,25,20));
	camera->setRotation(QVector3D(-30,145,0));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian_ship/scene.gltf"))
				.setRotation(QVector3D(-90,0,0))
			)
		)
		.end("BasePass", QBasePassForward::BaseColor)
	);

	widget.showMaximized();
	return app.exec();
}


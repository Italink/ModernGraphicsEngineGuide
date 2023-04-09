#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QSkeletalMeshRenderComponent.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(0, 190, -700));
	camera->setRotation(QVector3D(-5, 265, 0));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QSkeletalMeshRenderComponent::Create("SkeletalMesh")
				.setSkeletalMesh(QSkeletalMesh::CreateFromFile(RESOURCE_DIR"/Model/Catwalk Walk Turn 180 Tight R.fbx"))
			)
		)
		.end("BasePass", QBasePassForward::BaseColor)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

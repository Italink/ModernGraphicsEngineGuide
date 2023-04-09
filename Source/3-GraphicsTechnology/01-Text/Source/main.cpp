#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera()
		->setPosition(QVector3D(0,0, 2000));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("TextTexture")
				.setStaticMesh(QStaticMesh::CreateFromText("TextTexture",QFont("微软雅黑",64), Qt::white, Qt::Horizontal, 2, true))
				.setTranslate(QVector3D(0, 100, 0))
			)
			.addComponent(
				QStaticMeshRenderComponent::Create("TextMesh")
				.setStaticMesh(QStaticMesh::CreateFromText("TextMesh", QFont("微软雅黑", 64), Qt::white, Qt::Horizontal, 2, false))
				.setTranslate(QVector3D(0,-100,0))
			)
		)
		.end("BasePass", QBasePassForward::BaseColor)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


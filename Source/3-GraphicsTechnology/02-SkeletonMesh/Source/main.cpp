#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Component/QSkeletalMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	QRenderWidget widget(initParams);
	QCamera* camera = widget.setupCamera();
	camera->setPosition(QVector3D(725.0f, 300.0f, 225.0f));
	camera->setRotation(QVector3D(-0.225f, 3.0f, 0.0f));
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("SkeletalMesh", (new QSceneForwardRenderPass())
			->addRenderComponent((new QSkeletalMeshRenderComponent)
				->setupSkeletalMeshPath(RESOURCE_DIR"/Catwalk Walk Turn 180 Tight R.fbx")
			)
		)
		->end("SkeletalMesh",QSceneForwardRenderPass::BaseColor)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

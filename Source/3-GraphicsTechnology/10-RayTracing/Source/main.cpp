#include <QApplication>
#include "Asset/QStaticMesh.h"
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::D3D12;
	QRenderWidget widget(initParams);

	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(20, 15, 12));
	camera->setRotation(QVector3D(-30, 145, 0));

	auto staticMesh = QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian_ship/scene.gltf");

	widget.resize({ 800,600 });
	widget.show();

	return app.exec();
}
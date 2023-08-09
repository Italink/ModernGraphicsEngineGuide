#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Asset/QStaticMesh.h"
#include "Render/Component/Light/QPointLightComponent.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/BlinnPhong/QBlinnPhongBasePassDeferred.h"
#include "Render/Pass/BlinnPhong/QBlinnPhongLightingPass.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QSkyRenderPass.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);

	QRhiWindow::InitParams initParams;

	QRenderWidget widget(initParams);
	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(20, 15, 12));
	camera->setRotation(QVector3D(-30, 145, 0));

	auto staticMesh = QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian_ship/scene.gltf");

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QSkyRenderPass::Create("Sky")
			.setSkyBoxImagePath(RESOURCE_DIR"/Image/environment.hdr")
		)
		.addPass(
			QBlinnPhongBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(staticMesh)
				.setRotation(QVector3D(-90,0,0))
			)
			.addComponent(
				QPointLightComponent::Create("PointLight")
			)
		)
		.addPass(
			QBlinnPhongLightingPass::Create("Lighting")
			.setTextureIn_Albedo("BasePass", QBlinnPhongBasePassDeferred::Out::BaseColor)
			.setTextureIn_Position("BasePass", QBlinnPhongBasePassDeferred::Out::Position)
			.setTextureIn_Normal("BasePass", QBlinnPhongBasePassDeferred::Out::Normal)
			.setTextureIn_Specular("BasePass", QBlinnPhongBasePassDeferred::Out::Specular)
			.setTextureIn_SkyTexture("Sky", QSkyRenderPass::Out::SkyTexture)
		)
		.end("Lighting", QBlinnPhongLightingPass::Out::FragColor)
	);

	widget.showMaximized();

	return app.exec();
}
#include <QApplication>
#include "Asset/QStaticMesh.h"
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/PBR/QPbrLightingPass.h"
#include "Render/Pass/QSkyRenderPass.h"
#include "Render/Component/Derived/QSpectrumRenderComponent.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");

	QApplication app(argc, argv);

	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
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
			QPbrBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(staticMesh)
				.setRotation(QVector3D(-90,0,0))
			)
		)
		.addPass(
			QPbrLightingPass::Create("Lighting")
			.setTextureIn_Albedo("BasePass", QPbrBasePassDeferred::Out::BaseColor)
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
			.setTextureIn_Normal("BasePass", QPbrBasePassDeferred::Out::Normal)
			.setTextureIn_Metallic("BasePass", QPbrBasePassDeferred::Out::Metallic)
			.setTextureIn_Roughness("BasePass", QPbrBasePassDeferred::Out::Roughness)
			.setTextureIn_SkyTexture("Sky", QSkyRenderPass::Out::SkyTexture)
			.setTextureIn_SkyCube("Sky", QSkyRenderPass::Out::SkyCube)
		)
		.end("Lighting", QPbrLightingPass::Out::FragColor)
	);

	widget.resize({ 800,600 });
	widget.show();

	return app.exec();
}
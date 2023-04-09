#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Component/QSkeletalMeshRenderComponent.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera()
		->setPosition(QVector3D(0,100,800));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Genji/Genji.FBX"))
				.setTranslate(QVector3D(-200, 0, 0))
				.setScale3D(QVector3D(10, 10, 10))
			)
			.addComponent(
				QSkeletalMeshRenderComponent::Create("SkeletalMesh")
				.setSkeletalMesh(QSkeletalMesh::CreateFromFile(RESOURCE_DIR"/Catwalk Walk Turn 180 Tight R.fbx"))
				.setTranslate(QVector3D(200, 0, 0))
			)
			.addComponent(
				QParticlesRenderComponent::Create("GPU Particles")
			)
		)
		.addPass(
			QPixelFilterRenderPass::Create("BrightPixels")
			.setTextureIn_Src("BasePass",QBasePassForward::Out::BaseColor)
			.setFilterCode(R"(
				const float threshold = 1.0f;
				void main() {
					vec4 color = texture(uTexture, vUV);
					float value = max(max(color.r,color.g),color.b);
					outFragColor = (1-step(value, threshold)) * color * 100;
				}
			)")
		)
		.addPass(
			QBlurRenderPass::Create("Blur")
			.setBlurIterations(1)
			.setTextureIn_Src("BrightPixels", QPixelFilterRenderPass::Out::Result)
		)
		.addPass(
			QBloomRenderPass::Create("ToneMapping")
			.setTextureIn_Raw("BasePass", QBasePassForward::Out::BaseColor)
			.setTextureIn_Blur("BloomBlur", QBlurRenderPass::Out::Result)
		)
		.end("ToneMapping", QBloomRenderPass::Out::Result)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


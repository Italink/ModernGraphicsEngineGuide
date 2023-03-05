#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Component/QSkyboxRenderComponent.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Component/QSkeletalMeshRenderComponent.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomMerageRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera()
		->setPosition(QVector3D(0,100,800));

	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Scene", (new QSceneForwardRenderPass())
			->addRenderComponent((new QSkyboxRenderComponent)
				->setupSkyBoxImage(QImage(RESOURCE_DIR"/Skybox.jpeg"))
			)
			->addRenderComponent((new QStaticMeshRenderComponent)
				->setupStaticMeshPath(RESOURCE_DIR"/Genji/Genji.FBX")
				->setTranslate(QVector3D(-200, 0, 0))
				->setScale3D(QVector3D(10, 10, 10))
			)
			->addRenderComponent((new QSkeletalMeshRenderComponent)
				->setupSkeletalMeshPath(RESOURCE_DIR"/Catwalk Walk Turn 180 Tight R.fbx")
				->setTranslate(QVector3D(200, 0, 0))
			)
			->addRenderComponent((new QParticlesRenderComponent))
		)
		->addPass("BrightPixel", (new QPixelFilterRenderPass)
			->setupDownSample(2)
			->setupFilterCode(R"(
				const float threshold = 1.0f;
				void main() {
					vec4 color = texture(uTexture, vUV);
					float value = max(max(color.r,color.g),color.b);
					outFragColor = (1-step(value, threshold)) * color * 100;
				}
			)")
			->setupInputTexture(QPixelFilterRenderPass::InSlot::Src, "Scene", QSceneForwardRenderPass::OutSlot::BaseColor)
		)
		->addPass("BloomBlur", (new QBlurRenderPass)
			->setupBlurIter(1)
			->setupInputTexture(QBlurRenderPass::InpSlot::Src, "BrightPixel", QPixelFilterRenderPass::OutSlot::Result)
		)
		->addPass("ToneMapping", (new QBloomMerageRenderPass)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Raw, "Scene", QSceneForwardRenderPass::OutSlot::BaseColor)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Blur, "BloomBlur", QBlurRenderPass::OutSlot::Result)
		)
		->end("ToneMapping", QBloomMerageRenderPass::OutSlot::Result)
		);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}


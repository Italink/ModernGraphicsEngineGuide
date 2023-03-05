#include <QApplication>
#include "QRenderWidget.h"
#include "Render/ComponentS/QParticlesRenderComponent.h"
#include "Render/Pass/QSceneForwardRenderPass.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomMerageRenderPass.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("FloatTexture", (new QSceneForwardRenderPass)
			->addRenderComponent((new QParticlesRenderComponent)
				->setupColor(QColor4D(0.2,1,1.8))
			)
		)
		->addPass("BrightPixelFilter", (new QPixelFilterRenderPass)
			->setupDownSample(2)
			->setupFilterCode(R"(
				const float threshold = 1.0f;
				void main() {
					vec4 color = texture(uTexture, vUV);
					float value = max(max(color.r,color.g),color.b);
					outFragColor = (1-step(value, threshold)) * color * 100;
				}
			)")
			->setupInputTexture(QPixelFilterRenderPass::InSlot::Src, "FloatTexture", QSceneForwardRenderPass::OutSlot::BaseColor)
		)
		->addPass("BloomBlur",(new QBlurRenderPass)
			->setupBlurIter(1)
			->setupInputTexture(QBlurRenderPass::InpSlot::Src,"BrightPixelFilter",QPixelFilterRenderPass::OutSlot::Result)
		)
		->addPass("BloomMerage", (new QBloomMerageRenderPass)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Raw, "FloatTexture", QSceneForwardRenderPass::OutSlot::BaseColor)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Blur, "BloomBlur", QBlurRenderPass::OutSlot::Result)
		)
		->end("BloomMerage", QBloomMerageRenderPass::OutSlot::Result)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomRenderPass.h"
#include "Render/Pass/QToneMappingRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QParticlesRenderComponent::Create("GPU Particles")
				.setColor(QColor4D(0.2, 1, 1.8))
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
			QBloomRenderPass::Create("Bloom")
			.setTextureIn_Raw("BasePass", QBasePassForward::Out::BaseColor)
			.setTextureIn_Blur("Blur", QBlurRenderPass::Out::Result)
		)
		.addPass(
			QToneMappingRenderPass::Create("ToneMapping")
			.setTextureIn_Src("Bloom", QBloomRenderPass::Out::Result)
		)
		.end("ToneMapping", QToneMappingRenderPass::Out::Result)
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

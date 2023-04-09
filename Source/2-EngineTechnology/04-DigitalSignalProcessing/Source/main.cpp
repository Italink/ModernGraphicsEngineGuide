#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/Derived/QSpectrumRenderComponent.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomRenderPass.h"
#include "Render/Pass/QToneMappingRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	QRenderWidget widget(initParams);

	widget.setupCamera();

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QSpectrumRenderComponent::Create("Spectrum")
				.setAudio(RESOURCE_DIR"/Audio/MySunset.mp3")
				.setBarCount(1000)
				.setTranslate(QVector3D(0,-0.5,0))
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
					outFragColor = (1-step(value, threshold)) * color ;
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

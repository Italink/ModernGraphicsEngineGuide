#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QVideoRenderPass.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomRenderPass.h"
#include "Render/Pass/QToneMappingRenderPass.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QVideoRenderPass::Create("Video")
			.setVideoPath(QUrl::fromLocalFile(RESOURCE_DIR"/Video/BadApple.mp4"))
		)
		.addPass(
			QPixelFilterRenderPass::Create("Outline")
			.setTextureIn_Src("Video", QVideoRenderPass::Out::Output)
			.setFilterCode(R"(
				const float threshold = 0.05f;
				void main() {
					vec2 texOffset = 1.0 / textureSize(uTexture, 0);		
					int count = 0;
					float center = texture(uTexture, vUV).r;
					count += abs(texture(uTexture,vUV+vec2(texOffset.x,0)).r-center)>threshold ? 1 : 0;
					count += abs(texture(uTexture,vUV-vec2(texOffset.x,0)).r-center)>threshold ? 1 : 0;
					count += abs(texture(uTexture,vUV+vec2(0,texOffset.y)).r-center)>threshold ? 1 : 0;
					count += abs(texture(uTexture,vUV-vec2(0,texOffset.y)).r-center)>threshold ? 1 : 0;
					if(count>0&&count<5){
						outFragColor = vec4(vUV.x * count,vUV.y * count,0.5,1.0);
					}
					else{
						outFragColor = vec4(0);
					}
				}
			)")
		)
		.addPass(
			QBlurRenderPass::Create("Blur")
			.setBlurIterations(1)
			.setTextureIn_Src("Outline", QPixelFilterRenderPass::Out::Result)
		)
		.addPass(
			QToneMappingRenderPass::Create("ToneMapping")
			.setTextureIn_Src("Blur", QBlurRenderPass::Out::Result)
			.setExposure(7)
			.setGamma(0.5)
		)
		.end("ToneMapping", QToneMappingRenderPass::Out::Result)
	);

	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

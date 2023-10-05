#include <QApplication>
#include "Render/RHI/QRhiWindow.h"

class ComputeShaderWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiBuffer> mStorageBuffer;
	QScopedPointer<QRhiTexture> mTexture;

	QScopedPointer<QRhiComputePipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;

	QScopedPointer<QRhiSampler> mPaintSampler;
	QScopedPointer<QRhiShaderResourceBindings> mPaintShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPaintPipeline;

	const int ImageWidth = 64;
	const int ImageHeight = 64;
public:
	ComputeShaderWindow(QRhiHelper::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (mSigInit.ensure()) {
			mStorageBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, sizeof(float)));							//缓冲区被用作StorageBuffer
			mStorageBuffer->create();
			mTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(ImageWidth, ImageHeight), 1, QRhiTexture::UsedWithLoadStore));		//图像可被计算管线读取和存储
			mTexture->create();

			mPipeline.reset(mRhi->newComputePipeline());
			mShaderBindings.reset(mRhi->newShaderResourceBindings());
			mShaderBindings->setBindings({
				QRhiShaderResourceBinding::bufferLoadStore(0,QRhiShaderResourceBinding::ComputeStage,mStorageBuffer.get()),					//设置计算管线的资源绑定，Load代表可读，Store代表可写
				QRhiShaderResourceBinding::imageLoadStore(1,QRhiShaderResourceBinding::ComputeStage,mTexture.get(),0),
			});
			mShaderBindings->create();

			QShader cs = QRhiHelper::newShaderFromCode(QShader::ComputeStage, R"(#version 440
				layout(std140, binding = 0) buffer StorageBuffer{
					int counter;
				}SSBO;
				layout (binding = 1, rgba8) uniform image2D Tex;
				const int ImageSize = 64 * 64;
				void main(){
					//int currentCounter = SSBO.counter = SSBO.counter + 1;		
					int currentCounter = atomicAdd(SSBO.counter,1);			//use atomic operation
					ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
					imageStore(Tex,pos,vec4(currentCounter/float(ImageSize),0,0,1));
				}
			)");
			Q_ASSERT(cs.isValid());

			mPipeline->setShaderStage({
				QRhiShaderStage(QRhiShaderStage::Compute, cs),
			});

			mPipeline->setShaderResourceBindings(mShaderBindings.get());
			mPipeline->create();

			mPaintPipeline.reset(mRhi->newGraphicsPipeline());
			QRhiGraphicsPipeline::TargetBlend blendState;
			blendState.dstColor = QRhiGraphicsPipeline::One;
			blendState.srcColor = QRhiGraphicsPipeline::One;
			blendState.dstAlpha = QRhiGraphicsPipeline::One;
			blendState.srcAlpha = QRhiGraphicsPipeline::One;
			blendState.enable = true;
			mPaintPipeline->setTargetBlends({ blendState });
			mPaintPipeline->setSampleCount(currentRenderTarget->sampleCount());
			mPaintPipeline->setDepthTest(false);

			QString vsCode = R"(#version 450
				layout (location = 0) out vec2 vUV;
				out gl_PerVertex{
					vec4 gl_Position;
				};
				void main() {
					vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
					gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
					%1
				}
			)";
			QShader vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());

			QShader fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
				layout (binding = 0) uniform sampler2D uSamplerColor;
				layout (location = 0) in vec2 vUV;
				layout (location = 0) out vec4 outFragColor;
				void main() {
					outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
				}
			)");
			mPaintPipeline->setShaderStages({
				{ QRhiShaderStage::Vertex, vs },
				{ QRhiShaderStage::Fragment, fs }
				});

			mPaintSampler.reset(mRhi->newSampler(
				QRhiSampler::Filter::Linear,
				QRhiSampler::Filter::Linear,
				QRhiSampler::Filter::None,
				QRhiSampler::Repeat,
				QRhiSampler::Repeat,
				QRhiSampler::Repeat
			));
			mPaintSampler->create();

			mPaintShaderBindings.reset(mRhi->newShaderResourceBindings());
			mPaintShaderBindings->setBindings({
				QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mTexture.get(),mPaintSampler.get())
			});
			mPaintShaderBindings->create();
			mPaintPipeline->setShaderResourceBindings(mPaintShaderBindings.get());
			mPaintPipeline->setRenderPassDescriptor(currentRenderTarget->renderPassDescriptor());
			mPaintPipeline->create();
		}

		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		resourceUpdates = mRhi->nextResourceUpdateBatch();
		const int counter = 0;
		resourceUpdates->uploadStaticBuffer(mStorageBuffer.get(), &counter);
		cmdBuffer->beginComputePass(resourceUpdates);
		cmdBuffer->setComputePipeline(mPipeline.get());
		cmdBuffer->setShaderResources();
		cmdBuffer->dispatch(ImageWidth, ImageHeight, 1);		//根据图像大小划分工作组
		cmdBuffer->endComputePass();

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue);
		cmdBuffer->setGraphicsPipeline(mPaintPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, currentRenderTarget->pixelSize().width(), currentRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mPaintShaderBindings.get());
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
};

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiHelper::InitParams initParams;
    ComputeShaderWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}
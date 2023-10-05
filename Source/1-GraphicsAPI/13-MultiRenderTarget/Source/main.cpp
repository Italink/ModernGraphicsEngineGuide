#include <QApplication>

#include "Render/RHI/QRhiWindow.h"
#include "Render/RenderGraph/Painter/TexturePainter.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class MRTWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	QScopedPointer<QRhiTexture> mColorAttachment0;
	QScopedPointer<QRhiTexture> mColorAttachment1;
	QScopedPointer<QRhiRenderBuffer> mDepthStencilBuffer;

	QScopedPointer<QRhiTextureRenderTarget> mRenderTarget;
	QScopedPointer<QRhiRenderPassDescriptor> mRenderPassDesc;

	QScopedPointer<QRhiSampler> mPaintSampler;
	QScopedPointer<QRhiShaderResourceBindings> mPaintShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPaintPipeline;
public:
	MRTWindow(QRhiHelper::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();
		if (mSigInit.ensure()) {
			mColorAttachment0.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(100, 100), 1, QRhiTexture::Flag::RenderTarget | QRhiTexture::UsedAsTransferSource));				//创建颜色附件0
			mColorAttachment0->create();
			mColorAttachment1.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(100, 100), 1, QRhiTexture::Flag::RenderTarget | QRhiTexture::UsedAsTransferSource));				//创建颜色附件1
			mColorAttachment1->create();
			mDepthStencilBuffer.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, QSize(100, 100), 1, QRhiRenderBuffer::Flag(), QRhiTexture::Format::D24S8));	//创建深度（24位）模版（8位）附件
			mDepthStencilBuffer->create();

			QRhiTextureRenderTargetDescription rtDesc;																												
			rtDesc.setColorAttachments({ mColorAttachment0.get(),mColorAttachment1.get() });	
			rtDesc.setDepthStencilBuffer(mDepthStencilBuffer.get());
			mRenderTarget.reset(mRhi->newTextureRenderTarget(rtDesc));			

			//根据RenderTarget的结构来创建RenderPass描述，在使用GraphicsPipeline时，必须指定RenderPassDesc
			//这是因为流水线在创建时就需要明确它被用于何种结构的RenderPass，这里的结构指的是：RenderTarget的附件数量和格式
			mRenderPassDesc.reset(mRenderTarget->newCompatibleRenderPassDescriptor());														
			mRenderTarget->setRenderPassDescriptor(mRenderPassDesc.get());

			mRenderTarget->create();

			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
			mVertexBuffer->create();

			mShaderBindings.reset(mRhi->newShaderResourceBindings());
			mShaderBindings->create();

			mPipeline.reset(mRhi->newGraphicsPipeline());

			QRhiGraphicsPipeline::TargetBlend targetBlend;
			targetBlend.enable = false;
			mPipeline->setTargetBlends({ targetBlend,targetBlend });

			mPipeline->setSampleCount(mRenderTarget->sampleCount());

			mPipeline->setDepthTest(false);
			mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
			mPipeline->setDepthWrite(false);

			QShader vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, R"(#version 440
				layout(location = 0) in vec2 position;
				out gl_PerVertex { 
					vec4 gl_Position;
				};
				void main(){
					gl_Position = vec4(position,0.0f,1.0f);
				}
			)");
			Q_ASSERT(vs.isValid());

			QShader fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
				layout(location = 0) out vec4 fragColor0;		//输出到颜色附件0
				layout(location = 1) out vec4 fragColor1;		//输出到颜色附件1
				void main(){
					fragColor0 = vec4(1.0f,0.0f,0.0f,1.0f);
					fragColor1 = vec4(0.0f,0.0f,1.0f,1.0f);
				}
			)");
			Q_ASSERT(fs.isValid());

			mPipeline->setShaderStages({
				QRhiShaderStage(QRhiShaderStage::Vertex, vs),
				QRhiShaderStage(QRhiShaderStage::Fragment, fs)
				});

			QRhiVertexInputLayout inputLayout;
			inputLayout.setBindings({
				QRhiVertexInputBinding(2 * sizeof(float))
			});

			inputLayout.setAttributes({
				QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
			});

			mPipeline->setVertexInputLayout(inputLayout);
			mPipeline->setShaderResourceBindings(mShaderBindings.get());
			mPipeline->setRenderPassDescriptor(mRenderPassDesc.get());
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
			vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());

			fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
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
				QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mColorAttachment0.get(),mPaintSampler.get())
			});
			mPaintShaderBindings->create();
			mPaintPipeline->setShaderResourceBindings(mPaintShaderBindings.get());
			mPaintPipeline->setRenderPassDescriptor(currentRenderTarget->renderPassDescriptor());
			mPaintPipeline->create();
		}

		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (mSigSubmit.ensure()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		}

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(mRenderTarget.get(), clearColor, dsClearValue, resourceUpdates);			//该Pass会填充具有多个颜色附件的渲染目标

		cmdBuffer->setGraphicsPipeline(mPipeline.get());												
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings);
		cmdBuffer->draw(3);

		cmdBuffer->endPass();

		static int counter = 0;
		static QRhiTexture* CurrentTexture = nullptr;													//每60帧交替将颜色附件绘制到交换链上上
		if (counter && counter % 60 == 0) {
			if (!CurrentTexture || CurrentTexture == mColorAttachment1.get())
				CurrentTexture = mColorAttachment0.get();
			else {
				CurrentTexture = mColorAttachment1.get();
			}
			mPaintShaderBindings->setBindings({
				QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,CurrentTexture,mPaintSampler.get())		//更新纹理绑定
			});
			mPaintShaderBindings->create();
			counter = 0;
		}
		counter++;

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
    MRTWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}
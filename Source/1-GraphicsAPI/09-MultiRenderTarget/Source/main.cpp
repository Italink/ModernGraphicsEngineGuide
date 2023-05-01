#include <QApplication>

#include "Render/RHI/QRhiWindow.h"
#include "Render/Painter/TexturePainter.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class MRTWindow : public QRhiWindow {
private:
	QRhiEx::Signal mSigInit;
	QRhiEx::Signal mSigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	QScopedPointer<QRhiTexture> mTexture0;
	QScopedPointer<QRhiTexture> mTexture1;
	QScopedPointer<QRhiTextureRenderTarget> mRenderTarget;
	QScopedPointer<QRhiRenderPassDescriptor> mRenderPassDesc;

	QScopedPointer<TexturePainter> mTexturePainter;

public:
	MRTWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();
		if (mSigInit.ensure()) {
			initRhiResource();
		}
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (mSigSubmit.ensure()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		}

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(mRenderTarget.get(), clearColor, dsClearValue, resourceUpdates);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings);
		cmdBuffer->draw(3);

		cmdBuffer->endPass();

		static int counter = 0;
		static QRhiTexture* CurrentTexture = nullptr;
		if (counter && counter % 60 == 0) {
			if (!CurrentTexture || CurrentTexture == mTexture1.get())
				CurrentTexture = mTexture0.get();
			else {
				CurrentTexture = mTexture1.get();
			}
			mTexturePainter->setupTexture(CurrentTexture);
			mTexturePainter->compile();
			counter = 0;
		}
		counter++;

		cmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue);
		mTexturePainter->paint(cmdBuffer, currentRenderTarget);
		cmdBuffer->endPass();

	}

	void initRhiResource() {
		mTexture0.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(100, 100), 1, QRhiTexture::Flag::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mTexture1.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(100, 100), 1, QRhiTexture::Flag::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mTexture0->create();
		mTexture1->create();

		QRhiTextureRenderTargetDescription rtDesc;
		rtDesc.setColorAttachments({ mTexture0.get(),mTexture1.get() });
		mRenderTarget.reset(mRhi->newTextureRenderTarget(rtDesc));
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

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec2 position;
out gl_PerVertex { 
	vec4 gl_Position;
};
void main(){
    gl_Position = vec4(position,0.0f,1.0f);
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) out vec4 fragColor0;
layout(location = 1) out vec4 fragColor1;
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

		mTexturePainter.reset(new TexturePainter);
		mTexturePainter->setupRhi(mRhi.get());
		mTexturePainter->setupTexture(mTexture0.get());
		mTexturePainter->setupRenderPassDesc(mSwapChain->renderPassDescriptor());
		mTexturePainter->setupSampleCount(mSwapChain->sampleCount());
		mTexturePainter->compile();
	}
};

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    MRTWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}
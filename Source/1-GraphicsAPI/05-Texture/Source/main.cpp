#include <QApplication>
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {
	//position(xy)		texture coord(uv)
	 1.0f,   1.0f,		1.0f,  0.0f,
	-1.0f,   1.0f,		0.0f,  0.0f,
	 1.0f,  -1.0f,		1.0f,  1.0f,
	-1.0f,  -1.0f,		0.0f,  1.0f
};

class MyFirstTextureWindow : public QRhiWindow {
public:
	MyFirstTextureWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		sigInit.request();
	}
private:
	QRhiEx::Signal sigInit;
	QRhiEx::Signal sigSubmit;

	QImage mImage;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiTexture> mTexture;
	QScopedPointer<QRhiSampler> mSapmler;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
protected:
	void initRhiResource() {
		mImage = QImage(RESOURCE_DIR"/Image/Logo.png").convertedTo(QImage::Format_RGBA8888);

		mTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8, mImage.size()));
		mTexture->create();

		mSapmler.reset(mRhi->newSampler(
			QRhiSampler::Filter::Linear,
			QRhiSampler::Filter::Linear,
			QRhiSampler::Filter::Nearest,
			QRhiSampler::AddressMode::Repeat,
			QRhiSampler::AddressMode::Repeat,
			QRhiSampler::AddressMode::Repeat
		));
		mSapmler->create();

		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());

		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, mTexture.get(),mSapmler.get())
		});

		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(mSwapChain->sampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);
		mPipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec2 inPosition;
			layout(location = 1) in vec2 inUV;

			layout(location = 0) out vec2 vUV;

			out gl_PerVertex { vec4 gl_Position; };

			void main()
			{
				vUV = inUV;
				gl_Position = vec4(inPosition,0.0f,1.0f);
			}
		)");
		Q_ASSERT(vs.isValid());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout(location = 0) in vec2 vUV;
			layout(location = 0) out vec4 outFragColor;
			layout(binding = 0) uniform sampler2D inTexture;
			void main()
			{
				outFragColor = texture(inTexture,vUV);
			}
		)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
		});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			{ 4 * sizeof(float) }
		});
		inputLayout.setAttributes({
			{ 0, 0, QRhiVertexInputAttribute::Float2, 0 },
			{ 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }
		});

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
		mPipeline->create();

		sigSubmit.request();
	}

	void submitRhiData(QRhiResourceUpdateBatch* resourceUpdates) {
		resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		resourceUpdates->uploadTexture(mTexture.get(), mImage);
		qDebug() << mImage;
	}

	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* currentCmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (sigInit.ensure()) {
			initRhiResource();
		}

		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (sigSubmit.ensure()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			submitRhiData(resourceUpdates);
		}

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		currentCmdBuffer->setGraphicsPipeline(mPipeline.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		currentCmdBuffer->setVertexInput(0, 1, &vertexBindings);
		currentCmdBuffer->draw(4);

		currentCmdBuffer->endPass();
	}
};


int main(int argc, char **argv){
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);

    QRhiWindow::InitParams initParams;
    initParams.backend = QRhi::D3D11;
    MyFirstTextureWindow* window = new MyFirstTextureWindow(initParams);
	window->resize({ 800,600 });
	window->show();

    app.exec();
    delete window;
    return 0;
}
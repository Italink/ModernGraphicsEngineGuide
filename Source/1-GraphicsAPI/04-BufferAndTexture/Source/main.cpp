#include <QApplication>
#include "Render/RHI/QRhiWindow.h"
#include "QDateTime"

static float VertexData[] = {
	//position(xy)		texture coord(uv)
	 1.0f,   1.0f,		1.0f,  1.0f,
	 1.0f,  -1.0f,		1.0f,  0.0f,
	-1.0f,  -1.0f,		0.0f,  0.0f,
	-1.0f,   1.0f,		0.0f,  1.0f,
};

static uint32_t IndexData[] = {
	0,1,2,
	2,3,0
};

struct UniformBlock {
	QVector2D mousePos;
	float time;
};


class MyWindow : public QRhiWindow {
public:
	MyWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
	}
private:
	QRhiEx::Signal mSigInit;
	QRhiEx::Signal mSigSubmit;

	QImage mImage;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
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

		mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, sizeof(IndexData)));
		mIndexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mUniformBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());

		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, mTexture.get(),mSapmler.get()),
			QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::FragmentStage, mUniformBuffer.get())
		});

		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });
		mPipeline->setSampleCount(mSwapChain->sampleCount());
		mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

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
			layout(binding = 1) uniform UniformBlock{
				vec2 mousePos;
				float time;
			}UBO;
			void main()
			{
				const float speed = 5.0f;
				vec4 mixColor = vec4(1.0f) * sin(UBO.time * speed);
				outFragColor = mix(texture(inTexture,vUV), mixColor, distance(gl_FragCoord.xy,UBO.mousePos)/500);
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

		mSigSubmit.request();
	}

	virtual void onRenderTick() override {
		if (mSigInit.ensure()) {
			initRhiResource();
		}

		QRhiRenderTarget* renderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		if (mSigSubmit.ensure()) {
			batch->uploadStaticBuffer(mIndexBuffer.get(), IndexData);
			batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
			batch->uploadTexture(mTexture.get(), mImage);
		}
		UniformBlock ubo;
		ubo.mousePos = QVector2D(mapFromGlobal(QCursor::pos())) * qApp->devicePixelRatio();
		ubo.time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f;
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &ubo);

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, batch);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(6);
		cmdBuffer->endPass();
	}
};

int main(int argc, char **argv){
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);

    QRhiWindow::InitParams initParams;
    initParams.backend = QRhi::Vulkan;
    MyWindow* window = new MyWindow(initParams);
	window->resize({ 800,600 });
	window->show();

    app.exec();
    delete window;
    return 0;
}
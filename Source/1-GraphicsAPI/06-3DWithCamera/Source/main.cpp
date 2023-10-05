#include <QApplication>
#include "Render/RHI/QRhiWindow.h"
#include "QDateTime"
#include "Utils/QRhiCamera.h"

struct UniformBlock {
	QGenericMatrix<4, 4, float> MVP;
};

static float GridData[] = {
		//xyz					uv
		-1.0f,  0.0f, -1.0f,    0.0f  ,   0.0f,
		 1.0f,  0.0f, -1.0f,	100.0f,   0.0f,
		 1.0f,  0.0f,  1.0f,	100.0f, 100.0f,		//使用重复填充扩展UV
							
		 1.0f,  0.0f,  1.0f,	100.0f, 100.0f,
		-1.0f,  0.0f,  1.0f,	  0.0f, 100.0f,
		-1.0f,  0.0f, -1.0f,	  0.0f,   0.0f,
};

class MyWindow : public QRhiWindow {
public:
	MyWindow(QRhiHelper::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
	}
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiCamera> mCamera;
	QImage mImage;
	QScopedPointer<QRhiTexture> mTexture;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
protected:
	void initRhiResource() {
		mCamera.reset(new QRhiCamera);
		mCamera->setupRhi(mRhi.get());
		mCamera->setupWindow(this);
		mCamera->setPosition(QVector3D(0, 10, 0));

		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(GridData)));
		mVertexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mUniformBuffer->create();

		mSampler.reset(mRhi->newSampler(
			QRhiSampler::Filter::Linear,
			QRhiSampler::Filter::Nearest,
			QRhiSampler::Filter::Linear,
			QRhiSampler::AddressMode::Repeat,	
			QRhiSampler::AddressMode::Repeat,
			QRhiSampler::AddressMode::Repeat
		));
		mSampler->create();

		mImage = QImage(RESOURCE_DIR"/Image/Grid.png").convertedTo(QImage::Format_RGBA8888);
		mTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8, mImage.size(), 1 , QRhiTexture::Flag::MipMapped|QRhiTexture::UsedWithGenerateMips ));
		mTexture->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::StageFlag::VertexStage, mUniformBuffer.get()),
			QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::StageFlag::FragmentStage, mTexture.get(),mSampler.get())
		});
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		mPipeline->setSampleCount(mSwapChain->sampleCount());
		mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

		QShader vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec3 inPosition;
			layout(location = 1) in vec2 inUV;
			layout(location = 0) out vec2 vUV;
			layout(binding = 0) uniform UniformBlock{
				mat4 MVP;
			}UBO;

			out gl_PerVertex { vec4 gl_Position; };

			void main()
			{
				gl_Position = UBO.MVP * vec4(inPosition ,1.0f);
				vUV = inUV;
			}
		)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout(location = 0) in vec2 vUV;
			layout(location = 0) out vec4 outFragColor;
			layout(binding = 1) uniform sampler2D uTexture;
			void main()
			{
				outFragColor = texture(uTexture,vUV);
			}
		)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
		});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			{ 5 * sizeof(float) }
		});

		inputLayout.setAttributes({
			{ 0, 0, QRhiVertexInputAttribute::Float3, 0 },
			{ 0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float)}
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
			batch->uploadStaticBuffer(mVertexBuffer.get(), GridData);
			batch->uploadTexture(mTexture.get(), mImage);
			batch->generateMips(mTexture.get());
		}

		QMatrix4x4 project = mCamera->getProjectionMatrixWithCorr();

		QMatrix4x4 view = mCamera->getViewMatrix();

		QMatrix4x4 model;
		model.scale(10000);

		UniformBlock ubo;
		ubo.MVP = (project * view * model).toGenericMatrix<4, 4>();
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &ubo);
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, batch);
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings);
		cmdBuffer->draw(6);
		cmdBuffer->endPass();
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);

	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
	MyWindow* window = new MyWindow(initParams);
	window->resize({ 800,600 });
	window->show();

	app.exec();
	delete window;
	return 0;
}
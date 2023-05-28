#include <QApplication>
#include "Render/RHI/QRhiWindow.h"
#include "Utils/CubeData.h"
#include "QDateTime"
#include "Utils/QCamera.h"

struct UniformBlock {
	QGenericMatrix<4, 4, float> MVP;
};

class MyWindow : public QRhiWindow {
public:
	MyWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
	}
private:
	QRhiEx::Signal mSigInit;
	QRhiEx::Signal mSigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QCamera> mCamera;

	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

protected:
	void initRhiResource() {
		mCamera.reset(new QCamera);
		mCamera->setupWindow(this);
		mCamera->setPosition(QVector3D(0, 0, 10));


		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
		mVertexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mUniformBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::StageFlag::VertexStage, mUniformBuffer.get())
		});
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		mPipeline->setSampleCount(mSwapChain->sampleCount());
		mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec3 inPosition;
			layout(binding = 0) uniform UniformBlock{
				mat4 MVP;
			}UBO;
			out gl_PerVertex { vec4 gl_Position; };
			void main()
			{
				gl_Position = UBO.MVP * vec4(inPosition ,1.0f);
			}
		)");
		Q_ASSERT(vs.isValid());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout(location = 0) out vec4 outFragColor;
			void main()
			{
				outFragColor = vec4(1);
			}
		)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
		});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			{ 3 * sizeof(float) }
		});

		inputLayout.setAttributes({
			{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }
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
			batch->uploadStaticBuffer(mVertexBuffer.get(), CubeData);
		}

		QMatrix4x4 project = mCamera->getProjectionMatrixWithCorr(mRhi.get());

		QMatrix4x4 view = mCamera->getViewMatrix();

		float time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f;
		float factor = qAbs(qSin(time));
		QMatrix4x4 model;
		model.translate(QVector3D(0, factor * 5, 0));
		model.scale(factor);
		model.rotate(time * 180, QVector3D(1, 1, 1));

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
		cmdBuffer->draw(36);
		cmdBuffer->endPass();
	}
};

int main(int argc, char** argv) {
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
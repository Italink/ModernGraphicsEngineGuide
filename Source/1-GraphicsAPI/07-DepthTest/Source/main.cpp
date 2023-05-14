#include <QApplication>
#include "Render/RHI/QRhiWindow.h"
#include "Utils/CubeData.h"
#include "QDateTime"

struct UniformBlock {
	QVector4D color;
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

	QScopedPointer<QRhiBuffer> mCubeVertexBuffer;

	QScopedPointer<QRhiBuffer> mPlaneVertexBuffer;

	QScopedPointer<QRhiBuffer> mCubeUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mCubeShaderBindings;

	QScopedPointer<QRhiBuffer> mPlaneUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mPlaneShaderBindings;

	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
protected:
	void initRhiResource() {
		mCubeVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
		mCubeVertexBuffer->create();

		mPlaneVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(PlaneData)));
		mPlaneVertexBuffer->create();

		mCubeUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mCubeUniformBuffer->create();

		mCubeShaderBindings.reset(mRhi->newShaderResourceBindings());
		mCubeShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, mCubeUniformBuffer.get())
			});
		mCubeShaderBindings->create();

		mPlaneUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mPlaneUniformBuffer->create();

		mPlaneShaderBindings.reset(mRhi->newShaderResourceBindings());
		mPlaneShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, mPlaneUniformBuffer.get())
			});
		mPlaneShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());
		mPipeline->setSampleCount(mSwapChain->sampleCount());
		mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec3 inPosition;
			layout(binding = 0) uniform UniformBlock{
				vec4 color;
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
			layout(binding = 0) uniform UniformBlock{
				vec4 color;
				mat4 MVP;
			}UBO;
			void main()
			{
				outFragColor = UBO.color;
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
		mPipeline->setShaderResourceBindings(mCubeShaderBindings.get());
		mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
		mPipeline->setDepthTest(true);
		mPipeline->setDepthWrite(true);
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
			batch->uploadStaticBuffer(mCubeVertexBuffer.get(), CubeData);
			batch->uploadStaticBuffer(mPlaneVertexBuffer.get(), PlaneData);
		}

		QMatrix4x4 project = mRhi->clipSpaceCorrMatrix();

		project.perspective(90.0, renderTarget->pixelSize().width() / (float)renderTarget->pixelSize().height(), 0.01, 1000);
		QMatrix4x4 view;
		view.lookAt(QVector3D(5, 5, 0), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

		QMatrix4x4 model;
		UniformBlock ubo;
		ubo.color = QVector4D(0, 0, 1, 1);
		ubo.MVP = (project * view * model).toGenericMatrix<4, 4>();
		batch->updateDynamicBuffer(mCubeUniformBuffer.get(), 0, sizeof(UniformBlock), &ubo);

		ubo.color = QVector4D(1, 0, 0, 1);
		ubo.MVP = (project * view * model).toGenericMatrix<4, 4>();
		batch->updateDynamicBuffer(mPlaneUniformBuffer.get(), 0, sizeof(UniformBlock), &ubo);

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, batch);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources(mCubeShaderBindings.get());
		const QRhiCommandBuffer::VertexInput cubeVertexBindings(mCubeVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &cubeVertexBindings);
		cmdBuffer->draw(36);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources(mPlaneShaderBindings.get());
		const QRhiCommandBuffer::VertexInput planeVertexBindings(mPlaneVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &planeVertexBindings);
		cmdBuffer->draw(6);

		cmdBuffer->endPass();
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);

	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
	MyWindow window(initParams);
	window.resize({ 800,600 });
	window.show();

	return app.exec();
}
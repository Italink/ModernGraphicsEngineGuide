#include <QApplication>
#include "Render/RHI/QRhiWindow.h"
#include "QDateTime"

static float CubeData[] = {
		 1.0f, -1.0f, -1.0f, 	//+x
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f, 	//-x	
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f, 	//+y
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f, 	//-y
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,

		-1.0f, -1.0f,  1.0f, 	//+z
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f, 	//-z
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
};

struct UniformBlock {
	QGenericMatrix<4, 4, float> model;
	QGenericMatrix<4, 4, float> view;
	QGenericMatrix<4, 4, float> project;
};


class MyFirstTextureWindow : public QRhiWindow {
public:
	MyFirstTextureWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
	}
private:
	QRhiEx::Signal mSigInit;
	QRhiEx::Signal mSigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;

	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;


protected:
	void initRhiResource() {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
		mVertexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
		mUniformBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());

		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, mUniformBuffer.get())
		});

		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(mSwapChain->sampleCount());

		mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec3 inPosition;
			layout(location = 0) out float vDepth;
			layout(binding = 0) uniform UniformBlock{
				mat4 model;
				mat4 view;
				mat4 project;
			}UBO;
			out gl_PerVertex { vec4 gl_Position; };
			void main()
			{
				gl_Position = UBO.project * UBO.view * UBO.model * vec4(inPosition ,1.0f);
				vDepth = gl_Position.z;
			}
		)");
		Q_ASSERT(vs.isValid());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout(location = 0) in float vDepth;
			layout(location = 0) out vec4 outFragColor;
			void main()
			{
				outFragColor = vec4(vDepth/10);
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

		QMatrix4x4 project = mRhi->clipSpaceCorrMatrix();

		project.perspective(90.0, renderTarget->pixelSize().width() /(float) renderTarget->pixelSize().height(), 0.01, 1000);
		QMatrix4x4 view;
		view.lookAt(QVector3D(10, 0, 0), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

		float time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f;
		float factor = qAbs(qSin(time));
		QMatrix4x4 model;
		model.translate(QVector3D(0, factor * 5, 0));
		model.scale(factor);
		model.rotate(time * 180, QVector3D(1, 1, 1));

		UniformBlock ubo;
		ubo.project = project.toGenericMatrix<4, 4>();
		ubo.view = view.toGenericMatrix<4, 4>();
		ubo.model = model.toGenericMatrix<4, 4>();

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
	MyFirstTextureWindow* window = new MyFirstTextureWindow(initParams);
	window->resize({ 800,600 });
	window->show();

	app.exec();
	delete window;
	return 0;
}
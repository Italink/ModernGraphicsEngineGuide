#include <QApplication>
#include <QTime>
#include "QEngineApplication.h"
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {
	//position(xy)		color(rgba)
	 0.0f,    0.5f,		1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,   -0.5f,		0.0f, 1.0f, 0.0f, 1.0f,
	 0.5f,   -0.5f,		0.0f, 0.0f, 1.0f, 1.0f,
};

struct UniformBlock{
	QGenericMatrix<4, 4, float> MVP;
};

class TriangleWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
public:
	TriangleWindow(QRhiWindow::InitParams inInitParams)
		: QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		if (mSigInit.ensure()) {	//初始化资源
			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
			mVertexBuffer->create();
			mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
			mUniformBuffer->create();
			QRhiVertexInputLayout inputLayout;
			inputLayout.setBindings({
				QRhiVertexInputBinding(6 * sizeof(float))
				});

			inputLayout.setAttributes({
				QRhiVertexInputAttribute(0, 0 , QRhiVertexInputAttribute::Float2, 0),
				QRhiVertexInputAttribute(0, 1 , QRhiVertexInputAttribute::Float4, sizeof(float) * 2),
			});

			mPipeline.reset(mRhi->newGraphicsPipeline());

			mPipeline->setVertexInputLayout(inputLayout);

			mShaderBindings.reset(mRhi->newShaderResourceBindings());
			mShaderBindings->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::StageFlag::VertexStage, mUniformBuffer.get()),
			});
			mShaderBindings->create();
			mPipeline->setShaderResourceBindings(mShaderBindings.get());
			mPipeline->setSampleCount(mSwapChain->sampleCount());
			mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());

			QShader vs = QRhiHelper::newShaderFromCode(mRhi.get(), QShader::VertexStage, R"(#version 440
				layout(location = 0) in vec2 position;		
				layout(location = 1) in vec4 color;
				layout(location = 0) out vec4 vColor;		
				layout(binding = 0) uniform UniformBlock{
					mat4 MVP;
				}UBO;

				out gl_PerVertex { 
					vec4 gl_Position;
				};

				void main(){
					gl_Position = UBO.MVP * vec4(position,0.0f,1.0f);
					vColor = color;
				}
			)");
			Q_ASSERT(vs.isValid());

			QShader fs = QRhiHelper::newShaderFromCode(mRhi.get(), QShader::FragmentStage, R"(#version 440
				layout (location = 0) in vec4 vColor;		
				layout (location = 0) out vec4 fragColor;	
				void main(){
					fragColor = vColor;
				}
			)");
			Q_ASSERT(fs.isValid());

			mPipeline->setShaderStages({
				QRhiShaderStage(QRhiShaderStage::Vertex, vs),
				QRhiShaderStage(QRhiShaderStage::Fragment, fs)
				});

			mPipeline->setFrontFace(QRhiGraphicsPipeline::FrontFace::CCW);			//设置逆时针的顶点缠绕方向为正面
			mPipeline->setCullMode(QRhiGraphicsPipeline::CullMode::Back);			//开启背面剔除
			mPipeline->create();
		}

		QRhiRenderTarget* renderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		if (mSigSubmit.ensure()) {
			batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		}
		QMatrix4x4 MVP = mRhi->clipSpaceCorrMatrix();
		MVP.rotate(QTime::currentTime().msecsSinceStartOfDay() / 5.f, QVector3D(0, 1, 0));			//绕Y轴随时间旋转
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), MVP.data());
		cmdBuffer->resourceUpdate(batch);

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, nullptr);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexInput(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexInput);
		cmdBuffer->draw(3);

		cmdBuffer->endPass();
	}
};

int main(int argc, char** argv)
{
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
	TriangleWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
	app.exec();
	return 0;
}
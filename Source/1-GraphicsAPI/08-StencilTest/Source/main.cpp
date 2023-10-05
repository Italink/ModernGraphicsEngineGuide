#include <QApplication>
#include <QTime>
#include "QEngineApplication.h"
#include "Render/RHI/QRhiWindow.h"

static QVector2D Vertices[3] = {
	{ 0.0f,    0.5f},
	{-0.5f,   -0.5f},
	{ 0.5f,   -0.5f},
};

struct UniformBlock {
	QGenericMatrix<4, 4, float> MVP;
};

class TriangleWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiBuffer> mMaskVertexBuffer;
	QScopedPointer<QRhiGraphicsPipeline> mMaskPipeline;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
public:
	TriangleWindow(QRhiHelper::InitParams inInitParams)
		: QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		if (mSigInit.ensure()) {	
			mMaskVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertices)));
			mMaskVertexBuffer->create();

			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertices)));
			mVertexBuffer->create();

			QRhiVertexInputLayout inputLayout;
			inputLayout.setBindings({
				QRhiVertexInputBinding(sizeof(QVector2D))
			});

			inputLayout.setAttributes({
				QRhiVertexInputAttribute(0, 0 , QRhiVertexInputAttribute::Float2, 0),
			});

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
				layout (location = 0) out vec4 fragColor;	
				void main(){
					fragColor = vec4(1);
				}
			)");
			Q_ASSERT(fs.isValid());

			mShaderBindings.reset(mRhi->newShaderResourceBindings());
			mShaderBindings->create();

			mMaskPipeline.reset(mRhi->newGraphicsPipeline());
			mMaskPipeline->setVertexInputLayout(inputLayout);
			mMaskPipeline->setShaderResourceBindings(mShaderBindings.get());
			mMaskPipeline->setSampleCount(mSwapChain->sampleCount());
			mMaskPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
			mMaskPipeline->setShaderStages({
				QRhiShaderStage(QRhiShaderStage::Vertex, vs),
				QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});
			mMaskPipeline->setFlags(QRhiGraphicsPipeline::Flag::UsesStencilRef);			//开启流水线的模版测试功能
			mMaskPipeline->setStencilTest(true);											//开启模版测试
			QRhiGraphicsPipeline::StencilOpState maskStencilState;
			maskStencilState.compareOp = QRhiGraphicsPipeline::CompareOp::Never;			//该管线用于绘制模版（遮罩），我们不希望它在RT上绘制任何片段颜色，因此让它的片段永远不会通过模版测试
			maskStencilState.failOp = QRhiGraphicsPipeline::StencilOp::Replace;				//设置当该片段没有通过模版测试时，使用StencilRef填充模版缓冲区
			mMaskPipeline->setStencilFront(maskStencilState);								//指定正面的模版测试
			mMaskPipeline->setStencilBack(maskStencilState);								//指定背面的模版测试
			mMaskPipeline->create();

			mPipeline.reset(mRhi->newGraphicsPipeline());
			mPipeline->setVertexInputLayout(inputLayout);
			mPipeline->setShaderResourceBindings(mShaderBindings.get());
			mPipeline->setSampleCount(mSwapChain->sampleCount());
			mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
			mPipeline->setShaderStages({
				QRhiShaderStage(QRhiShaderStage::Vertex, vs),
				QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});
			mPipeline->setFlags(QRhiGraphicsPipeline::Flag::UsesStencilRef);				//开启流水线的模版测试功能
			mPipeline->setStencilTest(true);												//开启模版测试
			QRhiGraphicsPipeline::StencilOpState stencilState;
			stencilState.compareOp = QRhiGraphicsPipeline::CompareOp::Equal;				//我们希望在当前管线的StencilRef等于模版缓冲区上的片段值时才通过模版测试
			stencilState.passOp = QRhiGraphicsPipeline::StencilOp::Keep;					//在通过测试后不会对模版缓冲区进行赋值
			stencilState.failOp = QRhiGraphicsPipeline::StencilOp::Keep;					//在没有通过测试时也不会对模版缓冲区进行赋值
			mPipeline->setStencilFront(stencilState);
			mPipeline->setStencilBack(stencilState);
			mPipeline->create();

		}

		QRhiRenderTarget* renderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (mSigSubmit.ensure()) {
			QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
			batch->uploadStaticBuffer(mMaskVertexBuffer.get(), Vertices);					//上传模版（遮罩）的顶点数据，它是一个三角形

			for (auto& vertex : Vertices) 													//上传实际图形的顶点数据，它是一个上下翻转的三角形
				vertex.setY(-vertex.y());
			batch->uploadStaticBuffer(mVertexBuffer.get(), Vertices);

			cmdBuffer->resourceUpdate(batch);
		}

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };							//使用 0 清理模版缓冲区
		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, nullptr);

		cmdBuffer->setGraphicsPipeline(mMaskPipeline.get());
		cmdBuffer->setStencilRef(1);														//设置StencilRef为1，该管线会在模版缓冲区上填充一块值为1的三角形区域
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput maskVertexInput(mMaskVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &maskVertexInput);
		cmdBuffer->draw(3);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setStencilRef(1);														//设置StencilRef为1，该管线会用1跟对应位置的片段模版值进行比较，相等时才会通过模版测试，也就是会将片段绘制当颜色附件上
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
	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Vulkan;
	TriangleWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
	app.exec();
	return 0;
}
#include <QApplication>
#include "QEngineApplication.h"
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {										
	//position(xy)		color(rgba)
	 0.0f,  -0.5f,		1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,   0.5f,		0.0f, 1.0f, 0.0f, 1.0f,
	 0.5f,   0.5f,		0.0f, 0.0f, 1.0f, 1.0f,
};

class TriangleWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;										
	QRhiSignal mSigSubmit;										
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
		if (mSigInit.ensure()) {	//初始化资源
			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
			mVertexBuffer->create();
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
			mShaderBindings->create();
			mPipeline->setShaderResourceBindings(mShaderBindings.get());													
			mPipeline->setSampleCount(mSwapChain->sampleCount());
			mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());

			QShader vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, R"(#version 440
				layout(location = 0) in vec2 position;		//这里需要与上面的inputLayout 对应
				layout(location = 1) in vec4 color;

				layout (location = 0) out vec4 vColor;		//输出变量到 fragment shader 中，这里的location是out的，而不是in

				out gl_PerVertex { 
					vec4 gl_Position;
				};

				void main(){
					gl_Position = vec4(position,0.0f,1.0f);
					vColor = color;
				}
			)");
			Q_ASSERT(vs.isValid());

			QShader fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
				layout (location = 0) in vec4 vColor;		//上一阶段的out变成了这一阶段的in
				layout (location = 0) out vec4 fragColor;	//片段着色器输入
				void main(){
					fragColor = vColor;
				}
			)");
			Q_ASSERT(fs.isValid());

			mPipeline->setShaderStages({
				QRhiShaderStage(QRhiShaderStage::Vertex, vs),
				QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});

			mPipeline->setFlags(QRhiGraphicsPipeline::Flag::UsesScissor);							//开启流水线的裁剪测试功能
			mPipeline->create();
		}

		QRhiRenderTarget* renderTarget = mSwapChain->currentFrameRenderTarget();	
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();		

		if (mSigSubmit.ensure()) {
			QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();		
			batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);				
			cmdBuffer->resourceUpdate(batch);
		}

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);			
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };					
		cmdBuffer->beginPass(renderTarget, clearColor, dsClearValue, nullptr);		

		cmdBuffer->setGraphicsPipeline(mPipeline.get());							
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));		
		cmdBuffer->setScissor(QRhiScissor(0, 0, mSwapChain->currentPixelSize().width() / 2, mSwapChain->currentPixelSize().height() / 2));		//通过裁剪只保留四分之一边角区域，具体的表现情况跟不同API的标准坐标空间有关
		cmdBuffer->setShaderResources();											
		const QRhiCommandBuffer::VertexInput vertexInput(mVertexBuffer.get(), 0);	
		cmdBuffer->setVertexInput(0, 1, &vertexInput);								
		cmdBuffer->draw(3);															

		cmdBuffer->endPass();														
	}
};

int main(int argc, char **argv)
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
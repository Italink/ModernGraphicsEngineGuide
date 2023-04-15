#include <QApplication>
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {
	//position(xy)		color(rgba)
	 0.0f,  -0.5f,		1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,   0.5f,		0.0f, 1.0f, 0.0f, 1.0f,
	 0.5f,   0.5f,		0.0f, 0.0f, 1.0f, 1.0f,
};

class TriangleWindow : public QRhiWindow {
private:
	QRhiEx::Signal sigInit;
	QRhiEx::Signal sigSubmit;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
public:
	TriangleWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		sigInit.request();
		sigSubmit.request();
	}
protected:
	void initRhiResource() {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());															//创建绑定
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		mPipeline->setShaderResourceBindings(mShaderBindings.get());														//绑定到流水线中华

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(mSwapChain->sampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);


		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			QRhiVertexInputBinding(6 * sizeof(float))		//定义每个VertexBuffer，单组顶点数据的跨度，这里是 6 * sizeof(float)，可以当作是GPU会从Buffer0（0是Index）读取 6 * sizeof(float) 传给 Vertex Shader
		});

		inputLayout.setAttributes({
			QRhiVertexInputAttribute(0, 0 , QRhiVertexInputAttribute::Float2, 0),					// 从每组顶点数据的位置 0 开始作为 Location0（Float2） 的起始地址
			QRhiVertexInputAttribute(0, 1 , QRhiVertexInputAttribute::Float4, sizeof(float) * 2),	// 从每组顶点数据的位置 sizeof(float) * 2 开始作为 Location1（Float4） 的起始地址
		});

		mPipeline->setVertexInputLayout(inputLayout);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
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

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout (location = 0) in vec4 vColor;		//上一阶段的out变成了这一阶段的in
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
		mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
		mPipeline->create();
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
			resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);				//上传顶点数据
		}

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		currentCmdBuffer->setGraphicsPipeline(mPipeline.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);		//将 mVertexBuffer 绑定到Buffer0，内存偏移值为0
		currentCmdBuffer->setVertexInput(0, 1, &vertexBindings);
		currentCmdBuffer->draw(3);

		currentCmdBuffer->endPass();
	}
};

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    TriangleWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}
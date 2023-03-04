#include <QApplication>
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {
	//position (xy)	
	 0.0f,   0.1f,
	-0.1f,  -0.1f,
	 0.1f,  -0.1f,
};

static float InstancingData[] = {
	//offset (xy)
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class InstancingWindow : public QRhiWindow {
private:
	QRhiEx::Signal sigInit;
	QRhiEx::Signal sigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mInstancingBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
public:
	InstancingWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		sigInit.request();
		sigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* currentCmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (sigInit.receive()) {
			initRhiResource();
		}
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (sigSubmit.receive()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			submitRhiData(resourceUpdates);
		}

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		currentCmdBuffer->setGraphicsPipeline(mPipeline.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings[] = {
			{ mVertexBuffer.get(), 0 },
			{ mInstancingBuffer.get(), 0 },
		};
		currentCmdBuffer->setVertexInput(0, 2, vertexBindings);
		currentCmdBuffer->draw(3, 3);

		currentCmdBuffer->endPass();
	}

	void initRhiResource() {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mInstancingBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(InstancingData)));
		mInstancingBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(mSwapChain->sampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

		QShader vs = QRhiEx::newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 offset;

out gl_PerVertex { 
	vec4 gl_Position;
};
void main(){
    gl_Position = vec4(position + offset,0.0f,1.0f);
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiEx::newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) out vec4 fragColor;
void main(){
    fragColor = vec4(0.1f,0.5f,0.9f,1.0f);
}
)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
			});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			QRhiVertexInputBinding(2 * sizeof(float)),
			QRhiVertexInputBinding(2 * sizeof(float), QRhiVertexInputBinding::PerInstance),
			});

		inputLayout.setAttributes({
			QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
			QRhiVertexInputAttribute(1, 1, QRhiVertexInputAttribute::Float2, 0),
			});

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
		mPipeline->create();
	}

	void submitRhiData(QRhiResourceUpdateBatch* resourceUpdates) {
		resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		resourceUpdates->uploadStaticBuffer(mInstancingBuffer.get(), InstancingData);
	}
};
int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    initParams.backend = QRhi::D3D11;
    InstancingWindow* window = new InstancingWindow(initParams);
	window->resize({ 800,600 });
	window->show();
    app.exec();
    delete window;
    return 0;
}
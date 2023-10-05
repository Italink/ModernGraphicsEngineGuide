#include <QApplication>
#include "Render/RHI/QRhiWindow.h"

static float VertexData[] = {
	//position (xy)	
	 0.0f,    0.02f,
	-0.02f,  -0.02f,
	 0.02f,  -0.02f,
};

class InstancingWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mInstancingBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QVector<QVector2D> mInstanceData;
public:
	InstancingWindow(QRhiHelper::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();

		float offset = 0.05f;
		for (float i = -1.0f; i <= 1.0f; i += offset) {
			for (float j = -1.0f; j <= 1.0f; j += offset) {
				mInstanceData.push_back({ i,j });
			}
		}
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (mSigInit.ensure()) {
			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
			mVertexBuffer->create();

			mInstancingBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(QVector2D) * mInstanceData.size()));
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

			QShader vs = QRhiHelper::newShaderFromCode(QShader::VertexStage, R"(#version 440
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

			QShader fs = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
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
				QRhiVertexInputBinding(2 * sizeof(float), QRhiVertexInputBinding::PerInstance),			//声明逐实例数据
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
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (mSigSubmit.ensure()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
			resourceUpdates->uploadStaticBuffer(mInstancingBuffer.get(), mInstanceData.data());
		}
		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		cmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		cmdBuffer->setShaderResources();

		const QRhiCommandBuffer::VertexInput vertexBindings[] = {
			{ mVertexBuffer.get(), 0 },
			{ mInstancingBuffer.get(), 0 },
		};
		cmdBuffer->setVertexInput(0, 2, vertexBindings);
		cmdBuffer->draw(3, mInstanceData.size());
		cmdBuffer->endPass();
	}
};
int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiHelper::InitParams initParams;
    initParams.backend = QRhi::D3D11;
    InstancingWindow* window = new InstancingWindow(initParams);
	window->resize({ 800,600 });
	window->show();
    app.exec();
    delete window;
    return 0;
}
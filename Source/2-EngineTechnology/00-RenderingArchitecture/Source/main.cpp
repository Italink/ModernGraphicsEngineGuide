#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/IRenderComponent.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,  -0.5f,
	-0.5f,   0.5f,
	 0.5f,   0.5f,
};

class QTriangleRenderComponent : public IRenderComponent {
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	Q_BUILDER_BEGIN(QTriangleRenderComponent)
	Q_BUILDER_END()
protected:
	void onRebuildResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();
    }
	void onRebuildPipeline() override {
		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(getBasePass()->getSampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
			layout(location = 0) in vec2 position;
			out gl_PerVertex { 
				vec4 gl_Position;
			};
			void main(){
				gl_Position = vec4(position,0.0f,1.0f);
			}
		)");
		Q_ASSERT(vs.isValid());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
			layout(location = 0) out vec4 fragColor;
			void main(){
				fragColor = vec4(0.1f,0.5f,0.9f,1.0f);
			}
		)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			QRhiShaderStage(QRhiShaderStage::Vertex, vs),
			QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			QRhiVertexInputBinding(2 * sizeof(float))
			});

		inputLayout.setAttributes({
			QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
			});

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->setRenderPassDescriptor(getBasePass()->getRenderPassDescriptor());
		mPipeline->create();
    }

    void onUpload(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
    }

    void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
        cmdBuffer->setGraphicsPipeline(mPipeline.get());
        cmdBuffer->setViewport(viewport);
        cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
        cmdBuffer->setVertexInput(0, 1, &vertexBindings);
        cmdBuffer->draw(3);
    }
};

int main(int argc, char **argv){
    QEngineApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    QRenderWidget widget(initParams);

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QTriangleRenderComponent::Create("Triangle")
			)
		)
		.end()
	);

	widget.showMaximized();
    return app.exec();
}
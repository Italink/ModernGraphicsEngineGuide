#include <QApplication>
#include "QRenderWidget.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
#include "Render/RenderComponent/ISceneRenderComponent.h"
#include "QMetaData.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class QTriangleRenderComponent : public ISceneRenderComponent {
	Q_OBJECT
private:
	Q_PROPERTY(QColor, Color) = Qt::green;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
protected:
	void onRebuildResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(float)*16 + sizeof(QVector4D)));
		mUniformBuffer->create();
	}
	void onRebuildPipeline() override {
		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage,mUniformBuffer.get())
		});
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(sceneRenderPass()->getSampleCount());
		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

		QShader vs = QRhiEx::newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec2 iPostion;
layout(location = 0) out vec4 vColor;
layout(location = 1) out float vDepth;

layout(binding = 0) uniform UniformBuffer{
	mat4 uTransform;
	vec4 uColor;
}UBO;

out gl_PerVertex { 
	vec4 gl_Position;
};
void main(){
    gl_Position = UBO.uTransform * vec4(iPostion,0.0f,1.0f);
	vColor = UBO.uColor;
	vDepth = gl_Position.z;
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiEx::newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) in vec4 vColor;
layout(location = 1) in float vDepth;
layout(location = 0) out vec4 oFragColor;
void main(){
    oFragColor = vColor;
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
		mPipeline->setRenderPassDescriptor(sceneRenderPass()->getRenderPassDescriptor());
		mPipeline->create();
	}
	void onUpload(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void onUpdate(QRhiResourceUpdateBatch* batch) override {
		QMatrix4x4 mat = calculateMatrixMVP();
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(float) * 16, &mat);
		QVector4D vec4(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
		batch->updateDynamicBuffer(mUniformBuffer.get(), sizeof(float) * 16, sizeof(QVector4D), &vec4);
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

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::D3D11;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent(new QTriangleRenderComponent())
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#include "main.moc"
#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/IRenderComponent.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,  -0.5f,
	-0.5f,   0.5f,
	 0.5f,   0.5f,
};

class QTriangleRenderComponent: public IRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QColor Color READ getColor WRITE setColor)
public:
	QColor getColor() const { return mColor; }
	void setColor(QColor val) { mColor = val; }
private:
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QSharedPointer<QPrimitiveRenderProxy> mProxy;
	QColor mColor = QColor::fromRgbF(0.1f, 0.5f, 0.9f, 1.0f);
protected:
	void onRebuildResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mProxy = newPrimitiveRenderProxy();

		mProxy->addUniformBlock(QRhiShaderStage::Fragment, "UBO")
			->addParam("Color", mColor);

		mProxy->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),2 * sizeof(float))
		});

		mProxy->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition",0, 0, QRhiVertexInputAttribute::Float2, 0),
		});
		mProxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			void main(){
				gl_Position =vec4(inPosition, 0.0f,1.0f);
			}
		)");
		mProxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			void main(){
				%1
			})")
			.arg(hasColorAttachment("BaseColor")? "BaseColor = UBO.Color;" : "")
			.toLocal8Bit()
		);
		mProxy->setOnUpload([this](QRhiResourceUpdateBatch* batch) {
			batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
		});
		mProxy->setOnUpdate([this](QRhiResourceUpdateBatch* batch, const QPrimitiveRenderProxy::UniformBlocks& blocks, const QPrimitiveRenderProxy::UpdateContext& ctx) {
			blocks["UBO"]->setParamValue("Color", QVariant::fromValue(mColor));
		});
		mProxy->setOnDraw([this](QRhiCommandBuffer* cmdBuffer) {
			const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
			cmdBuffer->setVertexInput(0, 1, &vertexBindings);
			cmdBuffer->draw(3);
		});
    }
};

class QOutliningPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QOutliningPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColor);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QOutliningPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, OutliningResult)
	QRP_OUTPUT_END()
private:
	QRhiTextureRef mColorAttachment;
	QRhiTextureRenderTargetRef mRenderTarget;
	QShader mOutliningFS;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mOutliningBindings;
	QRhiGraphicsPipelineRef mOutliningPipeline;
public:
	QOutliningPassBuilder() {
		mOutliningFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
			layout (location = 0) in vec2 vUV;
			layout (location = 0) out vec4 outFragColor;

			layout (binding = 0) uniform sampler2D uBaseColor;

			void main() {
				vec2 texOffset = 1.0 / textureSize(uBaseColor, 0);		// gets size of single texel
		
				vec3 maxdiff = vec3(0.f);
				vec3 center = texture(uBaseColor,vUV).rgb;
				
				maxdiff = max(maxdiff, abs(texture(uBaseColor,vUV+vec2(texOffset.x,0)).rgb - center));
				maxdiff = max(maxdiff, abs(texture(uBaseColor,vUV-vec2(texOffset.x,0)).rgb - center));
				maxdiff = max(maxdiff, abs(texture(uBaseColor,vUV+vec2(0,texOffset.y)).rgb - center));
				maxdiff = max(maxdiff, abs(texture(uBaseColor,vUV-vec2(0,texOffset.y)).rgb - center));

				const vec4 outliningColor = vec4(1.0,0.0,0.0,1.0);
				
				outFragColor = length(maxdiff) > 0.1 ? outliningColor : vec4(center,1.0f);

			}
		)");
	}
	void setup(QRenderGraphBuilder& builder) override {
		builder.setupTexture(mColorAttachment, "Outlining", QRhiTexture::Format::RGBA32F, mInput._BaseColor->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
		builder.setupRenderTarget(mRenderTarget, "OutliningRT", QRhiTextureRenderTargetDescription(mColorAttachment.get()));

		builder.setupSampler(mSampler, "OutliningSampler", QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);

		builder.setupShaderResourceBindings(mOutliningBindings, "OutliningBindings", {
			QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage,mInput._BaseColor.get() ,mSampler.get()),
		});
		QRhiGraphicsPipelineState PSO;
		PSO.shaderResourceBindings = mOutliningBindings.get();
		PSO.sampleCount = mRenderTarget->sampleCount();
		PSO.renderPassDesc = mRenderTarget->renderPassDescriptor();
		QRhiGraphicsPipeline::TargetBlend targetBlends;
		targetBlends.enable = true;
		PSO.targetBlends = { targetBlends };
		PSO.shaderStages = {
			QRhiShaderStage(QRhiShaderStage::Vertex, builder.getFullScreenVS()),
			QRhiShaderStage(QRhiShaderStage::Fragment, mOutliningFS)
		};
		builder.setupGraphicsPipeline(mOutliningPipeline, "OutliningPipeline", PSO);

		mOutput.OutliningResult = mColorAttachment;
	}
	void execute(QRhiCommandBuffer* cmdBuffer) override {
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(mRenderTarget.get(), clearColor, dsClearValue);
		cmdBuffer->setGraphicsPipeline(mOutliningPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mOutliningBindings.get());
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
};

class MyRenderer : public IRenderer {
private:
	QTriangleRenderComponent mComp;
public:
	MyRenderer()
		: IRenderer({QRhi::Vulkan})
	{
		addComponent(&mComp);
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QMeshPassBuilder::Output meshPassOut = graphBuilder.addPassBuilder<QMeshPassBuilder>("MeshPass");

		QOutliningPassBuilder::Output outliningOut = graphBuilder.addPassBuilder<QOutliningPassBuilder>("OutliningPass")
			.setBaseColor(meshPassOut.BaseColor);

		QOutputPassBuilder::Output ret = graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(outliningOut.OutliningResult);
	}
};

int main(int argc, char **argv){
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
    return app.exec();
}

#include "main.moc"
#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QSsaoPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QBlurPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/PBR/QPbrMeshPassBuilder.h"

class QSsaoMergePassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QSsaoMergePassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColor);
		QRP_INPUT_ATTR(QRhiTextureRef, SsaoTexture);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QSsaoMergePassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, SsaoMergeResult)
	QRP_OUTPUT_END()
private:
	QRhiTextureRef mColorAttachment;
	QRhiTextureRenderTargetRef mRenderTarget;
	QShader mMergeFS;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mMergeBindings;
	QRhiGraphicsPipelineRef mMergePipeline;
public:
	QSsaoMergePassBuilder() {
		mMergeFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
			layout (binding = 0) uniform sampler2D uSrcTexture;
			layout (binding = 1) uniform sampler2D uSsaoTexture;
			layout (location = 0) in vec2 vUV;
			layout (location = 0) out vec4 outFragColor;
			void main() {
				vec4 srcColor = texture(uSrcTexture, vUV);
				vec4 ssaoColor = texture(uSsaoTexture, vUV);
				outFragColor = vec4(srcColor.rgb * ssaoColor.r,srcColor.a);
			}
		)");
	}
	void setup(QRenderGraphBuilder& builder) override {
		builder.setupTexture(mColorAttachment, "SsaoMerge", QRhiTexture::Format::RGBA32F, mInput._BaseColor->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
		builder.setupRenderTarget(mRenderTarget, "SsaoMergeRT", QRhiTextureRenderTargetDescription(mColorAttachment.get()));

		builder.setupSampler(mSampler, "SsaoMergeSampler", QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);

		builder.setupShaderResourceBindings(mMergeBindings, "SsaoMergeBindings", {
			QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage,mInput._BaseColor.get() ,mSampler.get()),
			QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,mInput._SsaoTexture.get() ,mSampler.get()),
		});
		QRhiGraphicsPipelineState PSO;
		PSO.shaderResourceBindings = mMergeBindings.get();
		PSO.sampleCount = mRenderTarget->sampleCount();
		PSO.renderPassDesc = mRenderTarget->renderPassDescriptor();
		QRhiGraphicsPipeline::TargetBlend targetBlends;
		targetBlends.enable = true;
		PSO.targetBlends = { targetBlends };
		PSO.shaderStages = {
			QRhiShaderStage(QRhiShaderStage::Vertex, builder.getFullScreenVS()),
			QRhiShaderStage(QRhiShaderStage::Fragment, mMergeFS)
		};
		builder.setupGraphicsPipeline(mMergePipeline, "SsaoMergePipeline", PSO);

		mOutput.SsaoMergeResult = mColorAttachment;
	}
	void execute(QRhiCommandBuffer* cmdBuffer) override {
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(mRenderTarget.get(), clearColor, dsClearValue);
		cmdBuffer->setGraphicsPipeline(mMergePipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mMergeBindings.get());
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
}; 

#define Q_PROPERTY_VAR(Type, Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
    } \
    Type Name


class MyRenderer : public IRenderer {
	Q_OBJECT

	Q_PROPERTY_VAR(float, Bias) = 0.1f;
	Q_PROPERTY_VAR(float, Radius) = 2.0f;
	Q_PROPERTY_VAR(int, SampleSize) = 64;

	Q_PROPERTY_VAR(int, BlurIterations) = 1;
	Q_PROPERTY_VAR(int, BlurSize) = 4;
	Q_PROPERTY_VAR(int, DownSampleCount) = 2;

	Q_CLASSINFO("SampleSize", "Min=1,Max=128")
	Q_CLASSINFO("BlurIterations", "Min=1,Max=8")
	Q_CLASSINFO("BlurSize", "Min=1,Max=80")
	Q_CLASSINFO("DownSampleCount", "Min=1,Max=16")
private:
	QStaticMeshRenderComponent mStaticComp;
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		mStaticComp.setStaticMesh(QStaticMesh::CreateFromFile("Resources/Model/mandalorian_ship/scene.gltf"));
		mStaticComp.setRotation(QVector3D(-90, 0, 0));

		addComponent(&mStaticComp);

		getCamera()->setPosition(QVector3D(20, 15, 12));
		getCamera()->setRotation(QVector3D(-30, 145, 0));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QPbrMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder<QPbrMeshPassBuilder>("MeshPass");

		QSsaoPassBuilder::Output ssaoOut = graphBuilder.addPassBuilder<QSsaoPassBuilder>("SsaoPass")
			.setNormalTexture(meshOut.Normal)
			.setPositionTexture(meshOut.Position)
			.setBias(Bias)
			.setRadius(Radius)
			.setSampleSize(SampleSize);

		QBlurPassBuilder::Output blurOut = graphBuilder.addPassBuilder<QBlurPassBuilder>("BlurPass")
			.setBaseColorTexture(ssaoOut.SsaoResult)
			.setBlurIterations(BlurIterations)
			.setBlurSize(BlurSize)
			.setDownSampleCount(DownSampleCount);

		QSsaoMergePassBuilder::Output merge = graphBuilder.addPassBuilder<QSsaoMergePassBuilder>("SsaoMergePass")
			.setBaseColor(meshOut.BaseColor)
			.setSsaoTexture(blurOut.BlurResult);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(merge.SsaoMergeResult);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}


#include "main.moc"
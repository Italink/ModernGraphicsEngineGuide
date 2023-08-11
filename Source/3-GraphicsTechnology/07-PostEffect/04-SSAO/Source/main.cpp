#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/Pass/PBR/QPbrBasePassDeferred.h"
#include "Render/Pass/QSsaoRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"

class QSsaoMergeRenderPass :public IRenderPass {
	Q_OBJECT
		Q_BUILDER_BEGIN_RENDER_PASS(QSsaoMergeRenderPass, Raw, Ssao)
		Q_BUILDER_END_RENDER_PASS(Result)
public:
	void resizeAndLinkNode(const QSize& size) override {
		QRhiTexture* raw = getTextureIn_Raw();
		QRhiTexture* ssao = getTextureIn_Ssao();

		mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, raw->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mRT.colorAttachment->create();
		mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
		mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
		mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
		mRT.renderTarget->create();

		mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
			QRhiSampler::Linear,
			QRhiSampler::None,
			QRhiSampler::ClampToEdge,
			QRhiSampler::ClampToEdge));
		mSampler->create();
		mBindings.reset(mRhi->newShaderResourceBindings());
		mBindings->setBindings({
			QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,raw, mSampler.get()),
			QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,ssao,mSampler.get())
			});
		mBindings->create();
		registerTextureOut_Result(mRT.colorAttachment.get());
	}
	void compile() override {
		mPipeline.reset(mRhi->newGraphicsPipeline());
		QRhiGraphicsPipeline::TargetBlend blendState;
		blendState.enable = true;
		mPipeline->setTargetBlends({ blendState });
		mPipeline->setSampleCount(mRT.renderTarget->sampleCount());

		QString vsCode = R"(#version 450
			layout (location = 0) out vec2 vUV;
			out gl_PerVertex{
				vec4 gl_Position;
			};
			void main() {
				vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
				gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
				%1
			}
		)";
		QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());

		QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
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

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
			});

		QRhiVertexInputLayout inputLayout;

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mBindings.get());
		mPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
		mPipeline->create();
	}
	void render(QRhiCommandBuffer* cmdBuffer) override {
		cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mBindings.get());
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
};


int main(int argc, char **argv){
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	auto camera = widget.setupCamera();
	camera->setPosition(QVector3D(20, 15, 12));
	camera->setRotation(QVector3D(-30, 145, 0));

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QPbrBasePassDeferred::Create("BasePass")
			.addComponent(
				QStaticMeshRenderComponent::Create("StaticMesh")
				.setStaticMesh(QStaticMesh::CreateFromFile(RESOURCE_DIR"/Model/mandalorian_ship/scene.gltf"))
				.setRotation(QVector3D(-90, 0, 0))
			)
		)
		.addPass(
			QSsaoRenderPass::Create("Ssao")
			.setTextureIn_Position("BasePass", QPbrBasePassDeferred::Out::Position)
			.setTextureIn_Normal("BasePass", QPbrBasePassDeferred::Out::Normal)
		)
		.addPass(
			QBlurRenderPass::Create("Blur")
			.setBlurIterations(1)
			.setBlurSize(4)
			.setDownSampleCount(2)
			.setTextureIn_Src( "Ssao", QSsaoRenderPass::Out::Result)
		)
		.addPass(
			QSsaoMergeRenderPass::Create("SsaoMerge")
			.setTextureIn_Raw("BasePass", QPbrBasePassDeferred::Out::BaseColor)
			.setTextureIn_Ssao("Blur", QBlurRenderPass::Out::Result)
		)
		.end("SsaoMerge", QSsaoMergeRenderPass::Result)
	);

	widget.showMaximized();
	return app.exec();
}

#include "main.moc"
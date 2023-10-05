#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include "Render/RHI/QRhiHelper.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);

    QSharedPointer<QRhi> rhi = QRhiHelper::create();

	if (!rhi->isFeatureSupported(QRhi::Feature::MultisampleTexture)) {
		return -1;
	}

	QScopedPointer<QRhiTexture> msaaTexture;
	QScopedPointer<QRhiTexture> msaaResolveTexture;
	QScopedPointer<QRhiTexture> renderTargetTexture;
	QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	QScopedPointer<QRhiRenderPassDescriptor> renderTargetDesc;

	msaaTexture.reset(rhi->newTexture(QRhiTexture::RGBA8, QSize(1280, 720), 8, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	msaaTexture->create();
	msaaResolveTexture.reset(rhi->newTexture(QRhiTexture::RGBA8, QSize(1280, 720), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	msaaResolveTexture->create();
	QRhiColorAttachment colorAttachment;
	colorAttachment.setTexture(msaaTexture.get());
	colorAttachment.setResolveTexture(msaaResolveTexture.get());
	renderTarget.reset(rhi->newTextureRenderTarget({ colorAttachment }));
	renderTargetDesc.reset(renderTarget->newCompatibleRenderPassDescriptor());
	renderTarget->setRenderPassDescriptor(renderTargetDesc.get());
	renderTarget->create();
	
	QScopedPointer<QRhiBuffer> vertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> shaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	{
		vertexBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		vertexBuffer->create();

		shaderBindings.reset(rhi->newShaderResourceBindings());
		shaderBindings->create();

		mPipeline.reset(rhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(renderTarget->sampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

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
		mPipeline->setShaderResourceBindings(shaderBindings.get());
		mPipeline->setRenderPassDescriptor(renderTargetDesc.get());
		mPipeline->create();
	}
	
	QRhiCommandBuffer* cmdBuffer;
	if (rhi->beginOffscreenFrame(&cmdBuffer) != QRhi::FrameOpSuccess)
		return 1;

	QRhiResourceUpdateBatch* resourceUpdates = rhi->nextResourceUpdateBatch();
	resourceUpdates->uploadStaticBuffer(vertexBuffer.get(), VertexData);

	const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
	const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

	cmdBuffer->beginPass(renderTarget.get(), clearColor, dsClearValue, resourceUpdates);

	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, msaaTexture->pixelSize().width(), msaaTexture->pixelSize().height()));
	cmdBuffer->setShaderResources();
	const QRhiCommandBuffer::VertexInput vertexBindings(vertexBuffer.get(), 0);
	cmdBuffer->setVertexInput(0, 1, &vertexBindings);
	cmdBuffer->draw(3);

	resourceUpdates = rhi->nextResourceUpdateBatch();
	QRhiReadbackResult rbResult;
	QString outputPath = "msaa.png";
	rbResult.completed = [&rbResult, &rhi, &outputPath] {
		if (!rbResult.data.isEmpty()) {
			const uchar* p = reinterpret_cast<const uchar*>(rbResult.data.constData());
			QImage image(p, rbResult.pixelSize.width(), rbResult.pixelSize.height(), QImage::Format_RGBA8888);
			if (rhi->isYUpInFramebuffer())
				image.mirrored().save(outputPath);
			else
				image.save(outputPath);
		}
	};
	QRhiReadbackDescription rb(msaaResolveTexture.get());		//回读 msaaResolveTexture 而不是 msaaTexture
	resourceUpdates->readBackTexture(rb, &rbResult);
	cmdBuffer->endPass(resourceUpdates);
	rhi->endOffscreenFrame();
	QDesktopServices::openUrl(QUrl("file:" + outputPath, QUrl::TolerantMode));
	return app.exec();
}
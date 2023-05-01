#include <QApplication>

#include "Render/RHI/QRhiWindow.h"
#include "Render/Painter/TexturePainter.h"
#include "private/qrhivulkan_p.h"
#include "qvulkanfunctions.h"
#include "private/qvulkandefaultinstance_p.h"

class IndirectDrawWindow : public QRhiWindow {
private:
	QRhiEx::Signal mSigInit;
	QRhiEx::Signal mSigSubmit;

	QScopedPointer<QRhiBuffer> mStorageBuffer;
	QScopedPointer<QRhiBuffer> mIndirectDrawBuffer;

	QScopedPointer<QRhiComputePipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;

	int mDispatchParam[3] = { 1,1,1 };
public:
	IndirectDrawWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	void initRhiResource() {
		mStorageBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, sizeof(float)));
		mStorageBuffer->create();

		mIndirectDrawBuffer.reset(mRhi->newVkBuffer(QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(mDispatchParam)));
		mIndirectDrawBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::bufferLoadStore(0,QRhiShaderResourceBinding::ComputeStage,mStorageBuffer.get()),
			});
		mShaderBindings->create();
		mPipeline.reset(mRhi->newComputePipeline());
		QShader cs = mRhi->newShaderFromCode(QShader::ComputeStage, R"(#version 440
			layout(std140, binding = 0) buffer StorageBuffer{
				int counter;
			}SSBO;
			layout (binding = 1, rgba8) uniform image2D Tex;

			void main(){
				int currentCounter = atomicAdd(SSBO.counter,1);
			}
		)");
		Q_ASSERT(cs.isValid());

		mPipeline->setShaderStage({
			QRhiShaderStage(QRhiShaderStage::Compute, cs),
			});

		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->create();
	}

	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (mSigInit.ensure()) {
			initRhiResource();
		}
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if(mSigSubmit.ensure()){
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			resourceUpdates->uploadStaticBuffer(mIndirectDrawBuffer.get(), 0, sizeof(mDispatchParam), &mDispatchParam);
			cmdBuffer->resourceUpdate(resourceUpdates);
			mRhi->finish();
		}

		cmdBuffer->beginComputePass(nullptr, QRhiCommandBuffer::ExternalContent);
		cmdBuffer->setComputePipeline(mPipeline.get());
		cmdBuffer->setShaderResources();
		
		QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		auto buffer = mIndirectDrawBuffer->nativeBuffer();
		VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
		QVulkanInstance* vkInstance = QVulkanDefaultInstance::instance();
		vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDispatchIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0);
		cmdBuffer->endComputePass();
		static QRhiBufferReadbackResult mCtxReader;
		mCtxReader.completed = [this]() {
			int counter;
			memcpy(&counter, mCtxReader.data.constData(), mCtxReader.data.size());
			qDebug() << counter;
		};
		resourceUpdates = mRhi->nextResourceUpdateBatch();
		resourceUpdates->readBackBuffer(mStorageBuffer.get(), 0, sizeof(float), &mCtxReader);
		cmdBuffer->resourceUpdate(resourceUpdates);
		mRhi->finish();
		mDispatchParam[0]++;
	}
};


int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    initParams.backend = QRhi::Vulkan;
    IndirectDrawWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}
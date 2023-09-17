#include <QApplication>

#include "Render/RHI/QRhiWindow.h"
#include "Render/Painter/TexturePainter.h"
#include "private/qrhivulkan_p.h"
#include "qvulkanfunctions.h"

struct DispatchStruct {
	int x, y, z;
};

class IndirectDrawWindow : public QRhiWindow {
private:
	QRhiSignal mSigInit;
	QRhiSignal mSigSubmit;

	QScopedPointer<QRhiBuffer> mStorageBuffer;
	QScopedPointer<QRhiBuffer> mIndirectDrawBuffer;

	QScopedPointer<QRhiComputePipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
public:
	IndirectDrawWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		mSigInit.request();
		mSigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* cmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (mSigInit.ensure()) {
			mStorageBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, sizeof(float)));
			mStorageBuffer->create();

			mIndirectDrawBuffer.reset(QRhiHelper::newVkBuffer(mRhi.get(), QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(DispatchStruct)));
			mIndirectDrawBuffer->create();

			mShaderBindings.reset(mRhi->newShaderResourceBindings());
			mShaderBindings->setBindings({
				QRhiShaderResourceBinding::bufferLoadStore(0,QRhiShaderResourceBinding::ComputeStage,mStorageBuffer.get()),
				});
			mShaderBindings->create();
			mPipeline.reset(mRhi->newComputePipeline());
			QShader cs = QRhiHelper::newShaderFromCode(mRhi.get(), QShader::ComputeStage, R"(#version 440
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
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if(mSigSubmit.ensure()){
			resourceUpdates = mRhi->nextResourceUpdateBatch();								//初始化间接缓冲区
			DispatchStruct dispatch;
			dispatch.x = dispatch.y = dispatch.z = 1;
			resourceUpdates->uploadStaticBuffer(mIndirectDrawBuffer.get(), 0, sizeof(DispatchStruct), &dispatch);
			cmdBuffer->resourceUpdate(resourceUpdates);
			mRhi->finish();
		}

		QVulkanInstance* vkInstance = vulkanInstance();
		QRhiVulkanNativeHandles* vkHandle = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		QVulkanDeviceFunctions* vkDevFunc =  vkInstance->deviceFunctions(vkHandle->dev);

		QRhiVulkanCommandBufferNativeHandles* cmdBufferHandles = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QVkCommandBuffer* cbD = QRHI_RES(QVkCommandBuffer, cmdBuffer);
		VkCommandBuffer vkCmdBuffer = cmdBufferHandles->commandBuffer;

		QVkComputePipeline* pipelineHandle = (QVkComputePipeline*)mPipeline.get();
		VkPipeline vkPipeline = pipelineHandle->pipeline;

		QRhiBuffer::NativeBuffer indirectBufferHandle = mIndirectDrawBuffer->nativeBuffer();
		VkBuffer vkIndirectBuffer = *(VkBuffer*)indirectBufferHandle.objects[0];

		cmdBuffer->beginExternal();															//开始扩展，之后可输入原生API的指令
		vkDevFunc->vkCmdBindPipeline(vkCmdBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);
		QRhiHelper::setShaderResources(mPipeline.get(), cmdBuffer, mShaderBindings.get());	//辅助函数，用于更新VK流水线的描述符集绑定
		vkDevFunc->vkCmdDispatchIndirect(vkCmdBuffer, vkIndirectBuffer, 0);

		cmdBuffer->endExternal();

		static QRhiReadbackResult mCtxReader;
		mCtxReader.completed = [this]() {
			int counter;
			memcpy(&counter, mCtxReader.data.constData(), mCtxReader.data.size());
			qDebug() << counter;
		};
		resourceUpdates = mRhi->nextResourceUpdateBatch();
		resourceUpdates->readBackBuffer(mStorageBuffer.get(), 0, sizeof(float), &mCtxReader);
		cmdBuffer->resourceUpdate(resourceUpdates);
		mRhi->finish();																		//等待回读结束
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
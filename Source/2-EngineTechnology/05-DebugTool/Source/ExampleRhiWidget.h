#ifndef EXAMPLEWIDGET_H
#define EXAMPLEWIDGET_H

#include "Render/RHI/QRhiWidget.h"

class ExampleRhiWidget : public QRhiWidget{
public:
    ExampleRhiWidget() {
        setDebugLayer(true);
    }

    void initialize(QRhi* rhi, QRhiTexture* outputTexture) override {
		if (mRhi != rhi) {
			mRenderTarget.reset();
			mRenderPassDesc.reset();
			mDSBuffer.reset();
		}
		else if (mOutputTexture != outputTexture) {
			mRenderTarget.reset();
			mRenderPassDesc.reset();
		}

		mRhi = rhi;
		mOutputTexture = outputTexture;

		if (!mDSBuffer) {
			mDSBuffer.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, mOutputTexture->pixelSize()));
			mDSBuffer->create();
		}
		else if (mDSBuffer->pixelSize() != mOutputTexture->pixelSize()) {
			mDSBuffer->setPixelSize(mOutputTexture->pixelSize());
			mDSBuffer->create();
		}

		if (!mRenderTarget) {
			mRenderTarget.reset(mRhi->newTextureRenderTarget({ { mOutputTexture }, mDSBuffer.data() }));
			mRenderPassDesc.reset(mRenderTarget->newCompatibleRenderPassDescriptor());
			mRenderTarget->setRenderPassDescriptor(mRenderPassDesc.data());
			mRenderTarget->create();
		}
    }
	void render(QRhiCommandBuffer* cb) override {
		const QColor clearColor = QColor::fromRgbF(1.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cb->beginPass(mRenderTarget.data(), clearColor, dsClearValue);
		cb->endPass();
	}

protected:
    QRhi *mRhi = nullptr;
    QRhiTexture* mOutputTexture = nullptr;
    QScopedPointer<QRhiRenderBuffer> mDSBuffer;
    QScopedPointer<QRhiTextureRenderTarget> mRenderTarget;
    QScopedPointer<QRhiRenderPassDescriptor> mRenderPassDesc;
};

#endif

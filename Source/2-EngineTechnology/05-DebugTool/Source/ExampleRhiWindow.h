#ifndef ExampleRhiWindow_h__
#define ExampleRhiWindow_h__

#include "Render/RHI/QRhiWindow.h"

class ExampleRhiWindow : public QRhiWindow {
public:
	ExampleRhiWindow(QRhiWindow::InitParams inInitParams):QRhiWindow(inInitParams) {}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* currentCmdBuffer = mSwapChain->currentFrameCommandBuffer();

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 1.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue);
		currentCmdBuffer->endPass();
	}
};

#endif // ExampleRhiWindow_h__

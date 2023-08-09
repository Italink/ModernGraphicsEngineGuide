#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	QRenderWidget widget(initParams);
	widget.setupCamera();
	widget.showMaximized();
	return app.exec();
}

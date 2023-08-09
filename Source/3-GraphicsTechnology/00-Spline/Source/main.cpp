#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QSplineRenderComponent.h"

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera()
		->setPosition(QVector3D(0,0, 200));

	QList<QSplinePoint> points;

	int numOfPoint = 10;
	int scaleFactor = 10;

	for (int i = 0; i < numOfPoint; i++) {
		QSplinePoint point;
		point.mPoint.setX((i - numOfPoint/2 ) * scaleFactor);
		point.mPoint.setY(i % 2 ? scaleFactor : -scaleFactor);
		points << point;
	}

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QSplineRenderComponent::Create("Spline")
				.setPoints(points)
			)
		)
		.end("BasePass", QBasePassForward::BaseColor)
	);

	widget.showMaximized();
	return app.exec();
}
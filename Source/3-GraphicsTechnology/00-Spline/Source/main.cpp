#include "QEngineApplication.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "QRenderWidget.h"
#include "Render/Component/QSplineRenderComponent.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QSplineRenderComponent mSplineComp;
	QSharedPointer<QMeshPassBuilder> mMeshPass{ new QMeshPassBuilder };
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		QList<QSplinePoint> points;

		int numOfPoint = 10;
		int scaleFactor = 10;

		for (int i = 0; i < numOfPoint; i++) {
			QSplinePoint point;
			point.mPoint.setX((i - numOfPoint / 2) * scaleFactor);
			point.mPoint.setY(i % 2 ? scaleFactor : -scaleFactor);
			points << point;
		}

		mSplineComp.setPoints(points);
		mSplineComp.setLineWidth(50);

		addComponent(&mSplineComp);

		setCurrentObject(&mSplineComp);

		getCamera()->setRotation(QVector3D(0, 90, 0));
		getCamera()->setPosition(QVector3D(0, 0, 150));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder("MeshPass", mMeshPass);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(meshOut.BaseColor);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}
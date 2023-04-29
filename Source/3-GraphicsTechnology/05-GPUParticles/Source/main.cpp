#include <QApplication>
#include "QRenderWidget.h"
#include "QRandomGenerator"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "QPainter"

class MyGpuParticleEmitter : public QGpuParticleEmitter {
	Q_OBJECT
	Q_PROPERTY(QImage PositionSampleImage READ getImage WRITE setImage)
private:
	QImage mImage;
	QImage getImage() const { return mImage; }
	void setImage(QImage val) { mImage = val; }
public:
	MyGpuParticleEmitter() {
		mImage = QImage(200, 200, QImage::Format_Alpha8);
		mImage.fill(Qt::transparent);
		QPainter painter(&mImage);
		painter.setBrush(Qt::white);
		painter.setFont(QFont("", 200));
		painter.drawText(QRect(0, 0, 200, 200), Qt::AlignCenter, "GPU");
		painter.end();
		mImage.mirror();
		QGpuParticleEmitter::InitParams params;
		QVector<QVector4D> positionPool;
		for (int i = 0; i < mImage.width(); i++) {
			for (int j = 0; j < mImage.height(); j++) {
				QColor color = mImage.pixelColor(i, j);
				if (color.alpha()) {
					positionPool << QVector4D(i / (float)mImage.width() - 0.5, j / (float)mImage.height() - 0.5f, 0, 1) * 3;
				}
			}
		}
		params.spawnParams->addParam("MinSize", 0.0f);
		params.spawnParams->addParam("MaxSize", 0.3f);
		params.spawnParams->addParam("PositionPool", positionPool, false);

		params.spawnDefine = R"(	
			float rand(float seed, float min, float max){
				return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
			}
		)";

		params.spawnCode = R"(		
			outParticle.age = 0.0f;
			outParticle.lifetime = 3.0f;
			outParticle.scaling = vec3(rand(0.45,Params.MinSize,Params.MaxSize));
			
			float noiseStrength = 0.01;
			vec3 noiseOffset = vec3(rand(0.12,-noiseStrength,noiseStrength),rand(0.11561,-noiseStrength,noiseStrength),0);
			outParticle.position = Params.PositionPool[int(rand(0.13212,0,Params.PositionPool.length()))].xyz + noiseOffset;

			outParticle.velocity = vec3(0,rand(0.124,0,0.0001),rand(0.4451,-0.00005,0.000000));
		)";

		params.updateDefine = R"(	
			float rand(float seed, float min, float max){
				return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
			}
		)";

		params.updateCode = R"(		
			outParticle.age	 = inParticle.age + UpdateCtx.deltaSec;
			outParticle.position = inParticle.position + inParticle.velocity;
			outParticle.velocity = inParticle.velocity + vec3(rand(0.41,-0.000001,0.000001),0,0);
			outParticle.scaling  = inParticle.scaling;
			outParticle.rotation = inParticle.rotation;
		)";
		setupParams(params);
	}
};

class MyCpuParticleEmitter : public QCpuParticleEmitter {
	Q_OBJECT
	Q_PROPERTY(float MinSize READ getMinSize WRITE setMinSize)
	Q_PROPERTY(float MaxSize READ getMaxSize WRITE setMaxSize)

public:
	float getMinSize() const { return mMinSize; }
	void setMinSize(float val) { mMinSize = val; }
	float getMaxSize() const { return mMaxSize; }
	void setMaxSize(float val) { mMaxSize = val; }
	float rand(float min, float max) {return mRandom.generateDouble() * (max - min) + min;}
	MyCpuParticleEmitter() {
		setNumOfSpawnPerFrame(10);
	}
private:
	QRandomGenerator mRandom;
	float mMinSize = 0.0f;
	float mMaxSize = 2.0f;
protected:
	void onSpawn(Particle& outParticle) override {
		outParticle.age = 0.0f;
		outParticle.lifetime = 2.0f;
		float uniformScale = rand(mMinSize, mMaxSize);
		outParticle.scaling = QVector3D(uniformScale, uniformScale, uniformScale);
		outParticle.position = QVector3D(rand(-2, 2), rand(-2, 2), rand(-2,2));
		outParticle.velocity = QVector3D(0.0, 0, rand(0.0001, 0.01));
	}
	void onUpdate(Particle& outParticle) override {
		if (outParticle.position.z() > 1)
			outParticle.age = outParticle.lifetime;
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRenderWidget widget(initParams);

	widget.setupCamera();

	widget.setFrameGraph(
		QFrameGraph::Begin()
		.addPass(
			QBasePassForward::Create("BasePass")
			.addComponent(
				QParticlesRenderComponent::Create("GPU Particles")
				.setEmitter(new MyGpuParticleEmitter)
			)
			.addComponent(
				QParticlesRenderComponent::Create("CPU Particles")
				.setEmitter(new MyCpuParticleEmitter)
			)
		)
		.end("BasePass",QBasePassForward::BaseColor)
	);

	widget.showMaximized();
	return app.exec();
}

#include "main.moc"
#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include <QPainter>
#include <QPainterPath>
#include "QtConcurrent/qtconcurrentrun.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/RenderGraph/PassBuilder/PBR/QPbrMeshPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QDepthOfFieldPassBuilder.h"

#define Q_PROPERTY_VAR(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
    } \
    Type Name

class MyGpuParticleEmitter : public QGpuParticleEmitter {
public:
	MyGpuParticleEmitter() {
		QGpuParticleEmitter::InitParams params;

		params.spawnParams->addParam("MinSize", 0.0f);
		params.spawnParams->addParam("MaxSize", 0.01f);

		params.spawnDefine = R"(	
			float rand(float seed, float min, float max){
				return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
			}
		)";

		params.spawnCode = R"(		
			outParticle.age = 0.0f;
			outParticle.lifetime = 3.0f;
			outParticle.scaling = vec3(rand(0.45,Params.MinSize,Params.MaxSize));
			
			const float noiseStrength = 10;
			vec3 noiseOffset = vec3(rand(0.12,-noiseStrength,noiseStrength),0,rand(0.11561,-noiseStrength,noiseStrength));
			outParticle.position = noiseOffset;
			outParticle.velocity = vec3(0,rand(0.124,0,0.05),rand(0.4451,-0.0005,0.0005));
		)";

		params.updateDefine = R"(	
			float rand(float seed, float min, float max){
				return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
			}
		)";

		params.updateCode = R"(		
			outParticle.age	 = inParticle.age + UpdateCtx.deltaSec;
			outParticle.position = inParticle.position + inParticle.velocity * UpdateCtx.deltaSec;
			outParticle.velocity = inParticle.velocity + vec3(rand(0.41,-0.1,0.1),0,0)* UpdateCtx.deltaSec;
			outParticle.scaling  = inParticle.scaling;
			outParticle.rotation = inParticle.rotation;
		)";
		setupParams(params);
		mNumOfSpawnPerFrame = 1000;
	}
	void onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer) override {
		QGpuParticleEmitter::onUpdateAndRecyle(inCmdBuffer);
	}
};

class MyRenderer : public IRenderer {
	Q_OBJECT
	Q_PROPERTY_VAR(float, Focus) = 0.05f;
	Q_PROPERTY_VAR(float, FocalLength) = 20.0f;
	Q_PROPERTY_VAR(float, Aperture) = 10.5f;
	Q_PROPERTY_VAR(int, ApertureBlades) = 5;
	Q_PROPERTY_VAR(float, BokehSqueeze) = 0.0f;
	Q_PROPERTY_VAR(float, BokehSqueezeFalloff) = 1.0f;
	Q_PROPERTY_VAR(int, Iterations) = 64;
private:
	QParticlesRenderComponent mParticlesComp;
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
		QImage image(100, 100, QImage::Format_RGBA8888);
		image.fill(Qt::transparent);
		QPainter painter(&image);
		QPoint center(50, 50);
		QPainterPath path;
		for (int i = 0; i < 5; i++) {
			float x1 = qCos((18 + 72 * i) * M_PI / 180) * 50,
				y1 = qSin((18 + 72 * i) * M_PI / 180) * 50,
				x2 = qCos((54 + 72 * i) * M_PI / 180) * 20,
				y2 = qSin((54 + 72 * i) * M_PI / 180) * 20;

			if (i == 0) {
				path.moveTo(x1 + center.x(), y1 + center.y());
				path.lineTo(x2 + center.x(), y2 + center.y());
			}
			else {
				path.lineTo(x1 + center.x(), y1 + center.y());
				path.lineTo(x2 + center.x(), y2 + center.y());
			}
		}
		path.closeSubpath();
		painter.fillPath(path, Qt::white);
		mParticlesComp.setParticleShape(QStaticMesh::CreateFromImage(image));
		mParticlesComp.setEmitter(new MyGpuParticleEmitter);

		addComponent(&mParticlesComp);

		getCamera()->setPosition(QVector3D(0.0f, 0.1f, 10.0f));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		QPbrMeshPassBuilder::Output meshOut
			= graphBuilder.addPassBuilder<QPbrMeshPassBuilder>("MeshPass");

		QDepthOfFieldPassBuilder::Output dofOut = graphBuilder.addPassBuilder<QDepthOfFieldPassBuilder>("DepthOfFieldPass")
			.setBaseColorTexture(meshOut.BaseColor)
			.setPositionTexture(meshOut.Position)
			.setFocus(Focus)
			.setFocalLength(FocalLength)
			.setAperture(Aperture)
			.setApertureBlades(ApertureBlades)
			.setBokehSqueeze(BokehSqueeze)
			.setBokehSqueezeFalloff(BokehSqueezeFalloff)
			.setIterations(Iterations);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(dofOut.DepthOfFieldResult);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}

#include "main.moc"
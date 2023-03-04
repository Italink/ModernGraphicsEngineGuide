#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include "private\qshaderbaker_p.h"

QShader newShaderFromCode(QShader::Stage stage, const char* code) {
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	baker.setGeneratedShaders({
		QShaderBaker::GeneratedShader{QShader::Source::SpirvShader,QShaderVersion(100)},
		QShaderBaker::GeneratedShader{QShader::Source::GlslShader,QShaderVersion(430)},
		QShaderBaker::GeneratedShader{QShader::Source::MslShader,QShaderVersion(12)},
		QShaderBaker::GeneratedShader{QShader::Source::HlslShader,QShaderVersion(60)},
		});

	baker.setSourceString(code, stage);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		qWarning(code);
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader createShaderFromFile(QShader::Stage stage, const char* filename) {
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly))
		return newShaderFromCode(stage,file.readAll().constData());
	return QShader();
}

QShader newShaderFromQSBFile(const char* filename) {
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(file.readAll());
	return QShader();
}

void RunQtShaderTool() {
	QProcess runShaderTool;
	runShaderTool.setProgram("qsb");
	runShaderTool.setProcessChannelMode(QProcess::MergedChannels);

	runShaderTool.setArguments({
		"--glsl","430",
		"--msl","12" ,
		"-c", "color.frag",
		"-o","color.frag.qsb" });
	runShaderTool.start();
	runShaderTool.waitForFinished();
	qDebug() << runShaderTool.readAllStandardOutput().constData();

	runShaderTool.setArguments({
	"--glsl","430",
	"--msl","12" ,
	"-c", "color.vert",
	"-o","color.vert.qsb" });

	runShaderTool.start();
	runShaderTool.waitForFinished();

	qDebug() << runShaderTool.readAllStandardOutput().constData();
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QDir::setCurrent(RESOURCE_DIR);

	RunQtShaderTool();

    return app.exec();
}
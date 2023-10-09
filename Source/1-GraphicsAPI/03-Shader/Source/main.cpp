#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include "QWidget"
#include "QTextEdit"
#include "QTextBrowser"
#include "QBoxLayout"
#include "QPushButton"
#include "QLabel"
#include "rhi/qshader.h"
#include "rhi/qshaderbaker.h"

QShader newShaderFromCode(QShader::Stage stage, const char* code) {
	QShaderBaker baker;						//着色器烘培器
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	baker.setGeneratedShaders({
	    QShaderBaker::GeneratedShader{QShader::Source::SpirvShader,QShaderVersion(100)},
		QShaderBaker::GeneratedShader{QShader::Source::GlslShader,QShaderVersion(430)},
		QShaderBaker::GeneratedShader{QShader::Source::MslShader,QShaderVersion(12)},
		QShaderBaker::GeneratedShader{QShader::Source::HlslShader,QShaderVersion(60)},
	});
	baker.setSourceString(code, stage);		//装配GLSL源码
	QShader shader = baker.bake();			//执行烘培，将之编译成各个图形API的着色器代码

	if (!shader.isValid()) {				//打印编译报错
		QStringList codelist = QString(code).split('\n');
		for (int i = 0; i < codelist.size(); i++) {
			qWarning() << i + 1 << codelist[i].toLocal8Bit().data();
		}
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader newShaderFromFile(QShader::Stage stage, const char* filename) {
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

void RunQtShaderToolByQProcess() {
	QProcess runShaderTool;
	runShaderTool.setProgram("qsb");
	runShaderTool.setProcessChannelMode(QProcess::MergedChannels);
	runShaderTool.setArguments({
		"--glsl","430",
		"--msl","12" ,
		"--hlsl","60",
		"-c", "color.frag",
		"-o","color.frag.qsb" });

	runShaderTool.start();
	runShaderTool.waitForFinished();
	qDebug() << runShaderTool.readAllStandardOutput().constData();
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QDir::setCurrent("Resources/Shader");

	QWidget main;

	QGridLayout* layout = new QGridLayout(&main);

	QPushButton* btCompile = new QPushButton("Vulkan GLSL");
	QTextEdit* editor = new QTextEdit;
	QTextBrowser* glslBrowser = new QTextBrowser;
	QTextBrowser* hlslBrowser = new QTextBrowser;
	QTextBrowser* mslBrowser = new QTextBrowser;

	layout->addWidget(btCompile, 0, 0);
	layout->addWidget(editor, 1, 0);

	layout->addWidget(new QLabel("OpenGL GLSL"), 0, 1, Qt::AlignCenter);
	layout->addWidget(glslBrowser, 1, 1);

	layout->addWidget(new QLabel("HLSL"), 0, 2, Qt::AlignCenter);
	layout->addWidget(hlslBrowser, 1, 2);

	layout->addWidget(new QLabel("MSL"), 0, 3, Qt::AlignCenter);
	layout->addWidget(mslBrowser, 1, 3);

	editor->setTabStopDistance(16);
	editor->setText(R"(#version 430
layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outFragColor;
layout(std140, binding = 0) uniform UniformBlock {
	float opacity;
}UBO;
void main()
{
	outFragColor = vec4(vColor, UBO.opacity);
})");

	QObject::connect(btCompile, &QPushButton::clicked, [&]() {
		QShader shader = newShaderFromCode(QShader::FragmentStage, editor->toPlainText().toLocal8Bit());
		glslBrowser->setText(shader.shader(QShaderKey(QShader::GlslShader, QShaderVersion(430))).shader());
		hlslBrowser->setText(shader.shader(QShaderKey(QShader::HlslShader, QShaderVersion(60))).shader());
		mslBrowser->setText(shader.shader(QShaderKey(QShader::MslShader, QShaderVersion(12))).shader());
	});

	main.setMinimumSize(1200,800);
	main.show();

    return app.exec();
}
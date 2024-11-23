//#include "DetailCustomization_QGlslSandboxRenderPass.h"
//#include "DetailView/QPropertyHandle.h"
//#include "DetailView/QDetailViewManager.h"
//#include "CodeEditor/QGLSLEditor.h"
//#include "DetailView/QDetailViewRow.h"
//#include "QPushButton"
//
//void DetailCustomization_QGlslSandboxRenderPass::customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
//	QGlslSandboxRenderPass* pass = (QGlslSandboxRenderPass*)Context.ObjectPtr;
//	QWidget* page = new QWidget;
//	QVBoxLayout* vLayout = new QVBoxLayout(page);
//	QGLSLEditor* editor = new QGLSLEditor;
//	QPushButton* btCompile = new QPushButton("Compile");
//	editor->setDisplayText(pass->getShaderCode());
//	vLayout->addWidget(btCompile);
//	vLayout->addWidget(editor);
//	QObject::connect(btCompile, &QPushButton::clicked, [pass, editor]() {
//		pass->setShaderCode(editor->text());
//	});
//	Builder->setPage(page);
//}

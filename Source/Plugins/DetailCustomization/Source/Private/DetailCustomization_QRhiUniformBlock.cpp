#include "DetailCustomization_QRhiUniformBlock.h"
#include "Render/RHI/QRhiUniformBlock.h"
#include "DetailView/QPropertyHandle.h"
#include "DetailView/QDetailViewManager.h"

void DetailCustomization_QRhiUniformBlock::customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QRhiUniformBlock* uniformBlock = (QRhiUniformBlock*)Context.ObjectPtr;
	for (const QSharedPointer<UniformParamDescBase>& param : uniformBlock->getParamList()) {
		if(!param->bVisible)
			continue;
		QPropertyHandle* handler = QPropertyHandle::FindOrCreate(
			uniformBlock,
			param->mValue.metaType(),
			param->mName,
			[weakParam = param.toWeakRef()]() {
				return weakParam.lock()->mValue;
			},
			[weakParam = param.toWeakRef()](QVariant var) {
				weakParam.lock()->setValue(var);
			}
		);
		Builder->addProperty(handler);
	}
}

#include "DetailCustomization_QRhiMaterialGroup.h"
#include <QMetaProperty>
#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"
#include "Render/RHI/QRhiMaterialGroup.h"

void DetailCustomization_QRhiMaterialGroup::customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QRhiMaterialGroup* group = (QRhiMaterialGroup*) Context.ObjectPtr;
	if (!group) {
		return;
	}
	int index = 0;
	for (auto materialDesc : group->getDescList()) {
		auto rowBuilder = Builder->addRowByNameValueWidget(QString::number(index++), nullptr);
		rowBuilder->addObject(materialDesc->uniformBlock.get());
		for (auto textureDesc : materialDesc->textureDescs) {
			QString path = "Textures." + textureDesc->Name;
			QPropertyHandle* handler = QPropertyHandle::FindOrCreate(
				group,
				QMetaType::fromType<QImage>(),
				path,
				[textureInfo = textureDesc]() {
					return textureInfo->ImageCache;
				},
				[textureInfo = textureDesc](QVariant var) {
					textureInfo->ImageCache = var.value<QImage>();
				}
			);
			rowBuilder->addProperty(handler);
		}
	}
}

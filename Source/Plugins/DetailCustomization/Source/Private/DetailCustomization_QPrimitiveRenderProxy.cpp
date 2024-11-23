#include "DetailCustomization_QPrimitiveRenderProxy.h"
#include <QMetaProperty>
#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"
#include "Render/QPrimitiveRenderProxy.h"

void DetailCustomization_QPrimitiveRenderProxy::customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QPrimitiveRenderProxy* pipeline = (QPrimitiveRenderProxy*) Context.ObjectPtr;
	QPropertyHandle* uniformblocksHandle = QPropertyHandle::FindOrCreate(pipeline, "UniformBlocks");
	for (auto uniformblock : pipeline->getUniformBlocks().keys()) {
		if(uniformblock != "Transform")
			Builder->addProperty(uniformblocksHandle->createChildHandle(uniformblock));
	}
	for (auto texture : pipeline->getTextures().asKeyValueRange()) {
		QString path = "Textures." + texture.first;
		QPropertyHandle* handler = QPropertyHandle::FindOrCreate(
			pipeline,
			QMetaType::fromType<QImage>(),
			path,
			[textureInfo = texture.second]() {
				return textureInfo->ImageCache;
			},
			[textureInfo = texture.second](QVariant var) {
				textureInfo->ImageCache = var.value<QImage>();
			}
		);
		Builder->addProperty(handler);
	}
}

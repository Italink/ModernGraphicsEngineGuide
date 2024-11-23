#include "PropertyTypeCustomization_TextureInfo.h"
#include "Render/QPrimitiveRenderProxy.h"

void PropertyTypeCustomization_TextureInfo::customizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	QRhiTextureDesc* textureInfo = PropertyHandle->getValue().value<QRhiTextureDesc*>();
	Builder->setNameValueWidget(PropertyHandle->generateNameWidget(), nullptr);
}

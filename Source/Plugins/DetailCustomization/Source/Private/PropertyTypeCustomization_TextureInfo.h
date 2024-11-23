#ifndef PropertyTypeCustomization_TextureInfo_h__
#define PropertyTypeCustomization_TextureInfo_h__

#include "DetailView/IPropertyTypeCustomization.h"

class PropertyTypeCustomization_TextureInfo :public IPropertyTypeCustomization{
public:
	void customizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) override;
};

#endif // PropertyTypeCustomization_TextureInfo_h__

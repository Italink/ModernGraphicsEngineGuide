#ifndef DetailCustomization_QRhiMaterialGroup_h__
#define DetailCustomization_QRhiMaterialGroup_h__

#include "DetailView/IDetailCustomization.h"

class QRhiMaterialGroup;

class DetailCustomization_QRhiMaterialGroup : public IDetailCustomization {
protected:
	void customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) override;
};

#endif // DetailCustomization_QRhiMaterialGroup_h__

#ifndef DetailCustomization_QRhiUniformBlock_h__
#define DetailCustomization_QRhiUniformBlock_h__

#include "DetailView/IDetailCustomization.h"

class DetailCustomization_QRhiUniformBlock : public IDetailCustomization {
protected:
	void customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) override;
};

#endif // DetailCustomization_QRhiUniformBlock_h__

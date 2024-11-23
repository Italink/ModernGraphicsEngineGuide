#ifndef DetailCustomization_QPrimitiveRenderProxy_h__
#define DetailCustomization_QPrimitiveRenderProxy_h__

#include "DetailView/IDetailCustomization.h"

class QPrimitiveRenderProxy;

class DetailCustomization_QPrimitiveRenderProxy : public IDetailCustomization {
protected:
	void customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) override;
};

#endif // DetailCustomization_QPrimitiveRenderProxy_h__

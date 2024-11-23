#ifndef DetailCustomizationPlugin_h__
#define DetailCustomizationPlugin_h__

#include "Plugin/IEnginePlugin.h"

class DetailCustomizationPlugin : public IEnginePlugin {
protected:	
	Info info() override;
	void startup() override;
	void shutdown() override;
};

#endif // DetailCustomizationPlugin_h__

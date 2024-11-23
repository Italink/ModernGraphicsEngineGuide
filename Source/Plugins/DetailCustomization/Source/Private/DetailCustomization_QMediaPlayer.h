#ifndef DetailCustomization_QMediaPlayer_h__
#define DetailCustomization_QMediaPlayer_h__

#include "DetailView/IDetailCustomization.h"
#include <QtMultimedia/QMediaPlayer>

class DetailCustomization_QMediaPlayer : public IDetailCustomization {
protected:
	void customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) override;
};

#endif // DetailCustomization_QMediaPlayer_h__

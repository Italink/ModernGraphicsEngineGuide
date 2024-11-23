#include "DetailCustomization_QMediaPlayer.h"
#include "DetailView/QPropertyHandle.h"
#include "DetailView/QDetailViewManager.h"
#include "Widgets/QMediaPlayerEditor.h"


void DetailCustomization_QMediaPlayer::customizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QMediaPlayer* player = (QMediaPlayer*)Context.ObjectPtr;
	QMediaPlayerEditor* editor = new QMediaPlayerEditor;
	editor->setupPlayer(player);
	Builder->addRowByWholeContent(editor);
}

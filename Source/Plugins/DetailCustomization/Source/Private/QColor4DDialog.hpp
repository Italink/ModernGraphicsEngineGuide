#ifndef QColor4DDialog_h__
#define QColor4DDialog_h__

#include "QColor4DChannelSlider.hpp"
#include "QPushButton"
#include "Type/QColor4D.h"
#include "Widgets/Color/Component/ColorLineEdit.hpp"
#include "Widgets/Color/Component/ColorPreview.hpp"
#include "Widgets/Color/Component/ColorWheel.hpp"
#include <QDialog>

class QColor4DDialog :public QDialog {
	Q_OBJECT
public:
	QColor4DDialog();
	~QColor4DDialog();
	void setColor(QColor4D color);
	static int CreateAndShow(QColor4D color, QRect inButtonGemotry);
	void setCloseWhenLoseFocus(bool val) { bCloseWhenLoseFocus = val; }
private:
	void createUI();
	void connectUI();
	void setCurrentColorInternal(QColor4D color);
	void refleshChannelGradiant();
Q_SIGNALS:
	void asColorChanged(QColor4D);
private:
	ColorWheel* mColorWheel;
	ColorPreview* mColorPreview;
	QColor4DChannelSlider* mRedBox;
	QColor4DChannelSlider* mGreenBox;
	QColor4DChannelSlider* mBlueBox;
	QColor4DChannelSlider* mAlphaBox;
	QColor4DChannelSlider* mHueBox;
	QColor4DChannelSlider* mSaturationBox;
	QColor4DChannelSlider* mValueBox;
	QPushButton* mPbPick;
	QPushButton* mPbOk;
	QPushButton* mPbCancel;
	ColorLineEdit* mLeHex;
	QColor4D mCurrentColor;
	QColor4D mLastColor;
	bool bCloseWhenLoseFocus = false;
	using QWidget = QWidget;
public:
	inline static QColor4DDialog* Current = nullptr;
protected:
	void focusOutEvent(QFocusEvent* event) override;

};

#endif // QColor4DDialog_h__
#ifndef QColor4DButton_h__
#define QColor4DButton_h__

#include "Type/QColor4D.h"
#include "Widgets/QHoverWidget.h"

class QColor4DButton :public QHoverWidget {
	Q_OBJECT
public:
	QColor4DButton(QColor4D color = QColor4D());
	void setColor(QColor4D color);
	QColor4D GetColor() const;
Q_SIGNALS:
	void asColorChanged(QColor4D color);
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	QColor4D mColor;
};

#endif // QColor4DButton_h__

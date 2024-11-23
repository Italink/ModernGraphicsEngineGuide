#include "QColor4DButton.hpp"
#include <QPainter>
#include "QColor4DDialog.hpp"
#include "QEngineEditorStyleManager.h"

QColor4DButton::QColor4DButton(QColor4D color)
	: mColor(color) {
	setMinimumWidth(100);
	setFixedHeight(20);
	setColor(color);
}

void QColor4DButton::setColor(QColor4D color) {
	mColor = color;
	update();
}

QColor4D QColor4DButton::GetColor() const {
	return mColor;
}

void QColor4DButton::paintEvent(QPaintEvent* event) {
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);
	painter.setBrush(mColor.toQColor());
	painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 2, 2);
}

void QColor4DButton::mousePressEvent(QMouseEvent* event) {
	QHoverWidget::mousePressEvent(event);
	QRect geom = rect();
	geom.moveTopLeft(mapToGlobal(QPoint(0, 0)));
	QColor4DDialog::CreateAndShow(mColor, geom);
	QColor4DDialog::Current->setStyleSheet(QEngineEditorStyleManager::Instance()->getStylesheet());
	QObject::connect(QColor4DDialog::Current, &QColor4DDialog::asColorChanged, this, [&](const QColor& color) {
		setColor(color);
		Q_EMIT asColorChanged(mColor);
	});
}

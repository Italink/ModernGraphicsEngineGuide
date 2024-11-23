#include "QColor4DChannelSlider.hpp"
#include "QBoxLayout"
#include "QPainter"
#include "QValidator"
#include "qevent.h"

void QColor4DChannelValueBox::createUI() {
	setAttribute(Qt::WA_StyledBackground);
	setFixedHeight(20);
	setStyleSheet("QLineEdit{background-color:transparent; font-size:11px;}");
	resize(60, 30);
	mLeValue = new QLineEdit_HasFocusSignal;
	mLbArrow = new QLabel;
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(2, 2, 2, 2);
	h->setSpacing(0);
	h->addWidget(mLeValue);
	h->addWidget(mLbArrow);
	mLbArrow->setFixedSize(1, height());
	mLbArrow->setCursor(Qt::CursorShape::SizeHorCursor);
	mLeValue->setFixedHeight(height());
	mLeValue->setFrame(QFrame::NoFrame);
	mLeValue->setValidator(new QDoubleValidator);
	mLeValue->setAlignment(Qt::AlignLeft);
	mLeValue->setMinimumWidth(50);
	mLeValue->setText(QString::number(mValue));
	setEditEnabled(false);
}

void QColor4DChannelValueBox::connectUI() {
	connect(mLeValue, &QLineEdit_HasFocusSignal::loseFocus, this, [this]() {
		setEditEnabled(false);
	});
	connect(mLeValue, &QLineEdit::editingFinished, this, [this]() {
		float currValue = mLeValue->text().toFloat();
		if (currValue != mValue) {
			mValue = qMax(0.0f, currValue);
			Q_EMIT asValueChanged(mValue);
		}
	});
}

void QColor4DChannelValueBox::setEditEnabled(bool enable) {
	if (enable) {
		mLeValue->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
		mLeValue->setAttribute(Qt::WA_TransparentForMouseEvents, false);
		setCursor(Qt::CursorShape::IBeamCursor);
		mLeValue->activateWindow();
		mLeValue->setFocus();
		mLeValue->setReadOnly(false);
		mLeValue->selectAll();
	}
	else {
		mLeValue->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		mLeValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		setCursor(Qt::CursorShape::SizeHorCursor);
		mLeValue->setReadOnly(true);
	}
}

bool QColor4DChannelValueBox::getEditEnabled() {
	return mLeValue->focusPolicy() == Qt::FocusPolicy::StrongFocus;
}

QString QColor4DChannelValueBox::getDisplayText() {
	return mLeValue->text();
}

float QColor4DChannelValueBox::getVar() {
	return mValue;
}

void QColor4DChannelValueBox::setVar(float var) {
	mValue = var;
	mLeValue->setText(QString::number(mValue));
}

void QColor4DChannelValueBox::mousePressEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		mClickPosition = event->pos();
		if (getEditEnabled() && mLbArrow->geometry().contains(event->pos())) {
			setEditEnabled(false);
		}
	}
}

void QColor4DChannelValueBox::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (this->cursor() == Qt::BlankCursor) {
			setCursor(Qt::CursorShape::SizeHorCursor);
			mLbArrow->setCursor(Qt::CursorShape::SizeHorCursor);
		}
		else if (mClickPosition == event->pos() && !getEditEnabled() && this->cursor() != Qt::BlankCursor) {
			setEditEnabled(true);
		}
	}
}

void QColor4DChannelValueBox::mouseMoveEvent(QMouseEvent* event) {
	if (!getEditEnabled() && event->buttons() & Qt::LeftButton) {
		setCursor(Qt::BlankCursor);
		mLbArrow->setCursor(Qt::CursorShape::BlankCursor);
		QPointF offset = event->position() - mClickPosition;
		float newValue = mValue + offset.x() * 0.001f;
		if (newValue != mValue) {
			mValue = qMax(0.0f, newValue);
			Q_EMIT asValueChanged(mValue);
		}
		mLeValue->setText(QString::number(mValue));
		QCursor::setPos(mapToGlobal(mClickPosition.toPoint()));
	}
}

void QColor4DChannelValueBox::paintEvent(QPaintEvent* event) {
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	QRect overRect = rect().adjusted(3, 3, -3, -3);
	overRect.setWidth(overRect.width() * qMin(mValue, 1.0f));
	painter.fillRect(overRect, mHoverd ? mHoverColor : QColor(100, 100, 100, 50));
	
}

QSize QColor4DChannelValueBox::sizeHint() const {
	return { 60,30 };
}


QColor4DChannelSlider::QColor4DChannelSlider(QString inName, float inDefault)
	: mLbName(inName)
	, mValueBox(inDefault)
{
	QHBoxLayout* h = new QHBoxLayout(this);
	QVBoxLayout* v = new QVBoxLayout;

	h->setContentsMargins(0, 0, 0, 0);
	v->setContentsMargins(0, 0, 0, 0);
	v->setSpacing(0);

	h->addWidget(&mLbName);
	h->addLayout(v);

	v->addWidget(&mValueBox);
	v->addSpacing(5);

	connect(&mValueBox, &QColor4DChannelValueBox::asValueChanged, this, [this](float var) {
		Q_EMIT asValueChanged(var);
	});
}

void QColor4DChannelSlider::SetGradientStops(const QGradientStops& inStops)
{
	mGradientStops = inStops;
	update();
}

void QColor4DChannelSlider::SetChannelValue(float inValue)
{
	mValueBox.setVar(inValue);
}

void QColor4DChannelSlider::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect colorRect(0, 0, mValueBox.width(), 5);
	colorRect.moveTopLeft(mValueBox.geometry().bottomLeft());
	QLinearGradient linear;
	linear.setStart(colorRect.bottomLeft());
	linear.setFinalStop(colorRect.bottomRight());
	linear.setStops(mGradientStops);
	painter.fillRect(colorRect, linear);
}


#include "PropertyTypeCustomization_QMatrix4x4.h"
#include "QMatrix4x4"
#include "Utils/MathUtils.h"

void PropertyTypeCustomization_QMatrix4x4::customizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	Builder->setNameValueWidget(PropertyHandle->generateNameWidget(), PropertyHandle->generateValueWidget());
}

void PropertyTypeCustomization_QMatrix4x4::customizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) {
	QPropertyHandle* position = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->getSubPath("Position"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			return MathUtils::getMatTranslate(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			MathUtils::setMatTranslate(Mat, var.value<QVector3D>());
			PropertyHandle->setValue(Mat);
		}
	);
	Builder->addProperty(position);

	QPropertyHandle* rotation = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->getSubPath("Rotation"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			return MathUtils::getMatRotation(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			MathUtils::setMatRotation(Mat, var.value<QVector3D>());
			PropertyHandle->setValue(Mat);
		}
		);
	Builder->addProperty(rotation);

	QPropertyHandle* scale = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->getSubPath("Scale"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			return MathUtils::getMatScale3D(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->getValue().value<QMatrix4x4>();
			MathUtils::setMatScale3D(Mat, var.value<QVector3D>());
			PropertyHandle->setValue(Mat);
		}
		);
	Builder->addProperty(scale);
}

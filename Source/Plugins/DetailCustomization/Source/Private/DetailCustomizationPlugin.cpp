#include "DetailCustomizationPlugin.h"
#include <QDebug>
#include "Asset/QStaticMesh.h"
#include "DetailCustomization_QGlslSandboxRenderPass.h"
#include "DetailCustomization_QMediaPlayer.h"
#include "DetailCustomization_QRhiMaterialGroup.h"
#include "DetailCustomization_QRhiUniformBlock.h"
#include "DetailView/QDetailViewManager.h"
#include "PropertyTypeCustomization_QMatrix4x4.h"
#include "PropertyTypeCustomization_QStaticMesh.h"
#include "PropertyTypeCustomization_TextureInfo.h"
#include "QMediaPlayer"
#include "Render/QPrimitiveRenderProxy.h"
#include "Render/RHI/QRhiMaterialGroup.h"
#include "Render/RHI/QRhiUniformBlock.h"
#include "QColor4DButton.hpp"

IEnginePlugin::Info DetailCustomizationPlugin::info()
{
	IEnginePlugin::Info i;
	i.icon = QUrl("qrc:/Resources/delete.png");
	i.name = "DetailCustomizationPlugin";
	i.author = "italink";
	i.description = "this is a detail customization plugin";
	i.link = "github.com";
	return i;
}

void DetailCustomizationPlugin::startup() {
	qDebug() << "DetailCustomizationPlugin::startup";
	QDetailViewManager* mgr = QDetailViewManager::Instance();
	mgr->registerCustomClassLayout<DetailCustomization_QRhiUniformBlock>(&QRhiUniformBlock::staticMetaObject);
	mgr->registerCustomClassLayout<DetailCustomization_QRhiMaterialGroup>(&QRhiMaterialGroup::staticMetaObject);
	//mgr->registerCustomClassLayout<DetailCustomization_QGlslSandboxRenderPass>(&QGlslSandboxRenderPass::staticMetaObject);
	mgr->registerCustomClassLayout<DetailCustomization_QMediaPlayer>(&QMediaPlayer::staticMetaObject);

	mgr->registerCustomPropertyTypeLayout<QRhiTextureDesc*, PropertyTypeCustomization_TextureInfo>();
	mgr->registerCustomPropertyTypeLayout<QSharedPointer<QStaticMesh>, PropertyTypeCustomization_QStaticMesh>();
	mgr->registerCustomPropertyTypeLayout<QMatrix4x4, PropertyTypeCustomization_QMatrix4x4>();

	mgr->registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QColor4D>(), [](QPropertyHandle* InHandler) {
		QColor4DButton* colorButton = new QColor4DButton();
		InHandler->bind(
			colorButton, 
			&QColor4DButton::asColorChanged,
			[colorButton]() {
				return QVariant::fromValue<QColor4D>(colorButton->GetColor());
			},
			[colorButton](QVariant var) {
				colorButton->setColor(var.value<QColor4D>());
			}
		);
		return colorButton;
	});

	qRegisterMetaType<QRhiTextureDesc>();
	qRegisterMetaType<QPrimitiveRenderProxy*>();
	qRegisterMetaType<IStaticMeshCreator*>();
}

void DetailCustomizationPlugin::shutdown() {
	qDebug() << "DetailCustomizationPlugin::shutdown";
	QDetailViewManager* mgr = QDetailViewManager::Instance();
	mgr->unregisterCustomClassLayout(&QRhiUniformBlock::staticMetaObject);
	mgr->unregisterCustomClassLayout(&QRhiMaterialGroup::staticMetaObject);
	//mgr->unregisterCustomClassLayout(&QGlslSandboxRenderPass::staticMetaObject);
	mgr->unregisterCustomClassLayout(&QMediaPlayer::staticMetaObject);

	mgr->unregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QRhiTextureDesc*>());
	mgr->unregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QSharedPointer<QStaticMesh>>());
	mgr->unregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QMatrix4x4>());
	mgr->unregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QColor4D>());
}

QENGINE_IMPLEMENT_PLUGIN(DetailCustomizationPlugin, DetailCustomization)

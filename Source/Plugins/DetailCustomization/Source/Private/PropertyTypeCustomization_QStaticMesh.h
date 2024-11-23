#ifndef PropertyTypeCustomization_QStaticMesh_h__
#define PropertyTypeCustomization_QStaticMesh_h__

#include "DetailView/IPropertyTypeCustomization.h"
#include "QComboBox"
#include "Asset/QStaticMesh.h"

#define Q_STATIC_MESH_CREATE_PROPERTY(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
		Q_EMIT AsCreateMesh(create()); \
    } \
    Type Name

class IStaticMeshCreator : public QObject {
	Q_OBJECT
public:
	virtual QSharedPointer<QStaticMesh> create() = 0;
	Q_SIGNAL void AsCreateMesh(QSharedPointer<QStaticMesh>);
};
Q_DECLARE_METATYPE(QSharedPointer<IStaticMeshCreator>)

class QStaticMeshCreator_FromFile : public IStaticMeshCreator {
	Q_OBJECT
	Q_CLASSINFO("FilePath", "Type=FilePath")
public:
	Q_INVOKABLE QStaticMeshCreator_FromFile() {}

	QSharedPointer<QStaticMesh> create() override {
		return QStaticMesh::CreateFromFile(FilePath);
	}

	Q_STATIC_MESH_CREATE_PROPERTY(QString, FilePath);
};

class QStaticMeshCreator_FromText : public IStaticMeshCreator {
	Q_OBJECT
public:
	Q_INVOKABLE QStaticMeshCreator_FromText() {}

	QSharedPointer<QStaticMesh> create() override {
		return QStaticMesh::CreateFromText(Text,Font,Color,Orientation,Spacing,UseTexture);
	}
	Q_STATIC_MESH_CREATE_PROPERTY(QString, Text) = "Text";
	Q_STATIC_MESH_CREATE_PROPERTY(QFont, Font);
	Q_STATIC_MESH_CREATE_PROPERTY(QColor, Color) = Qt::white;
	Q_STATIC_MESH_CREATE_PROPERTY(Qt::Orientation, Orientation) = Qt::Horizontal;
	Q_STATIC_MESH_CREATE_PROPERTY(int, Spacing) = 0;
	Q_STATIC_MESH_CREATE_PROPERTY(bool, UseTexture) = false;
};

class QStaticMeshCreator_FromCube : public IStaticMeshCreator {
	Q_OBJECT
public:
	Q_INVOKABLE QStaticMeshCreator_FromCube() {}

	QSharedPointer<QStaticMesh> create() override {
		return nullptr;
	}

	Q_STATIC_MESH_CREATE_PROPERTY(float, Size) = 1;
};

class QStaticMeshCreator_FromSphere : public IStaticMeshCreator {
	Q_OBJECT
public:
	Q_INVOKABLE QStaticMeshCreator_FromSphere() {}
	QSharedPointer<QStaticMesh> create() override {
		return nullptr;
	}

	Q_STATIC_MESH_CREATE_PROPERTY(float, Radius) = 1;
	Q_STATIC_MESH_CREATE_PROPERTY(int, HDivide) = 64;
	Q_STATIC_MESH_CREATE_PROPERTY(int, VDivide) = 64;
};

class QStaticMeshCreator_FromGrid : public IStaticMeshCreator {
	Q_OBJECT
public:
	Q_INVOKABLE QStaticMeshCreator_FromGrid() {}
	QSharedPointer<QStaticMesh> create() override {
		return nullptr;
	}

	Q_STATIC_MESH_CREATE_PROPERTY(float, Width) = 1;
	Q_STATIC_MESH_CREATE_PROPERTY(float, Height) = 1;
	Q_STATIC_MESH_CREATE_PROPERTY(int, HDivide) = 64;
	Q_STATIC_MESH_CREATE_PROPERTY(int, VDivide) = 64;
};

class PropertyTypeCustomization_QStaticMesh :public IPropertyTypeCustomization {
public:
	void customizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) override;
	void customizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) override;
private:
	QComboBox* CurrComboBox = nullptr;
	static QSharedPointer<IStaticMeshCreator> mCreator;
};

#endif // PropertyTypeCustomization_QStaticMesh_h__

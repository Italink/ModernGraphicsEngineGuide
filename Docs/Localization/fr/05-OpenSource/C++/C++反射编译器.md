---
comments: true
---

# Compilateur de réflexion C++

- Dépôt Github : https://github.com/Italink/XObject

## Construction

- Utiliser CMake (GUI) pour construire directement

## Brève description de la structure du projet

- Core : Module principal
  - 3rdParty : Bibliothèques tierces
  - XHT : Outil d'analyse de code
  - Test : Répertoire du projet de test des fonctionnalités du module
  - XObject : Encapsulation de la classe de base

## Brève description de la logique

### Classe de base XObject

XObject sert de classe de base pour tous les objets et fournit des opérations telles que la sérialisation et la réflexion.

### Réflexion

La bibliothèque utilisée pour la réflexion est Rttr. Rttr est une excellente bibliothèque de réflexion C++ qui ne se contente pas de fournir des informations sur les types simples, mais offre également un ensemble complet d'interfaces de traitement de réflexion :

- rttr::variant : Son utilisation est similaire à stl::any, mais elle est plus puissante et propose de nombreuses interfaces d'informations sur les types et de conversion (par exemple, une variant de type int peut être directement exportée en double, ce qui appelle l'interface de conversion de int vers double ; dans rttr, il est possible de définir des fonctions de conversion de type personnalisées pour variant).
- Prend en charge la création d'instances correspondantes selon le nom du type et propose trois modes de création (Type, Type\*, std::shared_ptr\<Type\*>).
- metadata est utilisé pour ajouter des informations, et grâce à cette fonctionnalité, il est possible d'ajouter de nombreuses informations supplémentaires à Property.

XHT est utilisé pour l'analyse du code (en se référant à QtMoc) et complète l'enregistrement automatique des types grâce aux marques de macros, résolvant ainsi le problème où RTTR nécessite l'écriture manuelle de fonctions d'enregistrement.

> XHT peut sembler dispensable, et il est certainement possible d'écrire les fonctions d'enregistrement soi-même, mais il existe de meilleurs raisons de l'utiliser :
>
> - Ce n'est pas seulement Rttr qui nécessite l'écriture de fonctions d'enregistrement ; toutes les fonctionnalités qui impliquent la sérialisation ou d'autres fonctionnalités supplémentaires pour les types nécessitent également de l'écriture. Actuellement, pour un fichier h contenant deux classes simples dans XObject, XHT générera près de cent lignes de code supplémentaire. Ce code doit suivre un format fixe, et il est très fatigant de l'écrire soi-même.
> - XHT est peu sujet aux erreurs ; il suffit de définir les règles de génération.
> - Lorsque la classe originale est modifiée, XHT synchronisera automatiquement les modifications du code supplémentaire, sans nécessiter de modifications manuelles.

## Mode de fonctionnement de XHT

- XObject.h : Définit des macros, ces marques de macros seront capturées par XHT

  ```C++
  #define XENTRY(...) \
  public: \
      static XMetaObject* staticMetaObject(); \
      virtual XMetaObject* metaObject() override; \
      virtual rttr::instance instance() override { return *this; } \
      using base_class_list = rttr::type_list<__VA_ARGS__>; \
  	virtual void __intrusive_deserialize(Deserializer& deserializer) override; \
      virtual void __intrusive_serialize(Serializer& serializer) override; \
      virtual void __intrusive_to_json(nlohmann::json& json) const override; \
  private:
  
  #define XFUNCTION(...)
  #define XPROPERTY(...)
  #define XENUM(...)
  
  ```

XHT est un projet de ligne de commande indépendant, et son format de paramètres est le suivant :

```
AxHeadeTool ${input_file} -o ${output_dir} 
Options:
-i include_dir: Chemin d'inclusion du fichier d'entrée
```

Cet outil peut analyser les fichiers, collecter les informations sur les symboles qu'ils contiennent et générer de nouveaux fichiers. Le processus de traitement est décrit dans main.cpp.

### Processus personnalisé

#### Définir les mots-clés

XHT analyse efficacement le contenu du fichier en chaînes de symboles à l'aide d'une table de recherche de mots-clés. Le contenu de la table se trouve dans le fichier `Keywords.inl`, qui est généré automatiquement par le projet `KeywordsGen`. Le projet définit tous les mots-clés utilisés dans XHT.

```C++
static const Keyword pp_keywords[] = {...}   // Mots-clés de prétraitement	
static const Keyword keywords[] = {
	{ "<", "LANGLE" },
	{ ">", "RANGLE" },
	...
	
	{ "XENTRY","XENTRY_TOKEN"},
	{ "XFUNCTION","XFUNCTION_TOKEN"},
	{ "XPROPERTY","XPROPERTY_TOKEN"},
	{ "XENUM","XENUM_TOKEN"},
	...
	};
```

Un mot-clé contient deux paramètres. Prenons `{ "XENTRY","XENTRY_TOKEN"}` comme exemple : `XENTRY` représente le symbole exact dans le fichier de code, et `XENTRY_TOKEN` correspond à l'énumération utilisée dans XHT. La définition de l'énumération se trouve dans le fichier `Token.h` de XHT :

```C++
#define FOR_ALL_TOKENS(F) \
    F(NOTOKEN) \
    F(IDENTIFIER) \
    ...
    F(XENTRY_TOKEN) \
    F(XFUNCTION_TOKEN) \
    F(XPROPERTY_TOKEN) \
    F(XENUM_TOKEN) \
    ...
```

Ainsi, pour définir un mot-clé, les étapes suivantes sont nécessaires :

- Ajouter une entrée de mot-clé `{ symbole, élément d'énumération }` dans le projet **KeywordsGen**
- Exécuter le projet **KeywordsGen** pour générer automatiquement `Keywords.inl` dans le répertoire de code de **XHT**
- Définir l'`élément d'énumération` dans le fichier `Token.h` du projet **XHT**

#### Écrire un collecteur

Il n'est pas nécessaire de se préoccuper du processus **FileParser** à moins que vous n'ayez besoin de traiter des opérations liées à include et aux macros. La seule chose à noter est la fonction `bool parse(FileDataDef&, const Symbols&)` de **SymbolParser**.

La structure hiérarchique principale du switch dans le parser est la suivante :

- Switch de niveau supérieur : Collecte les informations globales du fichier
  - namespace : Collecte les informations sur l'espace de noms
    - ...
  - class/struct : Collecte les informations sur la classe
    - ...
  - ...

En consultant le code, on peut voir que la classe contient `case AX_INVOKABLE_TOKEN` et `case AX_PROPERTY_TOKEN`.

Prenons `XPROPERTY_TOKEN` comme exemple : son rôle est de compléter le marquage de variable au format suivant :

```
XPROPERTY(GET getX SET setX)	// Où GET et SET spécifient les fonctions get et set de la variable 
int var;
```

En revenant à **SymbolParser**, on peut constater que ce case appelle en fait la fonction `parseAxProperty()` :

```c++
case XPROPERTY_TOKEN:
	def.propertyList.push_back(parseXProperty());
break;
```

La définition de la fonction est la suivante :

```c++
void SymbolParser::parseXProperty()
{
    PropertyDef axVarDef;				// Définit la structure de données de Property
    next(LPAREN);						// Déplace vers le symbole suivant et vérifie s'il s'agit d'une parenthèse ouvrante
    while (test(IDENTIFIER)) {	    	// Vérifie s'il s'agit d'un identifiant (non mot-clé)
        std::string type = lexem(); 	// Obtient la chaîne de symboles
        if (type == "GET") {			// Vérifie si le caractère obtenu est GET ou SET
            next();						// Déplace vers le symbole suivant
            axVarDef.getter = lexem();  // Enregistre le symbole
        }
        else if (type == "SET") {
            next();	
            axVarDef.setter = lexem();
        }
    }
    next(RPAREN);						// Déplace vers le symbole suivant et vérifie s'il s'agit d'une parenthèse fermante
    axVarDef.type = parseType();		// Analyse le type de variable
    next(IDENTIFIER);					// Déplace vers le symbole suivant et vérifie s'il s'agit d'un identifiant
    axVarDef.name = lexem();			// Obtient le nom de la variable
    until(SEMIC);						// Déplace vers l'arrière jusqu'à ce qu'un point-virgule soit trouvé
    return axVarDef;
}
```

Toutes les structures de données sont définies dans le fichier `DataDef.h`.

Ainsi, pour collecter des informations, les étapes suivantes sont généralement nécessaires :

- Définir les données à collecter dans `DataDef.h`
- Ajouter un case à l'emplacement approprié dans `SymbolParser::parser`
- Écrire une fonction d'analyse de symboles et compléter la collecte d'informations à l'aide des méthodes pratiques fournies par **Parser**

  - Fonctions de base

    ```c++
    class ParserBase{
        inline bool hasNext() const { return (mIndex < mSymbols.size()); }
        inline Token next() { if (mIndex >= mSymbols.size()) return NOTOKEN; return mSymbols.at(mIndex++).token; }
        inline Token peek() { if (mIndex >= mSymbols.size()) return NOTOKEN; return mSymbols.at(mIndex).token; }
        bool test(Token);
        void next(Token);
        void next(Token, const char* msg);
        inline void prev() { --mIndex; }
        inline Token lookup(int k = 1);
        inline const Symbol& symbol_lookup(int k = 1) { return mSymbols.at(mIndex - 1 + k); }
        inline Token token() { return mSymbols.at(mIndex - 1).token; }
        inline std::string lexem() { return mSymbols.at(mIndex - 1).lexem(); }
        inline std::string unquotedLexem() { return mSymbols.at(mIndex - 1).unquotedLexem(); }
        inline const Symbol& symbol() { return mSymbols.at(mIndex - 1); }

        void error(int rollback);
        void error(const char* msg = nullptr);
        void warning(const char* = nullptr);
        void note(const char* = nullptr);
    };
    ```

  - Fonctions utilitaires

    ```C++
    class SymbolParser : public ParserBase{
        Type parseType();
        bool parseEnum(EnumDef* def);
        bool parseFunction(FunctionDef* def, bool inMacro = false);
        bool parseMaybeFunction(const ClassDef* cdef, FunctionDef* def);
        void parseFunctionArguments(FunctionDef* def);
        std::string lexemUntil(Token);
        bool until(Token);
    ```

#### Générer du code supplémentaire

La génération de code supplémentaire se trouve dans **FileGenerator**.

En ouvrant `FileGenerator.cpp`, on constate que la génération de code supplémentaire consiste simplement à écrire des données dans un fichier via fprintf :

```C++
bool FileGenerator::generateSource()
{
	FILE* out;
	std::filesystem::path outputPath(fileData->outputPath);

	if (fopen_s(&out, outputPath.string().c_str(), "w") != 0) {
		return false;
	}
	std::string header_path = std::regex_replace(std::filesystem::relative(fileData->inputFilePath, outputPath.parent_path()).string(), std::regex("\\\\"), "/");
	fprintf(out, "#include \"%s\"\n", header_path.c_str());					// Génère le code include du cpp actuel par rapport au répertoire h
	fprintf(out, "#include <rttr/registration>\n");
	fprintf(out, "#include <XMetaObject.h>\n");
	fprintf(out, "#include <Serialization/SerializationBriefSyntax.h>\n");

	fputs("\n", out);														

	for (auto classdef : fileData->classList) {
		generateAxonClass(out, classdef);
	}
	fputs("", out);

	generateGlobalData(out);											
	fclose(out);
	return true;
}
```

Ainsi, à cette étape, il suffit d'écrire du code selon les informations collectées.

## Construction automatique

Avec **XHT**, il est possible de traiter les fichiers manuellement via la ligne de commande, mais comment faire pour que le projet appelle automatiquement **XHT** pour traiter les fichiers de code ?

Cela est implémenté dans le **CMakeLists.txt** de **XObject** :

```cmake
function(target_xht_warp PROJECT_TARGET INPUT_FILE_PATH)            
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE_PATH} NAME_WE)               # Obtient le nom de fichier sans extension
    set(OUTPUT_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/AutoGenFiles/XHT_${INPUT_FILE_NAME}.cpp)   
    add_custom_command(
        OUTPUT ${OUTPUT_FILE_PATH}                                                   # Spécifie le fichier de sortie
        COMMAND XHT ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE_PATH} -o ${OUTPUT_FILE_PATH}  # Commande de ligne de commande
        MAIN_DEPENDENCY ${INPUT_FILE_PATH}                           # Spécifie la dépendance ; lorsque ce fichier change, la commande est appelée automatiquement
    )          
    set_property(TARGET ${PROJECT_TARGET} APPEND PROPERTY SOURCES ${OUTPUT_FILE_PATH})       # Ajoute au target de construction
    source_group("Generated Files" FILES ${OUTPUT_FILE_PATH})                                # Groupe de fichiers
endfunction()
```

Cette fonction peut traiter un fichier unique, générer automatiquement du code supplémentaire et le faire participer à la construction du projet.

Est-il possible de détecter si les fichiers de code contiennent certains symboles et d'appeler automatiquement HeaderTool, comme dans UE ou Qt ? La réponse est oui. CMake propose la fonction suivante pour rechercher si des fichiers de code contiennent certains symboles :

[ **check_cxx_symbol_exists** ](https://cmake.org/cmake/help/latest/module/CheckCXXSymbolExists.html)

Cependant, ce processus est relativement inefficace en pratique, car il nécessite de parcourir l'ensemble du fichier. Une meilleure méthode consiste à identifier les fichiers à traiter par HeaderTool via l'extension du fichier. Le CMakeLists de XObject propose également la fonction suivante :

```cmake
function(target_xht_auto PROJECT_TARGET)
    get_target_property(TARGET_SOURCES ${PROJECT_TARGET} SOURCES)
    message(WARNING "FILES ${TARGET_SOURCES}")
    foreach(FILE ${TARGET_SOURCES})
        get_filename_component(FILE_EXT ${FILE} EXT)     
        if(FILE_EXT STREQUAL ".hxx")
            target_xht_warp(${PROJECT_TARGET} ${FILE})
        endif()
    endforeach()
endfunction()
```

#### Quelques problèmes

##### Héritage

Rttr utilise des modèles pour enregistrer les types de réflexion. Le traitement de l'héritage est réalisé via la métaprogrammation de modèles (lorsque la classe dérivée appelle une fonction de la classe de base, elle recherchera dans la classe de base selon `using base_class_list = rttr::type_list<...>` défini dans la classe dérivée). Par conséquent, cette définition doit être écrite dans le fichier d'en-tête pour garantir sa visibilité pour la classe dérivée. Cela nécessite de spécifier à nouveau manuellement la classe de base lors du marquage de la réflexion, comme ceci :

```C++
class Base : public XObject {
	XENTRY(XObject) 					// Il est nécessaire de spécifier la classe de base ici, sinon Rttr ne peut pas déterminer la relation d'héritage
}
```

##### Pointeur dégradé

Dans Rttr, il est impossible d'utiliser un pointeur de classe de base pour appeler une méthode de la classe dérivée. Par exemple, le code suivant :

```c++
XObject* x = new Base;					// Base contient la méthode print
rttr::invoke(x,"print");				// Échec de l'appel, car Rttr ne peut pas déterminer le type réel de x
```

XObject a ajouté la définition suivante dans la macro `XEntry()` :

```c++
virtual rttr::instance instance() override { return *this; } 
```

Cela fournit une fonction virtuelle aux classes dérivées de XObject pour obtenir une instance rttr, évitant ainsi le problème de pointeur mentionné ci-dessus. L'appel se présente comme ceci :

```C++
XObject* x = new Base;
rttr::invoke(x->instance(),"print");	// Appel réussi
```

##### Moment de l'enregistrement

La méthode officielle consiste à utiliser l'enregistrement statique, dont le principe peut être simplifié comme suit :

```c++
// Ceci se trouve dans le fichier CPP
struct Register{
	Register(){
		// L'enregistrement du type RTTR est effectué ici
	}
};
static Register register;
```

Explication : RTTR crée une structure dans le fichier CPP, écrit la fonction d'enregistrement dans le constructeur de la structure, puis crée une instance statique pour compléter l'enregistrement du type.

Cela présente principalement les problèmes suivants :

- L'ordre de l'enregistrement n'est pas facile à contrôler.
- L'enregistrement statique de tous les types de réflexion d'un coup peut entraîner des ralentissements au démarrage du programme.

Pour résoudre ce problème, XObject a ajouté la classe **XMetaObject**, dont la structure est similaire à `Register` ci-dessus (c'est-à-dire que le code d'enregistrement de Rttr se trouve dans son constructeur). La différence est que nous n'avons pas créé directement une instance statique de XMetaObject dans le fichier cpp, mais avons plutôt implémenté cela de la manière suivante :

```C++
class XObject{
	static XMetaObject* staticMetaObject(){
        static XMetaObject instance;
        return &instance;
    }
}
```

Lors de la première accès à la fonction `staticMetaObject()`, l'instance statique à l'intérieur est créée. En plus, XObject utilise XMetaObject comme seule entrée pour les données de réflexion, donc l'enregistrement Rttr est effectué automatiquement lors de l'utilisation de l'interface de réflexion du type.

Il y a également un détail ici :

Lorsque l'interface de réflexion de la classe dérivée appelle l'interface de la classe de base, il faut garantir que la classe de base a déjà été enregistrée.

Pour cela, dans XObject, le constructeur de MetaObject de la classe dérivée appelle une fois `staticMetaObject` de la classe de base pour garantir que la classe de base est enregistrée à l'avance.

> ### Objet méta
>
> Nous avons mentionné XMetaObject ci-dessus. Expliquons brièvement comment XObject l'utilise comme seule entrée de réflexion pour le type.
>
> L'utilisation de la réflexion vise principalement les objectifs suivants :
>
> - Lire et écrire Property, obtenir des informations sur Property (type, métadonnées...).
> - Appeler Function, obtenir des informations sur Function (informations sur les paramètres...).
> - Créer une instance selon le nom du type.
>
> XMetaObject est lié au type et indépendant de l'instance. Il est utilisé simplement pour obtenir des données de réflexion. Une partie de son code est la suivante :
>
> ```C++
> struct XMetaObject {
> public:
> 	XMetaObject();
> 	XObject* newInstance(std::vector<rttr::argument> args = {});	// Crée une instance
> 	rttr::property getProperty(std::string name);					// Obtient la property nommée name
> 	rttr::array_range<rttr::property> getProperties();				// Obtient toutes les properties du type
> 	rttr::method getMethod(std::string name);						// Obtient la méthode nommée name
> 	rttr::array_range<rttr::method> getMethods();					// Obtient toutes les méthodes du type
> };
> ```
>
> L'appel de fonction est lié à l'instance, et pour appeler une fonction, il faut fournir une instance. L'opération se présente comme ceci :
>
> ```c++
> Base* base = new Base;
> XMetaObject* meta = Base::staticMetaObject();			// Obtient l'objet méta statique de Base
> rttr::method method = meta->getMethod("print");			// Obtient les informations sur la fonction print de Base
> method.invoke(base);  									// Un instance est nécessaire lors de l'appel
> ```
>
> Comme on le sait, un pointeur de type affaibli ne peut pas appeler une fonction statique du type réel, c'est-à-dire :
>
> ```c++
> XObejct *x = new Base;
> x->staticMetaObject();		// Cela appellera XObject::staticMetaObject() plutôt que Base::staticMetaObject()
> ```
>
> Pour résoudre ce problème, la macro `XEntry()` a ajouté la définition suivante :
>
> ```C++
> virtual XMetaObject* metaObject() override { return staticMetaObject(); }
> ```
>
> De plus, la recherche dans le pool de types selon le nom du type dans Rttr est coûteuse. Pour résoudre ce problème, XMetaObject propose une fonction virtuelle supplémentaire pour lier MetaObject à rttr::type :
>
> ```c++
> virtual rttr::type getRttrType() { return rttr::type::get<void>(); };	
> ```
>
> Lorsque XHT génère la fonction d'enregistrement de Rttr, il génère également la définition de cette fonction, qui retourne un rttr::type déjà déterminé.
>
> Selon l'interface de l'objet méta, XObject propose également certaines interfaces de réflexion liées à l'instance :
>
> ```C++
> bool setProperty(std::string name, rttr::argument var);		// Définit la propriété
> rttr::variant getProperty(std::string name);				// Obtient la propriété
> rttr::variant invoke(rttr::string_view name, std::vector<rttr::argument> args = {});		// Appelle la fonction
> ```

### Sérialisation

Si vous écrivez manuellement les fonctions de sérialisation, la sérialisation n'est pas difficile, mais si vous voulez une sérialisation automatique, vous devrez faire face aux problèmes suivants :

- Sérialisation de types personnalisés.
- Sérialisation de types complexes (conteneurs).
- Désérialisation de pointeurs.
- Vérification des erreurs d'exécution lors de la désérialisation.

Il existe différentes méthodes de traitement selon les objectifs de sérialisation :

- La sérialisation binaire offre les meilleures performances et la meilleure utilisation de la mémoire, mais son contenu est difficile à lire.
- La sérialisation non binaire (telle que xml, json, cbor, etc.) a une structure de stockage élégante et une grande lisibilité, mais elle entraîne une perte de performances et de mémoire relativement élevée.

#### Binary

La bibliothèque utilisée pour la sérialisation binaire est [BitSery](https://github.com/fraillt/bitsery), qui prend en charge la sérialisation de types complexes (conteneurs) via des opérations de modèle.

Pour qu'un type prenne en charge la sérialisation BitSery, il suffit de fournir la fonction de modèle `serialize(Serialize& s, type& o)` :

```C++
struct MyStruct {
    uint32_t i;
    std::vector<float> fs;
};

template <typename Serialize>
void serialize(Serialize& s, MyStruct& o) {	 // Cette fonction de modèle peut être utilisée à la fois comme fonction de sérialisation et de désérialisation
    s(o.i);
    s(o.fs);
}
```

Pour sérialiser cet objet, il suffit de :

```C++
using SerializeBuffer = std::vector<uint8_t>;		// Définit le type de Buffer
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;
using Serializer = bitsery::Serializer<OutputAdapter>;
using Deserializer = bitsery::Deserializer<InputAdapter>;

void main(){
    MyStruct myStruct;
    SerializeBuffer buffer;
    
    // Sérialisation : écrit myStruct dans buffer
    bitsery::quickSerialization(OutputAdapter(buffer), myStruct); 
    
    // Désérialisation : lit les données depuis buffer et les écrit dans myStruct
    bitsery::quickDeserialization(InputAdapter(buffer), myStruct);   	
}
```

##### Contourner le modèle

Dans XObject, pour éviter les modèles, les fonctions de sérialisation et de désérialisation sont séparées, et les définitions suivantes sont ajoutées dans la macro **XENTRY()** :

```c++
virtual void __intrusive_deserialize(Deserializer& deserializer) override; 
virtual void __intrusive_serialize(Serializer& serializer) override; 
```

L'implémentation de ces fonctions est générée par XHT selon Property. De plus, la fonction de sérialisation de la classe de base est appelée ici.

> Ici, l'API Bitsery est transférée via un modèle. Le problème est que le modèle ne peut pas déterminer la relation d'héritage des types.
>
> Heureusement, des définitions supplémentaires ont été ajoutées lors de la mise en œuvre de l'héritage Rttr, et ces définitions peuvent être utilisées pour déterminer facilement si un type est une classe dérivée de XObject :
>
> ```C++
> template<typename T, typename std::enable_if< std::is_class<T>::value&& rttr::detail::has_base_class_list<T>::value>::type* = nullptr>
> void serialize(Serializer& serializer, T& ptr) {
> 	ptr.__intrusive_serialize(serializer);
> }
> 
> template<typename T, typename std::enable_if<std::is_class<T>::value&& rttr::detail::has_base_class_list<T>::value>::type* = nullptr>
> void serialize(Deserializer& deserializer, T& ptr) {
> 	ptr.__intrusive_deserialize(deserializer);
> }
> ```

##### Spécialisation du pointeur

Étant donné que Bitsery ne prend pas en charge la sérialisation des pointeurs, il faut écrire专门ement des modèles de sérialisation pour les pointeurs.

Le type pointeur et le type original diffèrent sur deux points :

- Le pointeur peut être nul, et nullptr ne peut pas être sérialisé.
- Lors de la désérialisation du pointeur, il peut être nécessaire de créer une nouvelle instance.

Pour résoudre ces deux problèmes, les solutions suivantes sont adoptées :

- Lors de la sérialisation, le type du pointeur est écrit ; si le pointeur est nul, un type nul est écrit. Lors de la désérialisation, le type est toujours lu en premier ; si le type n'est pas nul, la sérialisation continue.
- Étant donné que le type du pointeur est écrit lors de la sérialisation, une nouvelle instance peut être créée via RTTR selon le nom du type, mais il faut garantir que le type a enregistré un constructeur sans paramètres.

Mais pour XObject, il faut faire plus : pour x dans `XObject* x = new Base();`, il n'est pas possible d'utiliser simplement `std::remove_pointer_t<T>::type` pour obtenir le type original. Par conséquent, pour le type XObject, il faut obtenir le type depuis l'objet metaObject de l'instance. Le code principal est le suivant :

```c++
template<typename T, typename std::enable_if<std::is_pointer<T>::value && !rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = rttr::type::get<rttr::detail::raw_type_t<T>::type>();		// Obtient le type original selon le type du pointeur
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					// Écrit le type
	if (ptr != nullptr) {
		writer(*ptr);					// Si le pointeur n'est pas nul, écrit les données pointées par le pointeur
	}
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value&& rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = ptr->metaObject()->getRttrType();							// Obtient le type depuis metaObject
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					// Écrit le type
	if (ptr != nullptr) {
		writer(*ptr);					// Si le pointeur n'est pas nul, écrit les données pointées par le pointeur
	}	
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
void serialize(Deserializer& reader, T& ptr) {
	std::string typeName;				
	reader(typeName);					// Lit le type
	if (typeName.empty())				// Si le type est nul, retourne directement
		return;
	if (ptr != nullptr) {				// Si l'objet pointé lors de la désérialisation n'est pas nul, désérialise directement
		reader(*ptr);
		return;
	}
	rttr::type type = rttr::type::get_by_name(typeName);		// Obtient le type Rttr
	if (!type.is_valid())
		return;
	rttr::variant var = type.create();							// Crée une instance via Rttr selon le nom du type
	if (!var.can_convert<T>()) {								
		return;
	}
	ptr = var.get_value<T>();									// Si l'instance est valide, désérialise
	reader(*ptr);
}
```

##### Optimisation de Include

Bitsery est une bibliothèque **Header-Only**. Pour ne pas inclure tout le contenu dans **XObject.h**, la solution consiste à écrire les définitions principales dans **SerializationDefine.h**, et XObject n'inclut que ce fichier ; pour le reste du code, il est inclus dans **SerializationBriefSyntax.h**, et XHT inclut ce fichier lors de la génération de code supplémentaire.

#### Json et Cbor

La bibliothèque utilisée ici est [nlohmann json](https://github.com/nlohmann/json.git). L'implémentation est un peu sommaire : seule la sérialisation est réalisée, pas la désérialisation, et elle ne peut être utilisée que pour le débogage. La fonction de sérialisation est également générée par XHT. Naturellement, une nouvelle définition est ajoutée dans **XENTRY()** :

```c++
virtual void __intrusive_to_json(nlohmann::json& json) const override;
```

Son mode de traitement est similaire à la sérialisation binaire.

Au départ, la sérialisation json était censée être réalisée via la réflexion, car j'avais déjà réalisé la sérialisation json via la réflexion dans Qt. Mais quand j'ai réellement commencé à l'implémenter, j'ai découvert que ce n'était pas aussi simple :

Via rttr, on peut obtenir une donnée rttr::variant. Comment faire pour la convertir automatiquement en données json ? Il semble que je ne peux faire que ceci :

```C++
rttr::variant property;
if(var.isType<int>()){
	json[propertyName] = property.get_value<int>();
}
else if(var.isType<double>()){
	json[propertyName] = property.get_value<double>();
}
else if(...){
	...
}
```

> Une solution détournée... ?

J'avais toujours pensé que l'E/S sur QVariant dans Qt se transférait automatiquement vers la fonction de sérialisation du type correspondant.

Cependant, rttr::variant ne prend pas en charge cela, donc j'ai consulté la méthode de QVariant pour combler cette lacune.

J'ai découvert que sa méthode est en fait la même, mais elle utilise une Map pour stocker les fonctions :

```c++
rttr::variant property;
json[propertyName] = FuncMap[property.typeId()](property);
```

Pour les types personnalisés, Qt déclare qu'il faut les modifier avec la macro **Q_DECLARE_METATYPE** :

```C++
struct CustomType{};
Q_DECLARE_METATYPE(CustomType)  // Cette macro enregistre le meta type ; si le type a une fonction de sérialisation, le pointeur de fonction est lié à l'ID du type.
```

L'enregistrement RTTR ne prend pas en charge cette opération, donc pour l'instant, la fonction de sérialisation json est générée par XHT.

### Éditeur

Le module Core ne dépend pas du tout du module Editor, tandis que le module Editor utilise les données de réflexion du Core pour créer l'éditeur.

J'ai écrit beaucoup d'éditeurs et fait beaucoup d'essais, passant principalement par les étapes suivantes :

- **L'éditeur est au cœur**

  - L'objectif initial du développement était simple : il fallait un éditeur de telle sorte pour obtenir un tel effet, donc j'ai directement commencé à concevoir l'UI et la logique de l'éditeur. L'éditeur appelait directement les méthodes de la classe d'effet pour éditer l'effet, et parfois il fallait même demander à la classe d'effet de fournir des API spécifiques à l'éditeur. À la fin, la classe d'effet originale était complètement déformée, le code de l'éditeur et de la classe d'effet était dispersé partout, le couplage était très grave, et il fallait être extrêmement prudent lors de l'écriture des interfaces d'importation et d'exportation.

- **Éditeur de type centré sur Property** : Il y a eu beaucoup d'étapes de transition avant cela, mais elles n'ont pas de sens à mentionner. Vous pouvez en savoir plus par les caractéristiques suivantes :

  - Property est l'objectif de l'édition et dispose d'interfaces de lecture/écriture logiquement cohérentes

    > Lors de l'écriture de la classe d'effet, il faut réserver des Property éditables et fournir des interfaces de lecture/écriture correspondantes. Lors de l'utilisation de ces Property, il faut garantir leur synchronisation pour pouvoir utiliser correctement les données générées par la sérialisation de l'éditeur.

  - Les composants de l'éditeur correspondent aux types de données un à un

    > C'est-à-dire :
    >
    > Le type int a un composant d'édition int, la classe de courbe a un composant d'édition de courbe, la structure arborescente a un composant d'édition d'arborescence...
    >
    > Ces composants sont très fins, et l'éditeur complet est assemblé à partir de ces petits composants.
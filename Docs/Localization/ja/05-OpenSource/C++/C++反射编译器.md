---
comments: true
---

# C++ リフレクションコンパイラ

- Githubリポジトリ：https://github.com/Italink/XObject

## ビルド

- CMake（GUI）を使用して直接ビルド可能

## プロジェクト構造の概要

- Core：コアモジュール
  - 3rdParty：サードパーティライブラリ
  - XHT：コードスキャンツール
  - Test：モジュール機能のテストプロジェクトディレクトリ
  - XObject：基底クラスのカプセル化

## ロジックの概要

### 基底クラスXObject

xobjectはすべてのオブジェクトの基底クラスとして、シリアライズ、リフレクションなどの操作を提供する。

### リフレクション

リフレクションに使用されるライブラリはRttrである。Rttrは優れたC++リフレクションライブラリで、単純な型情報のリフレクションだけでなく、一連の完全なリフレクション処理インターフェースを提供する：

- rttr::variant：stl::anyに似た使い方だが、より強力で多くの型情報と変換インターフェースを提供する（例：int型のvariantは直接doubleにエクスポート可能。ここではintからdoubleへの変換インターフェースが呼び出され、rttrではvariantの型変換関数をカスタマイズ可能）
- 型名に基づいて対応するインスタンスを作成することをサポートし、3つの作成モード（Type、Type\*、std::shared_ptr\<Type\*>）を提供する
- metadataは情報を追加するために使用され、この機能を利用してPropertyに多くの追加情報を付加可能

XHTはコードスキャンに使用される（QtMocを参考）。マクロマークによって型の自動登録を実現し、RTTRが登録関数を手動で記述する必要がある問題を解決する。

> XHTは不要なように見えるかもしれない。自分で登録関数を書くことも当然可能だが、XHTを使用するためのより良い理由が存在する：
>
> - Rttrだけが登録関数を記述する必要があるわけではない。シリアライズなど型に追加機能を提供する場合も登録が必要となる。現在のXObjectでは、2つの単純なクラスを含むhファイルに対し、XHTは約百行の追加コードを生成する。この部分のコードは固定形式が必要で、自分で書くと非常に手間がかかる。
> - XHTはエラーが発生しにくい。生成ルールを定義するだけで良い。
> - 元のクラスが変更された場合、XHTは追加コードの変更を自動的に同期し、手動での変更は不要。

## XHTの動作方式

- XObject.h：マクロを定義する。これらのマクロマークはXHTによってキャプチャされる。

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

XHTは独立したコマンドラインプロジェクトで、そのパラメータ形式は以下の通り：

```
AxHeadeTool ${input_file} -o ${output_dir} 
Options:
-i include_dir:入力ファイルのインクルードパス
```

このツールはファイルをスキャンし、其中のシンボル情報を収集して新しいファイルを生成する。処理プロセスはmain.cppを参照。

### カスタマイズプロセス

#### キーワードの定義

XHTはキーワード検索テーブルを使用してファイル内容を効率的にシンボル文字列に解析する。其中のテーブル内容はファイル`Keywords.inl`に存在し、このテーブルはプロジェクト`KeywordsGen`によって自動生成される。プロジェクト内でXHTで使用されるすべてのキーワードが定義される。

```C++
static const Keyword pp_keywords[] = {...}   //プリプロセスのキーワード	
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

キーワードは2つのパラメータを含む。`{ "XENTRY","XENTRY_TOKEN"}`を例にすると、`XENTRY`はコードファイル中の正確なシンボルを表し、`XENTRY_TOKEN`はXHTで使用される列挙型に対応する。列挙型の定義はXHTの`Token.h`ファイルに存在する：

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

したがって、キーワードを定義するには以下の手順が必要：

- **KeywordsGenプロジェクト** で新しいキーワードエントリ`｛シンボル、列挙要素｝`を追加
- **KeywordsGenプロジェクト** を実行すると、**XHT** のコードディレクトリ下に`Keywords.inl`が自動生成される
- **XHTプロジェクト** の`Token.h`ファイルで`列挙要素`を定義

#### コレクタの記述

**FileParser** プロセスは特にincludeやマクロ関連の操作を処理する必要がある場合を除き、関心を払う必要はない。唯一注意が必要なのは **SymbolParser** の`bool parse(FileDataDef&, const Symbols&)`関数である。

parser内のswitchの主な階層構造は以下の通り：

- トップレベルswitch：ファイルのグローバル情報を収集
  - namespace：名前空間の情報を収集
    - ...
  - class/struct：クラスの情報を収集
    - ...
  - ...

コードを参照すると、class内に ` case AX_INVOKABLE_TOKEN`と`case AX_PROPERTY_TOKEN`が含まれていることがわかる。

`XPROPERTY_TOKEN`を例にすると、その役割は以下の形式の変数マークを完成させる：

```
XPROPERTY(GET getX SET setX)	//其中 GET 和 SET 指定了该变量的get函数和set函数 
int var;
```

**SymbolParser** に戻ると、このcaseが実際に`parseAxProperty()`関数を呼び出していることがわかる：

```c++
case XPROPERTY_TOKEN:
	def.propertyList.push_back(parseXProperty());
break;
```

関数の定義は以下の通り：

```c++
void SymbolParser::parseXProperty()
{
    PropertyDef axVarDef;				//Propertyのデータ構造を定義
    next(LPAREN);						//次のシンボルに移動し、左括弧かどうかを判断
    while (test(IDENTIFIER)) {	    	//識別子（キーワードではない）かどうかを判断
        std::string type = lexem(); 	//シンボル文字列の取得
        if (type == "GET") {			//取得した文字列がGETかSETかを判断
            next();						//次のシンボルに移動
            axVarDef.getter = lexem();  //シンボルを保存
        }
        else if (type == "SET") {
            next();	
            axVarDef.setter = lexem();
        }
    }
    next(RPAREN);						//次のシンボルに移動し、右括弧かどうかを判断
    axVarDef.type = parseType();		//変数の型を解析
    next(IDENTIFIER);					//次のシンボルに移動し、識別子かどうかを判断
    axVarDef.name = lexem();			//変数名を取得
    until(SEMIC);						//セミコロンに一致するまで後方に移動
    return axVarDef;
}
```

すべてのデータ構造は`DataDef.h`ファイルに定義されている。

したがって、情報の収集を完成させるには、一般的に以下の手順が必要：

- `DataDef.h`で収集したいデータを定義
- `SymbolParser::parser`内の適切な場所にcaseを追加
- シンボル解析関数を記述し、**Parser** が提供する便利なメソッドを利用して情報の収集を完成させる

  - 基礎関数

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

  - ツール関数

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

#### 追加コードの生成

追加コードの生成は **FileGenerator** 内に位置する。

`FileGenerator.cpp`を開くと、追加コードの生成が単純にfprintfによってファイルにデータを書き込むことで実現されていることがわかる：

```C++
bool FileGenerator::generateSource()
{
	FILE* out;
	std::filesystem::path outputPath(fileData->outputPath);

	if (fopen_s(&out, outputPath.string().c_str(), "w") != 0) {
		return false;
	}
	std::string header_path = std::regex_replace(std::filesystem::relative(fileData->inputFilePath, outputPath.parent_path()).string(), std::regex("\\\\"), "/");
	fprintf(out, "#include \"%s\"\n", header_path.c_str());					//現在のcppのhディレクトリに対する相対パスのincludeコードを生成
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

したがって、この段階では収集した情報に基づいてコードを書き込むだけで良い。



## 自動ビルド

**XHT** があれば、手動でコマンドラインを通じてファイルを処理することができる。しかし、プロジェクト内で自動的に **XHT** を呼び出してコードファイルを処理したい場合はどうすれば良いだろうか？

**XObject** の **CMakeLists.txt** でこれが実現されている：

```cmake
function(target_xht_warp PROJECT_TARGET INPUT_FILE_PATH)            
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE_PATH} NAME_WE)               #拡張子なしのファイル名を取得
    set(OUTPUT_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/AutoGenFiles/XHT_${INPUT_FILE_NAME}.cpp)   
    add_custom_command(
        OUTPUT ${OUTPUT_FILE_PATH}                                                   #出力ファイルを指定
        COMMAND XHT ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE_PATH} -o ${OUTPUT_FILE_PATH}  #コマンドライン指令
        MAIN_DEPENDENCY ${INPUT_FILE_PATH}                           #依存関係を指定。このファイルが変更された場合、自動的にこの指令を呼び出す
    )          
    set_property(TARGET ${PROJECT_TARGET} APPEND PROPERTY SOURCES ${OUTPUT_FILE_PATH})       #ビルドターゲットに追加
    source_group("Generated Files" FILES ${OUTPUT_FILE_PATH})                                #ファイルグループ化
endfunction()
```

この関数は単一ファイルを処理し、追加コードを自動生成してプロジェクトのビルドに参加させることができる。

UEやQtのように、コード中に特定のシンボルが含まれているかどうかを検出して自動的にHeaderToolを呼び出すことは可能だろうか？答案は肯定的である。cmakeは以下の関数を提供し、コードファイル中に特定のシンボルが含まれているかどうかを検索するために使用できる：

[ **check_cxx_symbol_exists** ](https://cmake.org/cmake/help/latest/module/CheckCXXSymbolExists.html)

しかし、実際にはこのプロセスは比較的非効率的で、ファイル全体を走査する必要がある。より良い方法はファイルの拡張子によってHeaderToolで処理する必要のあるファイルを識別することである。XObjectのCMakeListsは以下の関数も提供している：

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

#### いくつかの問題

##### 継承

Rttrはテンプレートを使用してリフレクション型を登録する。継承の処理はテンプレートメタプログラミングによって実現される（サブクラスが親クラスの関数を呼び出す場合、サブクラスで定義された`using base_class_list = rttr::type_list<...>`に基づいて親クラス中で検索される）。したがって、この定義をヘッダファイルに記述する必要があり、これによってサブクラスからこの定義が可視になる。このため、リフレクションマークを行う際に、親クラスを再度手動で指定する必要がある。例えば以下のように：

```C++
class Base : public XObject {
	XENTRY(XObject) 					//ここで親クラスを指定しなければならない。そうしないとRttrで継承関係を確定できない
}
```

##### ポインタの退化

Rttrでは、親クラスのポインタを使用してサブクラスのメソッドを呼び出すことができない。例えば以下のコード：

```c++
XObject* x = new Base;					//Baseにprintメソッドが含まれている
rttr::invoke(x,"print");				//呼び出し失敗。原因はRttrがxの実際の型を判断できないことにある
```

XObjectはマクロ`XEntry()`中に以下の定義を追加している：

```c++
virtual rttr::instance instance() override { return *this; } 
```

これによりXObjectのサブクラスにrttrのインスタンスを取得するための仮想関数が提供され、上記のポインタの問題が回避される。呼び出しは以下のようになる：

```C++
XObject* x = new Base;
rttr::invoke(x->instance(),"print");	//呼び出し成功
```

##### 登録タイミング

公式が提供する方法は静的登録であり、その原理は以下のように簡略化できる：

```c++
//これはCPP中に存在する
struct Register{
	Register(){
		//ここでRTTRの型登録を行う
	}
};
static Register register;
```

説明：RTTRはCPP中に構造体を作成し、登録関数を構造体のコンストラクタに記述し、次に静的インスタンスを作成することで、型の登録を完成させる。

この方法には主に以下の問題が存在する：

- 登録の順序を制御することが容易ではない。
- 一度にすべてのリフレクション型を登録すると、プログラム起動時にカクツキが発生しやすい。

XObjectではこの問題を解決するために **XMetaObject** クラスを追加している。その構造は上記の`Register`と類似している（すなわちRttrの登録コードはそのコンストラクタ中に存在する）。異なる点は、直接cpp中にXMetaObjectの静的インスタンスを作成していないことである。代わりに以下の方式で実現している：

```C++
class XObject{
	static XMetaObject* staticMetaObject(){
        static XMetaObject instance;
        return &instance;
    }
}
```

`staticMetaObject()`関数に初めてアクセスした場合に、内部の静的インスタンスが作成される。さらにXObjectはXMetaObjectをリフレクションデータの唯一のエントリとして使用しているため、この型のリフレクションインターフェースを使用する際に、自動的にRttrの登録が行われる。

ここにはさらなる詳細が存在する：

サブクラスのリフレクションインターフェースを使用して親クラスのインターフェースを呼び出す場合、親クラスが既に登録されていることを保証する必要がある。

XObjectではこの操作を完成させるために、サブクラスMetaObjectのコンストラクタ中で、親クラスの`staticMetaObject`を一度呼び出し、親クラスが事前に登録されることを保証している。

> ### メタオブジェクト
>
> 上記でXMetaObjectについて言及した。ここではXObjectがどのようにそれを型の唯一のリフレクションエントリとして使用するかについて簡単に説明する。
>
> リフレクションを使用する主な目的は以下の通り：
>
> - Propertyの読み書き、Property情報（型、メタデータ...）の取得
> - Functionの呼び出し、Function情報（パラメータ情報...）の取得
> - 型名に基づいてインスタンスを作成
>
> XMetaObjectは型に関連し、インスタンスには関連しない。XMetaObjectを通じてリフレクションデータを取得するだけである。その一部のコードは以下の通り：
>
> ```C++
> struct XMetaObject {
> public:
> 	XMetaObject();
> 	XObject* newInstance(std::vector<rttr::argument> args = {});	//インスタンスを作成
> 	rttr::property getProperty(std::string name);					//nameという名前のpropertyを取得
> 	rttr::array_range<rttr::property> getProperties();				//この型のすべてのpropertyを取得
> 	rttr::method getMethod(std::string name);						//nameという名前のmethodを取得
> 	rttr::array_range<rttr::method> getMethods();					//この型のすべてのmethodを取得
> };
> ```
>
> 関数の呼び出しはインスタンスに関連する。関数を呼び出すにはインスタンスを提供する必要がある。この操作は以下のようになる：
>
> ```c++
> Base* base = new Base;
> XMetaObject* meta = Base::staticMetaObject();			//Baseの静的メタオブジェクトを取得
> rttr::method method = meta->getMethod("print");			//Baseのprint関数の情報を取得
> method.invoke(base);  									//呼び出し時にインスタンスを提供する必要がある
> ```
>
> 周知のように、弱化されたポインタ型は実際の型の静的関数を呼び出すことができない。即ち：
>
> ```c++
> XObejct *x = new Base;
> x->staticMetaObject();		//この時XObject::staticMetaObject()が呼び出され、Base::staticMetaObject()ではない
> ```
>
> この問題を解決するために、マクロ`XEntry()`中に以下の定義が追加されている：
>
> ```C++
> virtual XMetaObject* metaObject() override { return staticMetaObject(); }
> ```
>
> また、Rttrでは型名に基づいて型プール中で検索するコストが高い。この問題を解決するために、XMetaObjectはMetaObjectからrttr::typeへのバインディングを行うための追加の仮想関数を提供している：
>
> ```c++
> virtual rttr::type getRttrType() { return rttr::type::get<void>(); };	
> ```
>
> XHTがRttrの登録関数を生成する際に、この関数の定義も生成する。この関数は確定したrttr::typeを返す。
>
> メタオブジェクトのインターフェースに基づいて、XObjectはインスタンスに関連するいくつかのリフレクションインターフェースも提供している：
>
> ```C++
> bool setProperty(std::string name, rttr::argument var);		//プロパティを設定
> rttr::variant getProperty(std::string name);				//プロパティを取得
> rttr::variant invoke(rttr::string_view name, std::vector<rttr::argument> args = {});		//関数を呼び出す
> ```

### シリアライズ

シリアライズ関数を手動で記述する場合、シリアライズは困難ではない。しかし自動シリアライズを実現したい場合、以下の難題に直面する：

- カスタム型のシリアライズ
- 複雑型（コンテナ）のシリアライズ
- ポインタのデシリアライズ
- デシリアライズのランタイムエラーチェック

異なるシリアライズ目的には異なる処理方式が存在する：

- バイナリシリアライズは最高の性能とメモリ効率を持つが、その内容は読みにくい。
- 非バイナリシリアライズ（xml、json、cborなど）はエレガントなストレージ構造を持ち、可読性が非常に高いが、性能とメモリの損耗は相対的に高い。

#### Binary

バイナリシリアライズに使用されるライブラリは[BitSery](https://github.com/fraillt/bitsery)である。いくつかのテンプレート操作によって複雑型（コンテナ）のシリアライズをサポートしている。

ある型がBitSeryシリアライズをサポートするためには、テンプレート関数`serialize(Serialize& s, type& o)`を提供するだけで良い：

```C++
struct MyStruct {
    uint32_t i;
    std::vector<float> fs;
};

template <typename Serialize>
void serialize(Serialize& s, MyStruct& o) {	 //このテンプレート関数はシリアライズ関数としても、デシリアライズ関数としても使用可能
    s(o.i);
    s(o.fs);
}
```

このオブジェクトをシリアライズするには以下のように：

```C++
using SerializeBuffer = std::vector<uint8_t>;		//Bufferの型を定義
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;
using Serializer = bitsery::Serializer<OutputAdapter>;
using Deserializer = bitsery::Deserializer<InputAdapter>;

void main(){
    MyStruct myStruct;
    SerializeBuffer buffer;
    
    //シリアライズ。myStructをbufferに出力する
    bitsery::quickSerialization(OutputAdapter(buffer), myStruct); 
    
    //デシリアライズ。bufferからデータを読み取ってmyStructに入力する
    bitsery::quickDeserialization(InputAdapter(buffer), myStruct);   	
}
```

##### テンプレートの回避

XObjectではテンプレートを回避するために、シリアライズとデシリアライズ関数を分割し、マクロ **XENTRY()** 中に以下の定義を追加している：

```c++
virtual void __intrusive_deserialize(Deserializer& deserializer) override; 
virtual void __intrusive_serialize(Serializer& serializer) override; 
```

その関数の実装はXHTによってPropertyに基づいて生成される。此外、ここでは親クラスのシリアライズ関数も呼び出される。

> ここではテンプレートを使用してBitsery APIを転送している。存在する問題は、テンプレートによって型の継承関係を判断できないことである。
>
> 幸いなことに、Rttrの継承を実現する際にいくつかの定義が追加されている。これらの定義を利用すると、ある型がXObjectのサブクラスであるかどうかを容易に判断できる：
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

##### ポインタの特殊化

Bitseryはポインタのシリアライズをサポートしていないため、ポインタのシリアライズテンプレートを専門的に記述する必要がある。

ポインタ型と原型には2つの異なる点が存在する：

- ポインタは空になる可能性があり、nullptrはシリアライズできない
- ポインタのデシリアライズ時に、新しいインスタンスを作成する必要がある場合がある

これら2つの問題を解決するために、以下の解決方式が採用されている：

- シリアライズ時にポインタの型を書き込む。空ポインタの場合は空の型を書き込む。デシリアライズ時には必ず先に型を読み取る。型が空でない場合はシリアライズを続行する。
- シリアライズ時にポインタの型を書き込んでいるため、RTTRを利用して型名に基づいて新しいインスタンスを作成できる。但し、この型が無参コンストラクタを登録していることを保証する必要がある。

しかし、XObjectについてはさらに多くのことを行う必要がある。`XObject* x = new Base();`中のxについては、単純に`std::remove_pointer_t<T>::type`を使用して原型を取得することができない。したがって、XObjectの型については、インスタンスのmetaObjectオブジェクトから取得する必要がある。その核心コードは以下の通り：

```c++
template<typename T, typename std::enable_if<std::is_pointer<T>::value && !rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = rttr::type::get<rttr::detail::raw_type_t<T>::type>();		//ポインタ型に基づいて原始型を取得
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//型を書き込む
	if (ptr != nullptr) {
		writer(*ptr);					//ポインタが空でない場合、このポインタが指すデータを書き込む
	}
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value&& rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = ptr->metaObject()->getRttrType();							//metaObjectから型を取得
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//型を書き込む
	if (ptr != nullptr) {
		writer(*ptr);					//ポインタが空でない場合、このポインタが指すデータを書き込む
	}	
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
void serialize(Deserializer& reader, T& ptr) {
	std::string typeName;				
	reader(typeName);					//型を読み取る
	if (typeName.empty())				//型が空の場合、直接返す
		return;
	if (ptr != nullptr) {				//デシリアライズが指すオブジェクトが既に空でない場合、直接それに対してデシリアライズを行う
		reader(*ptr);
		return;
	}
	rttr::type type = rttr::type::get_by_name(typeName);		//Rttr型を取得
	if (!type.is_valid())
		return;
	rttr::variant var = type.create();							//Rttrを利用してインスタンスを作成
	if (!var.can_convert<T>()) {								
		return;
	}
	ptr = var.get_value<T>();									//インスタンスが合法的な場合、それに対してデシリアライズを行う
	reader(*ptr);
}
```

##### Includeの最適化

Bitseryは**Header - Only**ライブラリである。すべての内容を**XObject.h**に導入しないため、ここでは核心的な定義を**SerializationDefine.h**に書き込み、XObjectはこのファイルだけを含むようにしている。その他のコードは**SerializationBriefSyntax.h**中に導入され、XHTが追加コードを生成する際にこのファイルを含むようになる。



#### Json And Cbor

ここで使用されるライブラリは [nlohmann json](https://github.com/nlohmann/json.git)である。ここでの実装は少し草率で、デシリアライズは完成していないため、デバッグにのみ使用可能である。シリアライズ関数もXHTによって生成される。当然のことながら、**XENTRY()** 中に新しい定義が追加されている：

```c++
virtual void __intrusive_to_json(nlohmann::json& json) const override;
```

その処理方式もバイナリシリアライズと類似している。

jsonシリアライズは当初リフレクションによって実現しようと考えていた。以前Qtでもリフレクションによってjsonシリアライズを行っていたためである。しかし、実際に実装しようとしたときに、それが想像以上に簡単ではないことに気づいた。

rttrを通じてrttr::variantデータを読み取ることができる。どのようにしてそれを自動的にjson中のデータに変換することができるだろうか？以下のようにするしかないようだ：

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

> 曲線...救国...？

以前はQtでQVariantに対してIOを行うと、自動的に対応する型のシリアライズ関数に転送されると思っていた。

しかしrttr::variantはこのような操作をサポートしていない。そのため、この部分の欠陥を補填するためにQVariantの実装方法を調査した。

その実装方法も実はこのようなものである。ただし、Mapを使用して関数を格納している：

```c++
rttr::variant property;
json[propertyName] = FuncMap[property.typeId()](property);
```

カスタム型については、Qtはマクロ **Q_DECLARE_METATYPE** を使用して修飾する必要があると宣言している：

```C++
struct CustomType{};
Q_DECLARE_METATYPE(CustomType)  //このマクロはmeta typeを登録する。この型にシリアライズ関数が存在する場合、関数ポインタと型idをバインドする。
```

RTTRの登録はこのような操作をサポートしていない。因此、ここでは暫くXHTを使用してjsonのシリアライズ関数を生成することにする。

### エディタ

Coreモジュールは完全にEditorモジュールに依存していない。EditorモジュールはCore中のリフレクションデータを利用してエディタを制作する。

以前は多くのエディタを書き、多くの試行を行った。主に以下の段階を経てきた：

- **エディタを核心とする**

  - 初期の開発目的も非常に単純であった：どのようなエディタが必要で、どのような効果を完成させるために、直接エディタのUIとロジックを設計し、エディタが直接効果クラスのメソッドを呼び出して効果編集を行う。時には効果クラスにエディタ用の特定のAPIを提供する必要さえあった。このように最後まで行うと、元の効果クラスは面目がなくなり、エディタと効果クラスのコードも到処に書かれ、カップリングが非常に深刻になる。最後にインポート・エクスポートインターフェースを記述する際にも非常に注意深くなければならない。

- **Propertyを核心とする型エディタ** ：これまでに多くの段階を経て移行してきたが、言及する意義はない。以下の特徴を通じていくつかの理解を得ることができる：

  - propertyを編集目標とし、ロジックが一致する読み書きインターフェースを持つ

    > 効果クラスを記述する際に、編集可能なpropertyを予約し、それに対応する読み書きインターフェースを提供する必要がある。また、これらのpropertyを使用する際に、それらの同期状況を保証する必要がある。これによってエディタのシリアライズによって生成されたデータを正しく利用することができる。

  - エディタ部品とデータ型が一対一に対応する

    > 即ち：
    >
    > int型にはintの編集部品、曲線クラスには曲線の編集部品、ツリー構造にはツリーの編集部品...
    >
    > これらの部品の粒度は非常に細かく、完全なエディタはこれらの小さな部品から組み立てられる。
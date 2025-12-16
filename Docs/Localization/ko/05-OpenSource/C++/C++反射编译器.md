---
comments: true
---

# C++ 리플렉션 컴파일러

- Github 저장소：https://github.com/Italink/XObject

## 빌드

- CMake（GUI）를 사용하여 직접 빌드할 수 있습니다.

## 프로젝트 구조 개요

- Core：핵심 모듈
  - 3rdParty：제3자 라이브러리
  - XHT：코드 스캔 도구
  - Test：모듈 기능의 테스트 프로젝트 디렉토리
  - XObject：기본 클래스 캡슐화

## 로직 개요

### 기본 클래스 XObject

xobject는 모든 객체의 기본 클래스로서, 직렬화, 리플렉션 등의 작업을 제공합니다.

### 리플렉션

리플렉션에 사용되는 라이브러리는 Rttr입니다. Rttr은 우수한 C++ 리플렉션 라이브러리로서, 단순한 타입 정보 리플렉션을 완료할 수 있을 뿐만 아니라, 완전한 리플렉션 처리 인터페이스를 제공합니다：

- rttr::variant：stl::any와 유사하지만 더 강력하며, 많은 타입 정보 및 변환 인터페이스를 제공합니다（예: int 타입의 variant는 직접 double로 내보낼 수 있으며, 여기서는 int에서 double로의 변환 인터페이스가 호출됩니다. rttr에서는 variant의 타입 변환 함수를 사용자 정의할 수 있습니다）。
- 타입 이름에 따라 해당 인스턴스를 생성할 수 있으며, 세 가지 생성 모드（Type, Type\*, std::shared_ptr\<Type\*>）를 제공합니다。
- metadata는 정보를 추가하는 데 사용되며, 이 기능을 활용하여 Property에 많은 추가 정보를 추가할 수 있습니다。



XHT는 코드 스캔에 사용되며（QtMoc 참고）, 매크로 표시를 통해 타입의 자동 등록을 완료하여 RTTR이 등록 함수를 수동으로 작성해야 하는 문제를 해결합니다。

> XHT는 있어도 없어도 되는 것처럼 보이지만, 직접 등록 함수를 작성하는 것은 물론 더 나은理由로 XHT를 사용해야 합니다：
>
> - Rttr만 등록 함수를 작성해야 하는 것이 아니라, 직렬화 등 타입에 추가 기능을 제공하는 경우 모두 등록 함수를 작성해야 합니다. 현재의 XObject에서 두 개의 간단한 클래스를 포함하는 h 파일은 XHT가 약 100줄에 달하는 추가 코드를 생성합니다. 이 부분 코드는 고정 형식이 필요하며, 직접 작성하면 매우 피곤합니다.
> - XHT는 오류가 발생하기 어렵고, 생성 규칙만 정의하면 됩니다.
> - 원본 클래스가 변경될 때 XHT는 추가 코드의 변경을 자동으로 동기화하며, 수동으로 변경할 필요가 없습니다.

## XHT 작업 방식

- XObject.h：매크로를 정의하며, 이 매크로 표시는 XHT에 의해 포착됩니다.

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

XHT는 독립적인 명령줄 프로젝트이며, 매개변수 형식은 다음과 같습니다：

```
AxHeadeTool ${input_file} -o ${output_dir} 
Options:
-i include_dir:입력 파일의 포함 경로
```

이 도구는 파일을 스캔하고 기호 정보를 수집하여 새 파일을 생성합니다. 처리 과정은 main.cpp를 참조하십시오.

### 사용자 정의 과정

#### 키워드 정의

XHT는 키워드 조회 테이블을 사용하여 파일 내용을 기호 문자열로 효율적으로 구문 분석합니다. 여기서 테이블 내용은 `Keywords.inl` 파일에 위치하며, 이 테이블은 프로젝트 `KeywordsGen`을 통해 자동으로 생성됩니다. 프로젝트에는 XHT에서 사용되는 모든 키워드가 정의되어 있습니다.

```C++
static const Keyword pp_keywords[] = {...}   //전처리 키워드	
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

키워드는 두 개의 매개변수를 포함합니다. `{ "XENTRY","XENTRY_TOKEN"}`를 예로 들면, `XENTRY`는 코드 파일의 정확한 기호를 나타내고, `XENTRY_TOKEN`은 XHT에서 사용되는 열거형에 해당합니다. 열거형 정의는 XHT의 `Token.h` 파일에 위치합니다：

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

따라서 키워드를 정의하려면 다음 단계를 거쳐야 합니다：

- **KeywordsGen 프로젝트**에서 새 키워드 항목`｛기호, 열거형 요소｝`를 추가합니다.
- **KeywordsGen 프로젝트**를 실행하면 **XHT**의 코드 디렉토리에 `Keywords.inl`이 자동으로 생성됩니다.
- **XHT 프로젝트**의`Token.h` 파일에`열거형 요소`를 정의합니다.

#### 수집기 작성

include 및 매크로 관련 작업을 처리해야 하는 경우를 제외하고 **FileParser** 과정을 신경 쓸 필요가 없습니다. 유일하게 주의해야 할 것은 **SymbolParser**의`bool parse(FileDataDef&, const Symbols&)` 함수입니다.

parser의 switch 주요 계층 구조는 다음과 같습니다：

- 최상위 switch：파일 전역 정보 수집
  - namespace：네임스페이스 정보 수집
    - ...
  - class/struct：클래스 정보 수집
    - ...
  - ...

코드를查阅하면 class에 ` case AX_INVOKABLE_TOKEN`과`case AX_PROPERTY_TOKEN`이 포함되어 있음을 알 수 있습니다.

`XPROPERTY_TOKEN`을 예로 들면, 다음 형식의 변수 표시를 완료하는 역할을 합니다：

```
XPROPERTY(GET getX SET setX)	//여기서 GET과 SET은 해당 변수의 get 함수와 set 함수를 지정합니다. 
int var;
```

**SymbolParser**로 돌아가면, 이 case가 실제로 함수`parseAxProperty()`를 호출함을 발견할 수 있습니다：

```c++
case XPROPERTY_TOKEN:
	def.propertyList.push_back(parseXProperty());
break;
```

함수 정의는 다음과 같습니다：

```c++
void SymbolParser::parseXProperty()
{
    PropertyDef axVarDef;				//Property의 데이터 구조 정의
    next(LPAREN);						//다음 기호로 이동하고 왼쪽 괄호인지 확인합니다.
    while (test(IDENTIFIER)) {	    	//식별자인지 확인합니다（키워드 아님）。
        std::string type = lexem(); 	//기호 문자열을 가져옵니다.
        if (type == "GET") {			//가져온 문자가 GET인지 SET인지 확인합니다.
            next();						//다음 기호로 이동합니다.
            axVarDef.getter = lexem();  //기호를 저장합니다.
        }
        else if (type == "SET") {
            next();	
            axVarDef.setter = lexem();
        }
    }
    next(RPAREN);						//다음 기호로 이동하고 오른쪽 괄호인지 확인합니다.
    axVarDef.type = parseType();		//변수 타입을 구문 분석합니다.
    next(IDENTIFIER);					//다음 기호로 이동하고 식별자인지 확인합니다.
    axVarDef.name = lexem();			//변수 이름을 가져옵니다.
    until(SEMIC);						//세미콜론과 일치할 때까지 뒤로 이동합니다.
    return axVarDef;
}
```

모든 데이터 구조는`DataDef.h` 파일에 정의되어 있습니다.

따라서 정보 수집을 완료하려면 일반적으로 다음 단계를 거쳐야 합니다：

- `DataDef.h`에서 수집하려는 데이터를 정의합니다.
- `SymbolParser::parser`의 적절한 위치에 case를 추가합니다.
- **Parser**가 제공하는 편리한 메서드를借助하여 기호 구문 분석 함수를 작성하고 정보 수집을 완료합니다.

  - 기본 함수

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

  - 도구 함수

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

#### 추가 코드 생성

추가 코드 생성은 **FileGenerator**에 위치합니다.

`FileGenerator.cpp`를 열면, 추가 코드 생성이 단순히 fprintf를 통해 파일에 데이터를 쓰는 것임을 발견할 수 있습니다：

```C++
bool FileGenerator::generateSource()
{
	FILE* out;
	std::filesystem::path outputPath(fileData->outputPath);

	if (fopen_s(&out, outputPath.string().c_str(), "w") != 0) {
		return false;
	}
	std::string header_path = std::regex_replace(std::filesystem::relative(fileData->inputFilePath, outputPath.parent_path()).string(), std::regex("\\\\"), "/");
	fprintf(out, "#include \"%s\"\n", header_path.c_str());					//현재 cpp의 h 디렉토리에 대한 include 코드 생성
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

따라서 이 단계에서는 수집된 정보에 따라 코드를 쓰기만 하면 됩니다.



## 자동 빌드

**XHT**가 있으면 명령줄을 통해 수동으로 파일을 처리할 수 있지만, 프로젝트에서 **XHT**를 자동으로 호출하여 코드 파일을 처리하려면 어떻게 해야 할까요？

**XObject**의 **CMakeLists.txt**에서 이를 구현했습니다：

```cmake
function(target_xht_warp PROJECT_TARGET INPUT_FILE_PATH)            
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE_PATH} NAME_WE)               #확장자가 없는 파일 이름 가져오기
    set(OUTPUT_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/AutoGenFiles/XHT_${INPUT_FILE_NAME}.cpp)   
    add_custom_command(
        OUTPUT ${OUTPUT_FILE_PATH}                                                   #출력 파일 지정
        COMMAND XHT ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE_PATH} -o ${OUTPUT_FILE_PATH}  #명령줄 명령
        MAIN_DEPENDENCY ${INPUT_FILE_PATH}                           #의존성 지정, 해당 파일이 변경될 때 자동으로 명령 호출
    )          
    set_property(TARGET ${PROJECT_TARGET} APPEND PROPERTY SOURCES ${OUTPUT_FILE_PATH})       #빌드 대상에 추가
    source_group("Generated Files" FILES ${OUTPUT_FILE_PATH})                                #파일 그룹화
endfunction()
```

이 함수는 단일 파일을 처리하여附加 코드를 자동으로生成하고 프로젝트 빌드에 참여시킬 수 있습니다.

UE나 Qt처럼 코드에 특정 기호가 포함되어 있는지检测하여 자동으로 HeaderTool을 호출할 수 있을까요？答案는肯定적입니다. cmake는 다음 함수를 제공하여 코드 파일에 특정 기호가 포함되어 있는지查找할 수 있습니다：

[ **check_cxx_symbol_exists** ](https://cmake.org/cmake/help/latest/module/CheckCXXSymbolExists.html)

但实际上这个过程是比较低效的，需要遍历整个文件，比较好的办法是通过文件后缀标识哪些文件需要被HeaderTool处理，XObject的CMakeLists还提供了以下函数：

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

#### 몇 가지 문제

##### 상속

Rttr은 템플릿을 사용하여 리플렉션 타입을 등록하며, 상속 처리가 템플릿 메타를 통해实现됩니다（자식 클래스가 부모 클래스 함수를 호출할 때, 자식 클래스에 정의된`using base_class_list = rttr::type_list<...>`에 따라 부모 클래스에서检索합니다）。因此必须把该定义写在头文件中，才能保证该定义对子类可见，这就需要在反射标记时，需要再次手动指定父类，就像是这样：

```C++
class Base : public XObject {
	XENTRY(XObject) 					//여기서 부모 클래스를 지정해야 Rttr에서 상속 관계를确定할 수 있습니다.
}
```

##### 포인터 퇴화

Rttr에서는 부모 클래스의 포인터를 사용하여 자식 클래스의 메서드를 호출할 수 없습니다. 예를 들어 다음 코드：

```c++
XObject* x = new Base;					//Base에 print 메서드가 포함되어 있습니다.
rttr::invoke(x,"print");				//호출 실패, 이유는 Rttr이 x의 실제 타입을判断할 수 없습니다.
```

XObject는 매크로`XEntry()`에 다음 정의를 추가했습니다：

```c++
virtual rttr::instance instance() override { return *this; } 
```

이것은 XObject의 자식 클래스에 rttr 인스턴스를 가져오는 가상 함수를 제공하여 위의 포인터 문제를避免합니다. 호출은 다음과 같습니다：

```C++
XObject* x = new Base;
rttr::invoke(x->instance(),"print");	//호출 성공
```

##### 등록 시기

공식적으로 제공하는 방법은 정적 등록이며, 그 원리는 다음과 같이 간소화할 수 있습니다：

```c++
//이것은 CPP에 위치합니다.
struct Register{
	Register(){
		//여기서 RTTR의 타입 등록을 수행합니다.
	}
};
static Register register;
```

설명：RTTR은 CPP에서 구조체를创建하고, 구조체의 생성자에 등록 함수를 작성한 다음 정적 인스턴스를创建하여 타입 등록을 완료합니다.

이렇게 하면 주로 다음 문제가 발생합니다：

- 등록 순서를 제어하기 어렵습니다.
- 한 번에 모든 리플렉션 타입을 등록하면 프로그램 시작 시卡顿이 발생하기 쉽습니다.

XObject에서는 이 문제를 해결하기 위해 **XMetaObject** 클래스를 추가했습니다. 그 구조는 위의`Register`와 유사합니다（즉, Rttr의 등록 코드가 생성자에 위치합니다）. 다만, 직접 cpp에서 XMetaObject의 정적 인스턴스를创建하지 않고 다음 방식으로实现합니다：

```C++
class XObject{
	static XMetaObject* staticMetaObject(){
        static XMetaObject instance;
        return &instance;
    }
}
```

`staticMetaObject()` 함수에 처음访问할 때만里面的静态实例를创建하며, XObject가 XMetaObject를 리플렉션 데이터의 유일한入口으로 사용하기 때문에, 해당 타입의 리플렉션 인터페이스를 사용할 때 자동으로 Rttr 등록이 수행됩니다.

여기에는 또 하나의 세부 사항이 있습니다：

자식 클래스 리플렉션 인터페이스를 사용하여 부모 클래스 인터페이스를 호출할 때, 부모 클래스가 이미 등록되어 있음을保证해야 합니다.

XObject에서는 이操作을 완료하기 위해 자식 MetaObject의 생성자에서 부모 클래스의`staticMetaObject`를 한 번 호출하여 부모 클래스가提前 등록되도록 합니다.

> ### 메타 객체
>
> 위에서 XMetaObject에 대해 언급했는데, 여기서는 XObject가 어떻게 이를 타입의 유일한 리플렉션入口으로 사용하는지 간단히 설명합니다.
>
> 리플렉션을 사용하는 주요 목적은 다음과 같습니다：
>
> - Property 읽기/쓰기, Property 정보（타입, 메타데이터...）获取
> - Function 호출, Function 정보（매개변수 정보...）获取
> - 타입 이름에 따라 인스턴스创建
>
> XMetaObject는 타입과 관련 있으며, 인스턴스와는无关합니다. XMetaObject를 통해 단순히 리플렉션 데이터를获取할 뿐입니다. 그 일부 코드는 다음과 같습니다：
>
> ```C++
> struct XMetaObject {
> public:
> 	XMetaObject();
> 	XObject* newInstance(std::vector<rttr::argument> args = {});	//인스턴스创建
> 	rttr::property getProperty(std::string name);					//name이라는 property获取
> 	rttr::array_range<rttr::property> getProperties();				//해당 타입의 모든 property获取
> 	rttr::method getMethod(std::string name);						//name이라는 method获取
> 	rttr::array_range<rttr::method> getMethods();					//해당 타입의 모든 method获取
> };
> ```
>
> 함수 호출은 인스턴스와 관련 있으므로, 함수를 호출하려면 인스턴스를提供해야 합니다. 이操作은 다음과 같습니다：
>
> ```c++
> Base* base = new Base;
> XMetaObject* meta = Base::staticMetaObject();			//Base의 정적 메타 객체获取
> rttr::method method = meta->getMethod("print");			//Base의 print 함수 정보获取
> method.invoke(base);  									//호출 시 인스턴스를提供해야 합니다.
> ```
>
> 주지해야 할 것은, 약화된 포인터 타입은 실제 타입의 정적 함수를 호출할 수 없습니다. 즉：
>
> ```c++
> XObejct *x = new Base;
> x->staticMetaObject();		//이때 XObject::staticMetaObject()가 호출되며 Base::staticMetaObject()가 아닙니다.
> ```
>
> 이 문제를 해결하기 위해 매크로`XEntry()`에 다음 정의를 추가했습니다：
>
> ```C++
> virtual XMetaObject* metaObject() override { return staticMetaObject(); }
> ```
>
> 또한, Rttr에서 타입 이름에 따라 타입 풀에서搜索하는 비용은高昂합니다. 이 문제를 해결하기 위해 XMetaObject는 MetaObject를 rttr::type에绑定하는额外的 가상 함수를提供합니다：
>
> ```c++
> virtual rttr::type getRttrType() { return rttr::type::get<void>(); };	
> ```
>
> XHT가 Rttr의 등록 함수를生成할 때도 이 함수의 정의를生成하며, 이 함수는确定된 rttr::type을 반환합니다.
>
> 메타 객체의 인터페이스에 따라 XObject는 인스턴스와 관련된 일부 리플렉션 인터페이도 제공합니다：
>
> ```C++
> bool setProperty(std::string name, rttr::argument var);		//속성 설정
> rttr::variant getProperty(std::string name);				//속성获取
> rttr::variant invoke(rttr::string_view name, std::vector<rttr::argument> args = {});		//함수 호출
> ```

### 직렬화

직접 직렬화 함수를 작성하면 직렬화가 어렵지 않지만, 자동 직렬화를 원하면 다음难题에 직면해야 합니다：

- 사용자 정의 타입의 직렬화
- 복잡한 타입（컨테이너）의 직렬화
- 포인터의 역직렬화
- 역직렬화의 런타임 오류检查

다른 직렬화目的에는 다른 처리 방식이 있습니다：

- 바이너리 직렬화는 최고의 성능과 메모리를拥有하지만,其内容을阅读하기 어렵습니다.
- 비바이너리 직렬화（xml, json, cbor 등）는优雅한存储 구조를拥有하며可读性이 매우强하지만, 성능 및 메모리 손실이相对较高습니다.

#### Binary

바이너리 직렬화에 사용되는 라이브러리는 [BitSery](https://github.com/fraillt/bitsery)이며, 일부 템플릿操作을 통해 복잡한 타입（컨테이너）의 직렬화를支持합니다.

타입이 BitSery 직렬화를支持하려면 템플릿 함수`serialize(Serialize& s, type& o)`만提供하면 됩니다：

```C++
struct MyStruct {
    uint32_t i;
    std::vector<float> fs;
};

template <typename Serialize>
void serialize(Serialize& s, MyStruct& o) {	 //이 템플릿 함수는 직렬화 함수로도 사용될 수 있으며, 역직렬화 함수로도 사용될 수 있습니다.
    s(o.i);
    s(o.fs);
}
```

이 객체를 직렬화하려면 다음을 수행하면 됩니다：

```C++
using SerializeBuffer = std::vector<uint8_t>;		//Buffer 타입 정의
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;
using Serializer = bitsery::Serializer<OutputAdapter>;
using Deserializer = bitsery::Deserializer<InputAdapter>;

void main(){
    MyStruct myStruct;
    SerializeBuffer buffer;
    
    //직렬화, myStruct를 buffer에输出
    bitsery::quickSerialization(OutputAdapter(buffer), myStruct); 
    
    //역직렬화, buffer에서 데이터를读取하여 myStruct에输入
    bitsery::quickDeserialization(InputAdapter(buffer), myStruct);   	
}
```

##### 템플릿 우회

XObject에서는 템플릿을避免하기 위해 직렬화와 역직렬화 함수를 분리하고, 매크로 **XENTRY()**에 다음 정의를 추가했습니다：

```c++
virtual void __intrusive_deserialize(Deserializer& deserializer) override; 
virtual void __intrusive_serialize(Serializer& serializer) override; 
```

其函数的实现은 XHT가 Property에 따라生成하며,此外,这里还会调用父类的 직렬화函数。

> 여기서는 템플릿을 사용하여 Bitsery API를转接합니다.存在的问题是：템플릿을 통해 타입의 상속 관계를判断할 수 없습니다.
>
> 다행히도 Rttr 상속을实现할 때 일부 정의를 추가했으며,这些定义을利用하면 특정 타입이 XObject의 자식 클래스인지轻松判断할 수 있습니다：
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

##### 포인터 특수화

Bitsery는 포인터의 직렬화를支持하지 않으므로,专门으로 포인터의 직렬화 템플릿을 작성해야 합니다.

포인터 타입과 원본 타입의 차이점은 다음 두 가지입니다：

- 포인터는 null일 수 있으며, nullptr는 직렬화할 수 없습니다.
- 포인터 역직렬화 시 인스턴스를新建해야 할 수 있습니다.

이 두 가지 문제를 해결하기 위해 다음과 같은解决 방식을采用했습니다：

- 직렬화 시 포인터의 타입을写入하며,空指针则空 타입을写入합니다. 역직렬화 시 항상先类型을读取하며,类型이空이 아니면继续序列化합니다.
- 직렬화 시 포인터 타입을写入했으므로, RTTR을 통해 타입 이름에 따라 인스턴스를新建할 수 있지만,该 타입에无参构造函数가 등록되어 있음을保证해야 합니다.

但对XObject来说,还需要做更多—— 对于`XObject* x = new Base();`中的x来说,并不能简单使用`std::remove_pointer_t<T>::type`来获取原类型,所以对于XObject的类型,需要从实例的metaObject对象中获取,其核心代码如下：

```c++
template<typename T, typename std::enable_if<std::is_pointer<T>::value && !rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = rttr::type::get<rttr::detail::raw_type_t<T>::type>();		//포인터 타입에 따라 원본 타입获取
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//타입写入
	if (ptr != nullptr) {
		writer(*ptr);					//포인터가 null이 아니면, 해당 포인터가 가리키는 데이터写入
	}
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value&& rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = ptr->metaObject()->getRttrType();							//metaObject에서 타입获取
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//타입写入
	if (ptr != nullptr) {
		writer(*ptr);					//포인터가 null이 아니면, 해당 포인터가 가리키는 데이터写入
	}	
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
void serialize(Deserializer& reader, T& ptr) {
	std::string typeName;				
	reader(typeName);					//타입读取
	if (typeName.empty())				//타입이空이면直接返回
		return;
	if (ptr != nullptr) {				//反序列所指的对象已不为空，则直接对其进行反序列化
		reader(*ptr);
		return;
	}
	rttr::type type = rttr::type::get_by_name(typeName);		//Rttr 타입获取
	if (!type.is_valid())
		return;
	rttr::variant var = type.create();							//Rttr을利用하여 인스턴스创建
	if (!var.can_convert<T>()) {								
		return;
	}
	ptr = var.get_value<T>();									//인스턴스가合法하면,对其进行反序列化
	reader(*ptr);
}
```

##### Include 최적화

Bitsery는 **Header - Only** 라이브러리이므로, 모든内容을 **XObject.h**에引入하지 않기 위해核心 정의를 **SerializationDefine.h**에写入하고, XObject는 이 파일만 포함합니다.其他 코드는 **SerializationBriefSyntax.h**에引入되며, XHT가附加代码를生成할 때 이 파일을 포함합니다.



#### Json And Cbor

这里使用的 라이브러리는 [nlohmann json](https://github.com/nlohmann/json.git)이며,这里的实现은有些草率하여反序列化만完成했고 역직렬화는完成하지 않았으며,调试용으로만 사용할 수 있습니다.而序列化函数也是通过XHT生成的,理所应当的, **XENTRY()** 中增加了新的定义：

```c++
virtual void __intrusive_to_json(nlohmann::json& json) const override;
```

其处理方式也与二进制 직렬화相似。

json 직렬화는最初反射를 통해实现하려 했습니다.因为我之前在Qt中也是通过反射进行json 직렬화的,可当我真正去实现的时候,才发现它原没有这么简单：

通过rttr,能读取到一个rttr::variant 数据,我要怎么做才能将其自动转换成json里的数据呢？貌似我只能这样：

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
...
```

> 곡선...救国...？

我之前一直以为：Qt中对QVariant进行IO会自动转接到对应类型的序列化函数上。

而rttr::variant并不支持这样做,所以我去查阅QVariant的做法,想要填补这部分缺陷。

发现它的做法其实也是这样的,只不过用了个Map存储函数：

```c++
rttr::variant property;
json[propertyName] = FuncMap[property.typeId()](property);
```

对于自定义类型,Qt声明需要使用宏 **Q_DECLARE_METATYPE** 进行修饰：

```C++
struct CustomType{};
Q_DECLARE_METATYPE(CustomType)  //该宏会注册meta type,如果该类型存在序列化函数,则会将函数指针与类型id进行绑定。
```

RTTR的注册并不支持这样的操作,因此这里就暂时还是用XHT生成json的序列化函数。

### 편집기

Core模块은 Editor模块에完全不依赖하며, Editor模块은 Core中的反射数据를利用하여 편집기를制作합니다.

我之前写过很多편집기,做过很多尝试,主要经历了以下的阶段：

- **편집기를核心으로**

  - 초기 개발目的也很简单：就是需要一个什么什么样的편집기,来完成什么样的效果,所以就直接开始设计편집기的UI和逻辑,편집기直接调用效果类的方法进行效果编辑,有时候还得专门让效果类提供一些特定API给편집기.这样做到最后,原本的效果类改的面目全非,편집기和效果类的代码也写的到处都是,耦合非常严重,最后编写导入导出接口的时候还得异常小心.

- **Property를核心으로 하는 타입 편집기** ：到这之前还有很多个阶段过渡,但没有再说的意义.你可以通过以下特征做一些了解：

  - property를编辑目标으로,且拥有逻辑一致的读写接口

    > 编写效果类时,应预留可编辑的property,并为之提供相应的读写接口,并在使用这些property的时候,保证它们的同步状况,这样才能正确利用편집기序列化产生的数据.

  - 편집기部件与数据类型一一对应

    > 即：
    >
    > int 타입에는 int 편집部件、曲线类에는曲线 편집部件、树结构에는树 편집部件...
    >
    > 这些部件的粒度都很细,完整的편집기都是由这些小部件组装而来.
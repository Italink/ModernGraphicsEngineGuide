---
comments: true
---

# C++ 反射编译器

- Github仓库：https://github.com/Italink/XObject

## 构建

- 使用CMake（GUI） 可直接构建

## 工程结构简述

- Core：核心模块
  - 3rdParty：第三方库
  - XHT：代码扫描工具
  - Test：模块功能的测试工程目录
  - XObject：基类封装

## 逻辑简述

### 基类XObject

xobject 作为所有对象的基类，提供序列化，反射等操作

### 反射

反射使用的库是Rttr，Rttr是一个优秀的C++反射库，它不仅仅能完成简单的类型信息反射，也提供了一套完整的反射处理接口：

- rttr::variant：用法类似stl::any，但更强大，提供了很多类型信息及转换接口（如 : 一个int类型的variant可直接导出为double，这里面就调用了int到double的转换接口，rttr里可以自定义variant的类型转换函数）
- 支持根据类型名称，创建对应实例，并提供三种创建模式（Type，Type\*，std::shared_ptr\<Type\*>）
- metadata 用于追加信息，利用该功能，可以为Property增加许多额外信息。



XHT用于代码扫描（参考QtMoc），通过宏标记来完成类型的自动注册，解决了RTTR需要手动编写注册函数问题。

> XHT看上去可有可无，自己写注册函数肯定也行，但还有更好的理由去使用XHT：
>
> - 不仅仅只有Rttr需要编写注册函数，涉及到序列化等其他为类型做附加的功能，都需要编写，目前的XObject，一个含两个简单类的h文件，XHT将生成近百行附加代码，这部分代码需要固定格式，自己写的话很累。
> - XHT不易出错，只需定义生成规则即可。
> - 原类改动时，XHT会自动同步附加代码的改动，而无需手动更改

## XHT工作方式

- XObject.h：定义宏，这些宏标记将会被XHT捕获

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

XHT是一个单独的命令行工程，其参数格式如下：

```
AxHeadeTool ${input_file} -o ${output_dir} 
Options:
-i include_dir:输入文件的包含路径
```

该工具可扫描文件，并搜集其中的符号信息，生成新的文件，处理过程请看main.cpp

### 自定义过程

#### 定义关键字

XHT通过使用关键字查找表高效地将文件内容解析为符号串，其中表的内容位于文件`Keywords.inl`中，该表是通过工程`KeywordsGen`自动生成的，工程中定义了XHT中所有使用的关键字。

```C++
static const Keyword pp_keywords[] = {...}   //预处理的关键字	
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

关键字包含两个参数，以`{ "XENTRY","XENTRY_TOKEN"}`为例，`XENTRY`表示代码文件中的确切符号，`XENTRY_TOKEN`对应XHT中使用的枚举，而枚举的定义则位于XHT的`Token.h`文件中：

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

所以要定义关键字，需要经历以下步骤：

- 在 **KeywordsGen工程** 中新增关键字条目`｛符号，枚举元素｝`
- 运行 **KeywordsGen工程** 将会在 **XHT** 的代码目录下自动生成`Keywords.inl`
- 在 **XHT工程** 的`Token.h`文件中定义`枚举元素`

#### 编写搜集器

无需关心 **FileParser** 过程，除非你需要处理include及宏相关操作，唯一要注意的是 **SymbolParser** 的`bool parse(FileDataDef&, const Symbols&)`函数。

parser中switch的主要层次结构如下：

- 顶层switch：搜集文件全局信息
  - namespace：搜集命名空间的信息
    - ...
  - class/struct：搜集类的信息
    - ...
  - ...

查阅代码可以看到class中包含了 ` case AX_INVOKABLE_TOKEN`和`case AX_PROPERTY_TOKEN`。

以`XPROPERTY_TOKEN`为例，它的作用是完成以下格式的变量标记：

```
XPROPERTY(GET getX SET setX)	//其中 GET 和 SET 指定了该变量的get函数和set函数 
int var;
```

回到 **SymbolParser** ，可以发现该case其实调用了函数`parseAxProperty()`：

```c++
case XPROPERTY_TOKEN:
	def.propertyList.push_back(parseXProperty());
break;
```

函数的定义如下：

```c++
void SymbolParser::parseXProperty()
{
    PropertyDef axVarDef;				//定义Property的数据结构
    next(LPAREN);						//移动到下一个符号并判断是否是左括号
    while (test(IDENTIFIER)) {	    	//判断是不是标识符（非关键字）
        std::string type = lexem(); 	//获取符号字符串
        if (type == "GET") {			//判断得到的字符GET还是SET
            next();						//移动到下一个符号
            axVarDef.getter = lexem();  //保存符号
        }
        else if (type == "SET") {
            next();	
            axVarDef.setter = lexem();
        }
    }
    next(RPAREN);						//移动到下一个符号并判断是否是右括号
    axVarDef.type = parseType();		//解析变量类型
    next(IDENTIFIER);					//移动到下一个符号并判断是不是标识符
    axVarDef.name = lexem();			//获取变量名
    until(SEMIC);						//往后移动，直到匹配到分号
    return axVarDef;
}
```

所有的数据结构均定义在`DataDef.h`文件中

因此，如果要完成信息的搜集，一般要经历如下步骤：

- 在`DataDef.h`定义想要搜集的数据

- 在`SymbolParser::parser`中合适的地方添加case

- 编写符号解析函数，借助 **Parser** 提供的便捷方法，完成信息的搜集

  - 基础函数

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

  - 工具函数

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

#### 生成附加代码

附加代码的生成位于 **FileGenerator** 中

打开`FileGenerator.cpp`，能会发现附加代码生成只是简单的通过fprintf向文件中写入数据：

```C++
bool FileGenerator::generateSource()
{
	FILE* out;
	std::filesystem::path outputPath(fileData->outputPath);

	if (fopen_s(&out, outputPath.string().c_str(), "w") != 0) {
		return false;
	}
	std::string header_path = std::regex_replace(std::filesystem::relative(fileData->inputFilePath, outputPath.parent_path()).string(), std::regex("\\\\"), "/");
	fprintf(out, "#include \"%s\"\n", header_path.c_str());					//生成当前cpp相较于h目录的include代码
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

所以该阶段只需根据搜集到的信息写入代码即可。



## 自动构建

有了 **XHT** ，只能手动通过命令行来处理文件，如果想在项目中能够自动调用 **XHT** 来处理代码文件，该怎么办呢？

**XObject** 的 **CMakeLists.txt** 中对此进行了实现：

```cmake
function(target_xht_warp PROJECT_TARGET INPUT_FILE_PATH)            
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE_PATH} NAME_WE)               #获取不带扩展名的文件名
    set(OUTPUT_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/AutoGenFiles/XHT_${INPUT_FILE_NAME}.cpp)   
    add_custom_command(
        OUTPUT ${OUTPUT_FILE_PATH}                                                   #指定输出文件
        COMMAND XHT ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE_PATH} -o ${OUTPUT_FILE_PATH}  #命令行指令
        MAIN_DEPENDENCY ${INPUT_FILE_PATH}                           #指定依赖，当该文件变动时，自动调用该指令
    )          
    set_property(TARGET ${PROJECT_TARGET} APPEND PROPERTY SOURCES ${OUTPUT_FILE_PATH})       #添加到构建目标中
    source_group("Generated Files" FILES ${OUTPUT_FILE_PATH})                                #文件分组
endfunction()
```

该函数能对单一文件进行处理，自动生成附加代码并参与项目的构建。

那是否能完成像UE或者Qt的那样，检测到代码中包含某些符号自动调用HeaderTool呢？答案是肯定的，cmake提供了以下函数可用于查找是否代码文件中是否包含某些符号：

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

#### 一些问题

##### 继承

Rttr使用模板注册反射类型，继承的处理是通过模板元实现 的（当子类调用父类函数时，会根据子类中定义的`using base_class_list = rttr::type_list<...>`，到父类中检索 ），因此必须把该定义写在 头文化中，才能保证该定义对子类可见，这就需要在反射标记时，需要再次手动指定父类，就像是这样：

```C++
class Base : public XObject {
	XENTRY(XObject) 					//必须在这里指定父类，否则Rttr中无法确定继承关系
}
```

##### 退化指针

在Rttr中，无法用父类的指针去调用子类的方法，比如下面的代码：

```c++
XObject* x = new Base;					//Base中包含print方法
rttr::invoke(x,"print");				//调用失败，原因是Rttr无法判断x的真实类型
```

XObject在宏`XEntry()`中添加了如下定义：

```c++
virtual rttr::instance instance() override { return *this; } 
```

这为XObject的子类提供了一个虚函数，用于获取rttr的实例，这就避免了上面指针的问题，调用看起来像是这样的：

```C++
XObject* x = new Base;
rttr::invoke(x->instance(),"print");	//调用成功
```

##### 注册时机

官方给的做法是静态注册，其原理可以简化如下：

```c++
//此位于CPP中
struct Register{
	Register(){
		//此处进行RTTR的类型注册
	}
};
static Register register;
```

说明：RTTR通过在CPP中创建一个结构体，把注册函数写在结构体的构造函数中，然后再创建一个静态实例，从而完成类型的注册。

这样做主要存在以下问题：

- 注册的顺序不容易控制。
- 一次性注册所有反射类型容易导致程序启动时的卡顿。

XObject中为了解决该问题，增加了 **XMetaObject** 类，它的结构同上面的`Register`相似（即Rttr的注册代码位于其构造函数中），不同的是，我们并没有直接在cpp中创建XMetaObject的静态实例，而是通过如下的方式实现：

```C++
class XObject{
	static XMetaObject* staticMetaObject(){
        static XMetaObject instance;
        return &instance;
    }
}
```

当初次访问`staticMetaObject()`函数时，才会创建里面的静态实例，在加上XObject使用XMetaObject作为反射数据的唯一入口，所以在使用该类型的反射接口时，会自动进行Rttr注册。

这里还有个细节：

当使用子类反射接口时调用父类接口时，需要保证父类已经注册。

XObject中为了完成该操作，在子类MetaObject的构造函数中，会调用一次父类的`staticMetaObject`保证父类提前注册

> ### 元对象
>
> 上面提到了XMetaObject，这里简单说下XObject是如何使用它作为类型唯一的反射入口。
>
> 使用反射，主要是为了以下用途：
>
> - 读写Property、获取Property信息（类型，元数据...）
> - 调用Function、获取Function信息（参数信息...）
> - 根据类型名称创建实例
>
> 而XMetaObject是以类型相关，实例无关的，通过XMetaObject仅仅是为了获取反射数据，它的部分代码如下
>
> ```C++
> struct XMetaObject {
> public:
> 	XMetaObject();
> 	XObject* newInstance(std::vector<rttr::argument> args = {});	//创建实例
> 	rttr::property getProperty(std::string name);					//获取名为name的property
> 	rttr::array_range<rttr::property> getProperties();				//获取该类型的所有property
> 	rttr::method getMethod(std::string name);						//获取名为name的method
> 	rttr::array_range<rttr::method> getMethods();					//获取该类型的所有method
> };
> ```
>
> 而调用函数是与实例相关的，想要调用函数就必须提供一个实例，这个操作看起来像是这样：
>
> ```c++
> Base* base = new Base;
> XMetaObject* meta = Base::staticMetaObject();			//获取Base的静态元对象
> rttr::method method = meta->getMethod("print");			//获取Base的print函数的信息
> method.invoke(base);  									//调用时需要提供实例
> ```
>
> 众所周知，弱化的指针类型是无法调用实际类型的静态函数，即：
>
> ```c++
> XObejct *x = new Base;
> x->staticMetaObject();		//此时将会调用XObject::staticMetaObject()而非Base::staticMetaObject()
> ```
>
> 为了解决这个问题，宏`XEntry()`中增加了如下定义：
>
> ```C++
> virtual XMetaObject* metaObject() override { return staticMetaObject(); }
> ```
>
> 另外，在Rttr中根据类型名称在类型池中进行搜索的代价是高昂的，为了解决这个问题，XMetaObject提供了额外的虚函数用于MetaObject到rttr::type的绑定：
>
> ```c++
> virtual rttr::type getRttrType() { return rttr::type::get<void>(); };	
> ```
>
> XHT生成Rttr的注册函数时，也会生成该函数的定义，该函数返回一个已经确定了的rttr::type
>
> 根据元对象的接口，XObject也提供了一些与实例相关的反射接口：
>
> ```C++
> bool setProperty(std::string name, rttr::argument var);		//设置属性
> rttr::variant getProperty(std::string name);				//获取属性
> rttr::variant invoke(rttr::string_view name, std::vector<rttr::argument> args = {});		//调用函数
> ```

### 序列化

如果手动编写序列化函数的话，序列化并不困难，但如果想要自动序列化，就得面临以下难题：

- 自定义类型的序列化
- 复杂类型（容器）的序列化
- 指针的反序列化
- 反序列化的运行时错误检查

对于不同的序列化目的有着不同的处理方式：

- 二进制序列化拥有最好的性能及内存，但其内容难以阅读。
- 非二进制序列化（如xml，json，cbor等），拥有优雅的存储结构，可读性很强，但性能及内存的损耗相对较高。

#### Binary

二进制序列化使用的库是[BitSery](https://github.com/fraillt/bitsery)，它通过了一些模板操作来支持复杂类型（容器）的序列化

想要一个类型支持BitSery序列化，只需提供模板函数`serialize(Serialize& s, type& o)`：

```C++
struct MyStruct {
    uint32_t i;
    std::vector<float> fs;
};

template <typename Serialize>
void serialize(Serialize& s, MyStruct& o) {	 //该模板函数即可作为序列化函数，又可作为反序列函数
    s(o.i);
    s(o.fs);
}
```

要想序列化该对象，只需：

```C++
using SerializeBuffer = std::vector<uint8_t>;		//定义Buffer的类型
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;
using Serializer = bitsery::Serializer<OutputAdapter>;
using Deserializer = bitsery::Deserializer<InputAdapter>;

void main(){
    MyStruct myStruct;
    SerializeBuffer buffer;
    
    //序列化，将myStruct输出到buffer中
    bitsery::quickSerialization(OutputAdapter(buffer), myStruct); 
    
    //反序列化，从buffer中读取数据输入到myStruct中
    bitsery::quickDeserialization(InputAdapter(buffer), myStruct);   	
}
```

##### 绕过模板

XObject中为了避免模板，将序列化与反序列化函数拆分，在宏 **XENTRY()** 中添加了如下定义：

```c++
virtual void __intrusive_deserialize(Deserializer& deserializer) override; 
virtual void __intrusive_serialize(Serializer& serializer) override; 
```

其函数的实现由XHT根据Property生成，此外，这里还会调用父类的序列化函数。

> 这里使用了模板转接Bitsery API，存在的问题是：通过模板无法判断类型的继承关系
>
> 好在实现Rttr继承时添加了一些定义，利用这些定义可以轻松判断某个类型是否是XObject的子类：
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

##### 指针特化

由于Bitsery并不支持指针的序列化，因此要专门编写指针的序列化模板。

指针类型和原类型有两点不同：

- 指针可以为空，nullptr不能进行序列化
- 指针反序列化时，可能需要新建实例

为了解决这两个问题，采用了如下的解决方式：

- 序列化时写入指针的类型，空指针则写入空类型，反序列化时，一定会先读取类型，类型不为空，则继续序列化
- 由于序列化时，写入了指针类型，所以可以通过RTTR根据类型名称新建实例，但需要保证该类型注册了一个无参构造函数。

但对XObject来说，还需要做更多—— 对于`XObject* x = new Base();`中的x来说，并不能简单使用`std::remove_pointer_t<T>::type`来获取原类型，所以对于XObject的类型，需要从实例的metaObject对象中获取，其核心代码如下：

```c++
template<typename T, typename std::enable_if<std::is_pointer<T>::value && !rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = rttr::type::get<rttr::detail::raw_type_t<T>::type>();		//根据指针类型获取原始类型
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//写入类型
	if (ptr != nullptr) {
		writer(*ptr);					//如果指针不为空，写入该指针所指数据
	}
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value&& rttr::detail::has_base_class_list<rttr::detail::raw_type_t<T>>::value>::type* = nullptr>
void serialize(Serializer& writer, T& ptr) {
	std::string typeName;
	if (ptr != nullptr) {
		rttr::type type = ptr->metaObject()->getRttrType();							//从metaObject中获取类型
		typeName = type.get_raw_type().get_name().to_string();
	}
	writer(typeName);					//写入类型
	if (ptr != nullptr) {
		writer(*ptr);					//如果指针不为空，写入该指针所指数据
	}	
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
void serialize(Deserializer& reader, T& ptr) {
	std::string typeName;				
	reader(typeName);					//读取类型
	if (typeName.empty())				//如果类型为空，则直接返回
		return;
	if (ptr != nullptr) {				//如果反序列所指的对象已不为空，则直接对其进行反序列化
		reader(*ptr);
		return;
	}
	rttr::type type = rttr::type::get_by_name(typeName);		//获取Rttr类型
	if (!type.is_valid())
		return;
	rttr::variant var = type.create();							//利用Rttr创建实例
	if (!var.can_convert<T>()) {								
		return;
	}
	ptr = var.get_value<T>();									//如果实例合法，则对其进行反序列化
	reader(*ptr);
}
```

##### Include优化

Bitsery是一个 **Header - Only** 库，为了不将所有内容都引入到 **XObject.h** 中，这里的做法是把核心的定义写入到 **SerializationDefine.h** 中，XObject只包含这个文件；而对于其他的代码，在 **SerializationBriefSyntax.h** 中被引入，XHT生成附加代码时会包含该文件。



#### Json And Cbor

这里使用的库是 [nlohmann json](https://github.com/nlohmann/json.git)，这里的实现就有些草率，只完成了反序列而没有反序列化，只能用作调试。而序列化函数也是通过XHT生成的，理所应当的， **XENTRY()** 中增加了新的定义：

```c++
virtual void __intrusive_to_json(nlohmann::json& json) const override;
```

其处理方式也与二进制序列化相似。

json序列化一开始是想通过反射来做，因为我之前在Qt中也是通过反射进行json序列化的，可当我真正去实现的时候，才发现它原没有这么简单：

通过rttr，能读取到一个rttr::variant 数据，我要怎么做才能将其自动转换成json里的数据呢？貌似我只能这样：

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

> 曲线...救国...？

我之前一直以为：Qt中对QVariant进行IO会自动转接到对应类型的序列化函数上。

而rttr::variant并不支持这样做，所以我去查阅QVariant的做法，想要填补这部分缺陷。

发现它的做法其实也是这样的，只不过用了个Map存储函数：

```c++
rttr::variant property;
json[propertyName] = FuncMap[property.typeId()](property);
```

对于自定义类型，Qt声明需要使用宏 **Q_DECLARE_METATYPE** 进行修饰：

```C++
struct CustomType{};
Q_DECLARE_METATYPE(CustomType)  //该宏会注册meta type，如果该类型存在序列化函数，则会将函数指针与类型id进行绑定。
```

RTTR的注册并不支持这样的操作，因此这里就暂时还是用XHT生成json的序列化函数。

### 编辑器

Core模块完全不依赖于Editor模块，而Editor模块则是利用Core中的反射数据制作编辑器

我之前写过很多编辑器，做过很多尝试，主要经历了以下的阶段：

- **编辑器为核心**

  - 早期的开发目的也很简单：就是需要一个什么什么样的编辑器，来完成什么样的效果，所以就直接开始设计编辑器的UI和逻辑，编辑器直接调用效果类的方法进行效果编辑，有时候还得专门让效果类提供一些特定API给编辑器。这样做到最后，原本的效果类改的面目全非，编辑器和效果类的代码也写的到处都是，耦合非常严重，最后编写导入导出接口的时候还得异常小心。

- **Property为核心的类型编辑器** ：到这之前还有很多个阶段过渡，但没有再说的意义。你可以通过以下特征做一些了解：

  - property为编辑目标，且拥有逻辑一致的读写接口

    > 编写效果类时，应预留可编辑的property，并为之提供相应的读写接口，并在使用这些property的时候，保证它们的同步状况，这样才能正确利用编辑器序列化产生的数据。

  - 编辑器部件与数据类型一一对应

    > 即：
    >
    > int类型有int的编辑部件、曲线类有曲线的编辑部件、树结构有树的编辑部件...
    >
    > 这些部件的粒度都很细，完整的编辑器都是由这些小部件组装而来。

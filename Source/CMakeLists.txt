cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsEngineGuide CXX)

option(QENGINE_BUILD_RELEASE "Build Release" FALSE)

set(RELEASE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Release)

function(add_shader TARGET_NAME SHADER_PATH)
    set(OUTPUT_SHADER_PATH ${SHADER_PATH}.qsb)  #输出文件路径
    add_custom_command(
        OUTPUT ${OUTPUT_SHADER_PATH}        #指定输出文件
        COMMAND qsb.exe -c ${SHADER_PATH} -o ${OUTPUT_SHADER_PATH} --glsl 430 --msl 12 --hlsl 60    #执行QSB工具
        MAIN_DEPENDENCY ${SHADER_PATH}      #指定依赖文件，即该文件变动时，触发上述命令
    )  
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY SOURCES ${OUTPUT_SHADER_PATH})               #需要把输出文件添加到一个构建目标中，才会触发CustomCommand
    source_group("Shader Files" FILES ${SHADER_PATH} ${OUTPUT_SHADER_PATH})                         #将着色器文件分类
endfunction()

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCE FILES ${EXAMPLE_PATH}/*.h  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCE}
    )
    target_link_libraries(${EXAMPLE_NAME} PRIVATE QEngineUtilities)
    qengine_setup_env(${EXAMPLE_NAME})
endfunction()

if(QENGINE_BUILD_RELEASE)
    set(QENGINE_RESOURCE_DIR "./Resources")
else()
    set(QENGINE_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
endif()

function(add_example_dir_internal DIR_PATH GROUP_NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${DIR_PATH} ${DIR_PATH}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(NOT EXISTS "${DIR_PATH}/${EXAMPLE_NAME}/Source")
            add_example_dir_internal(${DIR_PATH}/${EXAMPLE_NAME} ${GROUP_NAME}/${EXAMPLE_NAME})
        elseif(IS_DIRECTORY ${DIR_PATH}/${EXAMPLE_NAME})
            add_example(${DIR_PATH}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${GROUP_NAME})
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE RESOURCE_DIR="${QENGINE_RESOURCE_DIR}")      
            if(QENGINE_BUILD_RELEASE)
                set_target_properties(${EXAMPLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${RELEASE_OUTPUT_DIR}$<0:>)
                set_target_properties(${EXAMPLE_NAME} PROPERTIES OUTPUT_NAME ${GROUP_NAME}-${EXAMPLE_NAME})
            endif()
        endif()
    endforeach()
endfunction()

function(add_example_dir DIR_NAME)
    add_example_dir_internal(${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME} ${DIR_NAME})
endfunction()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

add_subdirectory(0-QEngineUtilities)

set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGEDITOR Debug Release)
find_package(Qt6 COMPONENTS Core Widgets Gui Multimedia ShaderTools REQUIRED)

add_example_dir(1-GraphicsAPI)
add_example_dir(2-EngineTechnology)
add_example_dir(3-GraphicsTechnology)

# 部分示例的特殊操作
add_shader(03-Shader ${QENGINE_RESOURCE_DIR}/Shader/color.frag)
set_property(TARGET 01-Editor PROPERTY AUTOMOC ON)
set_property(TARGET 05-GPUParticles PROPERTY AUTOMOC ON)
set_property(TARGET 04-SSAO PROPERTY AUTOMOC ON)

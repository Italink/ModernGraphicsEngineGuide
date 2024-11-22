cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsEngineGuide CXX)

set(INSTALLATION_PACKAGE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/InstallationPackage)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGEDITOR Debug Release)
find_package(Qt6 COMPONENTS Core Widgets Gui Multimedia ShaderTools REQUIRED)

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
    file(GLOB_RECURSE PROJECT_SOURCES FILES  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.h ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCES}
    )
    target_link_libraries(${EXAMPLE_NAME} PRIVATE QEngineLaunch)
    add_dependencies(${EXAMPLE_NAME} QEngineCopyDLL)
endfunction()

function(add_example_dir DIR_PATH GROUP_NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${DIR_PATH} ${DIR_PATH}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(NOT EXISTS "${DIR_PATH}/${EXAMPLE_NAME}/Source")
            add_example_dir(${DIR_PATH}/${EXAMPLE_NAME} ${GROUP_NAME}/${EXAMPLE_NAME})
        elseif(IS_DIRECTORY ${DIR_PATH}/${EXAMPLE_NAME})
            add_example(${DIR_PATH}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${GROUP_NAME})   
            install(TARGETS ${EXAMPLE_NAME} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})
        endif()
    endforeach()
endfunction()

add_subdirectory(Source/0-QEngineUtilities)
file(GLOB PLUGIN_DIR_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Source/Plugins/*)
foreach(PLUGIN_DIR ${PLUGIN_DIR_LIST})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_DIR})
        add_subdirectory(${PLUGIN_DIR})
    endif()
endforeach()
qengine_make_copy_target(QEngineCopyDLL)

add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/1-GraphicsAPI 1-GraphicsAPI)
add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/2-EngineTechnology 2-EngineTechnology)
add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/Source/3-GraphicsTechnology 3-GraphicsTechnology)

# 部分示例的特殊操作
add_shader(03-Shader ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/color.frag)
set_property(TARGET 00-Blur PROPERTY AUTOMOC ON)
set_property(TARGET 01-Bloom PROPERTY AUTOMOC ON)
set_property(TARGET 02-Outlining PROPERTY AUTOMOC ON)
set_property(TARGET 03-SSAO PROPERTY AUTOMOC ON)
set_property(TARGET 04-DepthOfField PROPERTY AUTOMOC ON)
set_property(TARGET 00-RenderingArchitecture PROPERTY AUTOMOC ON)
set_property(TARGET 05-GPUParticles PROPERTY AUTOMOC ON)
set_property(TARGET 03-SSAO PROPERTY AUTOMOC ON)



execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${CMAKE_CURRENT_BINARY_DIR}/Resources)

get_property(QENGINE_TARGET_LIST GLOBAL PROPERTY QENGINE_TARGET_LIST)
foreach(QENGINE_TARGET ${QENGINE_TARGET_LIST})
    get_target_property(PLUGIN_TYPE ${QENGINE_TARGET} PLUGIN_TYPE)
    if(PLUGIN_TYPE)
        install(TARGETS ${QENGINE_TARGET} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR}/Plugins)
    else()
        install(TARGETS ${QENGINE_TARGET} DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})
    endif()
endforeach()

install(DIRECTORY Resources DESTINATION ${INSTALLATION_PACKAGE_OUTPUT_DIR})

install(CODE "execute_process(COMMAND windeployqt 00-Blur.exe WORKING_DIRECTORY ${INSTALLATION_PACKAGE_OUTPUT_DIR})")

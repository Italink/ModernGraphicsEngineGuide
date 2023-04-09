cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsEngineGuide CXX)

option(QENGINE_BUILD_RELEASE "Build Release" FALSE)

set(RELEASE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Release)

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCE FILES ${EXAMPLE_PATH}/*.h  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCE}
    )
    target_link_libraries(${EXAMPLE_NAME} PRIVATE QEngineUtilities)
endfunction()

function(add_example_dir_internal DIR_PATH GROUP_NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${DIR_PATH} ${DIR_PATH}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(NOT EXISTS "${DIR_PATH}/${EXAMPLE_NAME}/Source")
            add_example_dir_internal(${DIR_PATH}/${EXAMPLE_NAME} ${GROUP_NAME}/${EXAMPLE_NAME})
        elseif(IS_DIRECTORY ${DIR_PATH}/${EXAMPLE_NAME})
            add_example(${DIR_PATH}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${GROUP_NAME})
            if(QENGINE_BUILD_RELEASE)
                file(RELATIVE_PATH RESOURCE_DIR ${RELEASE_OUTPUT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
                target_compile_definitions(${EXAMPLE_NAME} PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")
                set_target_properties(${EXAMPLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${RELEASE_OUTPUT_DIR}$<0:>)
                set_target_properties(${EXAMPLE_NAME} PROPERTIES OUTPUT_NAME ${GROUP_NAME}-${EXAMPLE_NAME})
            else()
                target_compile_definitions(${EXAMPLE_NAME} PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources")
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
set_property(TARGET 01-Editor PROPERTY AUTOMOC ON)

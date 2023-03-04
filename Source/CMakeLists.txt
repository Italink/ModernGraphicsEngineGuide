cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsEngineGuide CXX)

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCE FILES ${EXAMPLE_PATH}/*.h  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCE}
    )
    target_compile_definitions(${EXAMPLE_NAME} PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources")
    target_link_libraries(${EXAMPLE_NAME} PRIVATE QEngineUtilities)
endfunction()

function(add_example_dir DIR_PATH)
    set(EXAMPLE_FULL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_PATH})
    get_filename_component(DIR_NAME ${EXAMPLE_FULL_DIR} NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${EXAMPLE_FULL_DIR} ${EXAMPLE_FULL_DIR}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(IS_DIRECTORY ${EXAMPLE_FULL_DIR}/${EXAMPLE_NAME})
            add_example(${EXAMPLE_FULL_DIR}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${DIR_NAME})
        endif()
    endforeach()
endfunction()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

add_subdirectory(0-QEngineUtilities)

add_example_dir(1-GraphicsAPI)
add_example_dir(2-EngineTechnology)
add_example_dir(3-GraphicsTechnology)

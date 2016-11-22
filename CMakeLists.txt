cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME napalm)
project(${PROJECT_NAME})

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)


set_property(GLOBAL PROPERTY USE_FOLDERS On) 

set(PUBLIC_HEADERS
    napalm/napalm.h
)

set(SRCS
    napalm/napalm.cpp
)

add_library(${PROJECT_NAME} ${SRCS} ${PUBLIC_HEADERS})
add_library(::${PROJECT_NAME} ALIAS napalm)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>
        $<INSTALL_INTERFACE:include/napalm>
    
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}-config-version.cmake"
  VERSION 0.0.1
  COMPATIBILITY AnyNewerVersion
)

configure_file(cmake/${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets DESTINATION 
    ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake 
        ${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}-config-version.cmake
    DESTINATION 
        lib/cmake/${PROJECT_NAME})
    
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE :: DESTINATION lib/cmake/${PROJECT_NAME})

install(FILES
    ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h
	${PUBLIC_HEADERS}
    DESTINATION include/napalm/napalm)

option(BUILD_EXAMPLES "Build ${PROJECT_NAME} examples!" OFF)
if (BUILD_EXAMPLES)
    #file (GLOB SRCS samples/simple.cpp)
    #find_package(OpenCV REQUIRED)
    add_executable(napalm_simple samples/simple.cpp)
    set_target_properties(napalm_simple PROPERTIES DEBUG_POSTFIX _d)
    set_target_properties(napalm_simple PROPERTIES FOLDER examples)
    #target_include_directories(napalm_sim PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(napalm_simple PRIVATE ::napalm )#${OpenCV_LIBRARIES}
    install(TARGETS napalm_simple DESTINATION bin)
endif()

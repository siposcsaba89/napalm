cmake_minimum_required(VERSION 3.0)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(napalm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)


set_property(GLOBAL PROPERTY USE_FOLDERS On) 

set(PUBLIC_HEADERS
    napalm/napalm.h
    napalm/program_store.h
    napalm/context_manager.h
    napalm/program_store.h
)

set(SRCS
    napalm/napalm.cpp
    napalm/program_store.cpp
    napalm/context_manager.cpp
    napalm/program_store.cpp
)


add_library(${PROJECT_NAME} ${SRCS} ${PUBLIC_HEADERS})
add_library(::${PROJECT_NAME} ALIAS napalm)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)


include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h)

find_package(OpenCL QUIET)
if (OpenCL_FOUND)
    message(STATUS "Building with OpenCL, ${OpenCL_LIBRARIES}")
    add_subdirectory(napalm_OpenCL)
endif(OpenCL_FOUND)

find_package(CUDA QUIET)
if (CUDA_FOUND)
    message(STATUS "Building with CUDA")
    add_subdirectory(napalm_CUDA)
endif(CUDA_FOUND)



target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
    
)

#target_link_libraries(napalm PRIVATE ::napalm_OpenCL)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_file.cmake.in
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreatePrograms.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/program_template.cpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/program_template.h.in
        
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
	include(cmake/CreatePrograms.cmake)
	set (PROGRAMS samples/simple.cl samples/simple.cu)
	set(EMBED_PROGRAM_FILES TRUE)
	set(COMPILE_IN_BUILD_TYPE_IF_POSSIBLE FALSE)
    set(PROGRAM_RUNTIME_SOURCE_DIR "./alma/korte/")
	create_programs("${PROGRAMS}"
		output napalm_simple 
        COMPILE_IN_BUILD_TYPE_IF_POSSIBLE 
        ${EMBED_PROGRAM_FILES}
        ${PROGRAM_RUNTIME_SOURCE_DIR}
        )
    set_target_properties(napalm_simple PROPERTIES DEBUG_POSTFIX _d)
    set_target_properties(napalm_simple PROPERTIES FOLDER examples)
    #target_include_directories(napalm_sim PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(napalm_simple PRIVATE ::napalm ::napalm_OpenCL)#${OpenCV_LIBRARIES}
    install(TARGETS napalm_simple DESTINATION bin)
endif()

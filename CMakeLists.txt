cmake_minimum_required(VERSION 3.0)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

project(napalm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


option(BUILD_TIME_LINK_TO_NAPALM_CUDA "Link to napalm_CUDA in build time" OFF)
option(BUILD_TIME_LINK_TO_NAPALM_OPENCL "Link to napalm_OpenCL in build time" OFF)

if (NOT BUILD_SHARED_LIBS)
    set(BUILD_TIME_LINK_TO_NAPALM_CUDA ON CACHE BOOL "" FORCE)
    set(BUILD_TIME_LINK_TO_NAPALM_OPENCL ON CACHE BOOL "" FORCE)
endif()

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

include(cmake/CreatePrograms.cmake)

add_library(${PROJECT_NAME} ${SRCS} ${PUBLIC_HEADERS})
add_library(::${PROJECT_NAME} ALIAS napalm)
if (BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()


include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h)

find_package(OpenCL REQUIRED)
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

set(TARGET_LINK_LIBRARIES_TYPE PUBLIC)
if(BUILD_SHARED_LIB)
    set(TARGET_LINK_LIBRARIES_TYPE PRIVATE)
endif()

if (BUILD_TIME_LINK_TO_NAPALM_CUDA AND TARGET ::napalm_CUDA)
    target_link_libraries(napalm ${TARGET_LINK_LIBRARIES_TYPE} ::napalm_CUDA)
	target_compile_definitions(napalm PRIVATE -DBUILD_TIME_LINK_TO_NAPALM_CUDA)
endif()
if (BUILD_TIME_LINK_TO_NAPALM_OPENCL AND TARGET ::napalm_OpenCL)
    target_link_libraries(napalm ${TARGET_LINK_LIBRARIES_TYPE} ::napalm_OpenCL)
	target_compile_definitions(napalm PRIVATE -DBUILD_TIME_LINK_TO_NAPALM_OPENCL)
endif()

if (UNIX)
    target_link_libraries(napalm ${TARGET_LINK_LIBRARIES_TYPE} dl)
endif (UNIX)

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
	set (PROGRAMS samples/simple.cl samples/simple.cu)
	set(EMBED_PROGRAM_FILES FALSE)
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
    target_link_libraries(napalm_simple PRIVATE ::napalm)#${OpenCV_LIBRARIES}
    install(TARGETS napalm_simple DESTINATION bin)
endif()

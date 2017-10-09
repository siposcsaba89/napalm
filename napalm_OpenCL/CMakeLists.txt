cmake_minimum_required(VERSION 3.0)
project(napalm_OpenCL)

find_package(OpenCL REQUIRED)


set(PUBLIC_HDRS
    include/napalm/opencl/opencl_context.h
    include/napalm/opencl/cl_buffer.h
    include/napalm/opencl/cl_img.h
    include/napalm/opencl/cl_program.h
    include/napalm/opencl/create.h
)

set(SRCS 
    src/opencl_context.cpp
    src/create.cpp
    src/cl_buffer.cpp
    src/cl_img.cpp
    src/cl_program.cpp
    src/cl_utils.h
    src/cl_utils.cpp
    
)

add_library(napalm_OpenCL SHARED ${PUBLIC_HDRS} ${SRCS})
add_library(::napalm_OpenCL ALIAS napalm_OpenCL)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)

source_group(napalm_OpenCL FILES ${PUBLIC_HDRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCL_INCLUDE_DIRS})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/napalm/opencl/${PROJECT_NAME}_export.h)


target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>
        $<INSTALL_INTERFACE:include/napalm/${PROJECT_NAME}>
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
	${PUBLIC_HDRS}
    DESTINATION include/napalm/${PROJECT_NAME}/napalm/opencl)



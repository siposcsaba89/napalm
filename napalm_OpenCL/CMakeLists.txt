cmake_minimum_required(VERSION 3.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(napalm_OpenCL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_WITH_GL_SHARING_ENABLED "Build with opengl sharing enabled (glew, egl/es2 required)" ON)


find_package(OpenCL REQUIRED)
add_library(::opencl SHARED IMPORTED)
set_property(TARGET ::opencl PROPERTY IMPORTED_IMPLIB_RELEASE ${OpenCL_LIBRARY})
set_property(TARGET ::opencl PROPERTY IMPORTED_LOCATION_RELEASE ${OpenCL_LIBRARY})
set_property(TARGET ::opencl PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_property(TARGET ::opencl PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<INSTALL_INTERFACE:${OpenCL_INCLUDE_DIR}>  # <prefix>/include/mylib
    $<BUILD_INTERFACE:${OpenCL_INCLUDE_DIR}>  # <prefix>/include/mylib
)


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

add_library(napalm_OpenCL ${PUBLIC_HDRS} ${SRCS})
add_library(::napalm_OpenCL ALIAS napalm_OpenCL)
if (BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX _rd)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

if (BUILD_WITH_GL_SHARING_ENABLED)
	find_package(glew QUIET CONFIG)
	if (NOT glew_FOUND)
	    # hope gles2 is available
	    find_library(GLESV2_LIBRARY NAMES GLESv2 PATHS /usr/lib/)
	    find_path(GL2_INCLUDE NAMES GLES3/gl3.h PATHS /usr/include)
	 
	    if (GLESV2_LIBRARY AND GL2_INCLUDE)
	        add_library(::gles2 SHARED IMPORTED)
	        set_property(TARGET ::gles2 PROPERTY IMPORTED_IMPLIB_RELEASE ${GLESV2_LIBRARY})
	        set_property(TARGET ::gles2 PROPERTY IMPORTED_LOCATION_RELEASE ${GLESV2_LIBRARY})
	        set_property(TARGET ::gles2 PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
	        set_property(TARGET ::gles2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
	            $<INSTALL_INTERFACE:${GL2_INCLUDE}>  # <prefix>/include/mylib
	            $<BUILD_INTERFACE:${GL2_INCLUDE}>  # <prefix>/include/mylib
	        )
	        set_property(TARGET ::gles2 PROPERTY INTERFACE_COMPILE_DEFINITIONS HAVE_GLES2 GLFW_INCLUDE_ES2)
	        set(gles2_FOUND TRUE)
	    else()
	        set(gles2_FOUND FALSE)
	    endif()
	endif()
	 
	if (glew_FOUND)
	    target_link_libraries(${PROJECT_NAME} PUBLIC ::glew)
	    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAVE_GLEW=1)
        set(gles2_FOUND FALSE)
	elseif(gles2_FOUND)
	    target_link_libraries(${PROJECT_NAME} PUBLIC ::gles2)
	else()
	    message(FATAL_ERROR "Could not find glew nor gles2")
	endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DGL_SHARING_ENABLED=1)
endif()


source_group(napalm_OpenCL FILES ${PUBLIC_HDRS})

if (BUILD_SHARED_LIBS)
    target_link_libraries(${PROJECT_NAME} PRIVATE ::opencl)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC ::opencl)
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/napalm/opencl/${PROJECT_NAME}_export.h)


target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
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
    ${CMAKE_BINARY_DIR}/gen/napalm/opencl/${PROJECT_NAME}_export.h
	${PUBLIC_HDRS}
    DESTINATION include/napalm/${PROJECT_NAME}/napalm/opencl)



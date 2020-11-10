cmake_minimum_required(VERSION 3.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(napalm_CUDA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(CUDA_RUNTIME_COMPILE "Runtime compile cuda kernels!" ON)
option(BUILD_WITH_GL_SHARING_ENABLED "Build with opengl sharing enabled (glew, egl/es2 required)" ON)


find_package(CUDA REQUIRED)
        
#add_definitions(-DCUDA_API_PER_THREAD_DEFAULT_STREAM)
#set (CUDA_GENERATED_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code=sm_50)
#list(APPEND CUDA_NVCC_FLAGS "--use_fast_math")
add_library(::cuda SHARED IMPORTED)
find_library(CUDA_DRIVER_API NAMES cuda PATHS 
    ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64/
    ${CUDA_TOOLKIT_ROOT_DIR}/lib64/
    ${CUDA_TOOLKIT_ROOT_DIR}/lib64/stubs/
    NO_DEFAULT_PATH)
if (NOT CUDA_DRIVER_API)
    message(FATAL_ERROR "Coud not find cuda driver api library!")
endif()
set_property(TARGET ::cuda PROPERTY IMPORTED_IMPLIB_RELEASE ${CUDA_DRIVER_API})
set_property(TARGET ::cuda PROPERTY IMPORTED_LOCATION_RELEASE ${CUDA_DRIVER_API})
if (CUDA_RUNTIME_COMPILE)
    find_library(CUDA_NVRTC_API NAMES nvrtc PATHS ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64/ NO_DEFAULT_PATH)
    if (NOT CUDA_NVRTC_API)
        message(STATUS "Coud not find cuda nvrtc library! Disabling runtime compilation.")
        set(CUDA_RUNTIME_COMPILE 0)
    else()
        set_property(TARGET ::cuda PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE ${CUDA_NVRTC_API})
    endif()
endif()
set_property(TARGET ::cuda PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_property(TARGET ::cuda PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<INSTALL_INTERFACE:${CUDA_INCLUDE_DIRS}>  # <prefix>/include/mylib
    $<BUILD_INTERFACE:${CUDA_INCLUDE_DIRS}>  # <prefix>/include/mylib
)


set(PUBLIC_HDRS
    include/napalm/cuda/cuda_context.h
    include/napalm/cuda/cuda_buffer.h
    include/napalm/cuda/cuda_img.h
    include/napalm/cuda/cuda_program.h
    include/napalm/cuda/create.h
)

set(SRCS 
    src/cuda_context.cpp
    src/create.cpp
    src/cuda_buffer.cpp
    src/cuda_img.cpp
    src/cuda_program.cpp
    src/cuda_utils.h
    src/cuda_utils.cpp
    
)

add_library(napalm_CUDA ${PUBLIC_HDRS} ${SRCS})
add_library(::napalm_CUDA ALIAS napalm_CUDA)
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
	find_package(GLEW QUIET CONFIG)
	if (NOT GLEW_FOUND)
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
	 
	if (GLEW_FOUND)
	    target_link_libraries(${PROJECT_NAME} PUBLIC GLEW::GLEW)
	    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAVE_GLEW=1)
        set(gles2_FOUND FALSE)
	elseif(gles2_FOUND)
	    target_link_libraries(${PROJECT_NAME} PUBLIC ::gles2)
	else()
	    message(FATAL_ERROR "Could not find glew nor gles2")
	endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DGL_SHARING_ENABLED=1)
endif()


source_group(napalm_CUDA FILES ${PUBLIC_HDRS})

if (BUILD_SHARED_LIBS)
    target_link_libraries(${PROJECT_NAME} PRIVATE ::cuda)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC ::cuda)
endif()

if (CUDA_RUNTIME_COMPILE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAVE_NVRTC)
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/napalm/cuda/${PROJECT_NAME}_export.h)


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
    ${CMAKE_BINARY_DIR}/gen/napalm/cuda/${PROJECT_NAME}_export.h
	${PUBLIC_HDRS}
    DESTINATION include/napalm/${PROJECT_NAME}/napalm/cuda)



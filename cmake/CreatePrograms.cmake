
set(EMBED_FILE_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Directory containing template PROGRAM_FILES")
# TODO extension map to decide which file belongs which API and Binary or Source type, like ".cl:OpenCL:SOURCE;.cu:CUDA:SOURCE;.ptx:CUDA:BINARY"
function(create_programs PROGRAM_FILES_IN output target COMPILE_IN_BUILD_TYPE)

    set (EMBED_PROGRAM_FILES TRUE)
    #message(FATAL_ERROR ${ARGV4})
    if (DEFINED ARGV4)
        set (EMBED_PROGRAM_FILES ${ARGV4})
    endif()
    set (PROGRAM_RUNTIME_SOURCE_DIR "./")
    if (DEFINED ARGV5)
        set (PROGRAM_RUNTIME_SOURCE_DIR ${ARGV5})
        if(NOT IS_ABSOLUTE ${PROGRAM_RUNTIME_SOURCE_DIR})
            set(PROGRAM_RUNTIME_SOURCE_DIR_ABS "${CMAKE_BINARY_DIR}/${PROGRAM_RUNTIME_SOURCE_DIR}")
        else()
            set(PROGRAM_RUNTIME_SOURCE_DIR_ABS "${PROGRAM_RUNTIME_SOURCE_DIR}")
        endif()
        message(STATUS ${PROGRAM_RUNTIME_SOURCE_DIR})
    endif()


    target_sources(${target} PRIVATE ${PROGRAM_FILES_IN})
    source_group(compute_programs FILES ${PROGRAM_FILES_IN})
    set(PROGRAM_FILES)
    set(CU_PROGRAM_FILES)
    set(PROGRAM_NAMES)
    message(${PROGRAM_FILES_IN})
    foreach(f ${PROGRAM_FILES_IN})
        if (NOT IS_ABSOLUTE ${f})
            set(f ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        endif()
        get_filename_component(filename ${f} NAME)
        get_filename_component(filename_we ${f} NAME_WE)
        
        list(APPEND PROGRAM_NAMES ${filename_we})
        get_filename_component(f_ext ${filename} EXT)
        if (${f_ext} STREQUAL ".cu")
           list(APPEND CU_PROGRAM_FILES ${f})
           list(REMOVE_ITEM PROGRAM_FILES ${f})
        else ()
            list(APPEND PROGRAM_FILES ${f})
        endif()
    endforeach()

    set(output)
    list(REMOVE_DUPLICATES PROGRAM_NAMES)
    foreach(f ${PROGRAM_NAMES})
        list(APPEND output napalm/gen/programs/${f}.h napalm/gen/programs/${f}.cpp)
    endforeach()
    
    list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_50,code=sm_50 --include-path=${PROGRAM_RUNTIME_SOURCE_DIR}") 
    message(STATUS "CUDA NVCC FLAGS" ${CUDA_NVCC_FLAGS})
    set(CUDA_GENERATED_OUTPUT_DIR ${PROGRAM_RUNTIME_SOURCE_DIR_ABS})
    if (CUDA_FOUND AND ${COMPILE_IN_BUILD_TYPE})
        cuda_compile_ptx(CU_PROGRAM_FILES_PTXS ${CU_PROGRAM_FILES})
        list(APPEND PROGRAM_FILES ${CU_PROGRAM_FILES_PTXS})
    else()
        list(APPEND PROGRAM_FILES ${CU_PROGRAM_FILES})
    endif()
    
    set(program_template_h ${EMBED_FILE_TEMPLATE_DIR}/program_template.h.in)
    set(program_template_cpp ${EMBED_FILE_TEMPLATE_DIR}/program_template.cpp.in)
    
    set(CU_INCLUDE_PATH "${CUDA_INCLUDE_DIRS}")
    set (PROGRAM_FILES_DESCRIPTORS ".cl|OpenCL|SOURCE|\"-I \\\"${PROGRAM_RUNTIME_SOURCE_DIR}\\\"\"" 
        ".cu|CUDA|SOURCE|\"--gpu-architecture=compute_52+--include-path=\\\"${CU_INCLUDE_PATH}\\\"+--include-path=\\\"${PROGRAM_RUNTIME_SOURCE_DIR}\\\"\"" 
        ".cu.ptx|CUDA|BINARY|\"\"")
    if (DEFINED ARGV6)
        set (PROGRAM_FILES_DESCRIPTORS ${ARGV6})
    endif()
    message(STATUS "Program file descriptors: ${PROGRAM_FILES_DESCRIPTORS}")
    
    message(STATUS "EMBED_PROGRAM_FILES: ${EMBED_PROGRAM_FILES}")
    message(STATUS "PROGRAM_RUNTIME_SOURCE_DIR: ${PROGRAM_RUNTIME_SOURCE_DIR}")


    configure_file(${EMBED_FILE_TEMPLATE_DIR}/generate_file.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake"
        @ONLY
    )
    #message(STATUS ${EMBED_FILE_TEMPLATE_DIR}/generate_file.cmake.in)
    #message(STATUS ${PROGRAM_FILES})
    target_sources(${target} PRIVATE ${output})
    target_include_directories(${target} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
    set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
    source_group(napalm\\gen\\programs FILES ${output})
    
    if (EMBED_PROGRAM_FILES)
        add_custom_command(OUTPUT  ${output}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake
            DEPENDS ${PROGRAM_FILES} ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake)
    else()
        add_custom_command(OUTPUT  ${output}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake)
    endif()
        
        
    message(STATUS "PROGRAMS TO ADD: ${PROGRAM_FILES}")
    message(STATUS "PROGRAMS NAMES: ${PROGRAM_NAMES}")
endfunction()
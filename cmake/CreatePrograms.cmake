
set(EMBED_FILE_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Directory containing template PROGRAM_FILES")

function(create_programs PROGRAM_FILES_IN output target COMPILE_IN_BUILD_TYPE)
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
    
    if (CUDA_FOUND AND COMPILE_IN_BUILD_TYPE)
        cuda_compile_ptx(CU_PROGRAM_FILES_PTXS ${CU_PROGRAM_FILES})
        list(APPEND PROGRAM_FILES ${CU_PROGRAM_FILES_PTXS})
    else()
        list(APPEND PROGRAM_FILES ${CU_PROGRAM_FILES})
    endif()
    
    set(program_template_h ${EMBED_FILE_TEMPLATE_DIR}/program_template.h.in)
    set(program_template_cpp ${EMBED_FILE_TEMPLATE_DIR}/program_template.cpp.in)
    
    set (EMBED_PROGRAM_FILES TRUE)
    #message(FATAL_ERROR ${ARGV4})
    if (DEFINED ARGV4)
        set (EMBED_PROGRAM_FILES ${ARGV4})
    endif()
    set (PROGRAM_RUNTIME_SOURCE_DIR "./")
    if (DEFINED ARGV5)
        message(FATAL_ERROR "sdasdasd")
        set (EMBED_PROGRAM_FILES ${ARGV5})
    endif()
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
    
    add_custom_command(OUTPUT  ${output}
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake
        DEPENDS ${PROGRAM_FILES} ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake)
        
        
    message(STATUS "PROGRAMS TO ADD: ${PROGRAM_FILES}")
    message(STATUS "PROGRAMS NAMES: ${PROGRAM_NAMES}")
endfunction()
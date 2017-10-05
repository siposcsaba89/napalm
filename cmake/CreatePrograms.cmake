
set(EMBED_FILE_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Directory containing template files")

function(create_programs files_in output target)
    set(files "")
    set(cu_files)
    set(all_files_we)
    foreach(f ${files_in})
        if (NOT IS_ABSOLUTE ${f})
            set(f ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        endif()
        get_filename_component(filename ${f} NAME)
        get_filename_component(filename_we ${f} NAME_WE)
        list(APPEND all_files_we ${filename_we})
        get_filename_component(f_ext ${filename} EXT)
        if (${f_ext} STREQUAL ".cu")
           list(APPEND cu_files ${f})
           list(REMOVE_ITEM files ${f})
        else ()
            list(APPEND files ${f})
        endif()
    endforeach()

    set(output)
    list(REMOVE_DUPLICATES all_files_we)
    foreach(f ${all_files_we})
        list(APPEND output gen/programs/${f}.h gen/programs/${f}.cpp)
    endforeach()
    
    
    #message(STATUS ${cu_files})
    #message(STATUS ${files})
    
    if (CUDA_ENABLED AND NOT CUDA_RUNTIME_COMPILE)
        cuda_compile_ptx(cu_files_ptxs ${cu_files})
    endif()
    
    set(program_template_h ${EMBED_FILE_TEMPLATE_DIR}/program_template.h.in)
    set(program_template_cpp ${EMBED_FILE_TEMPLATE_DIR}/program_template.cpp.in)
    
    configure_file(${EMBED_FILE_TEMPLATE_DIR}/generate_file.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake"
        @ONLY
    )
    #message(STATUS ${EMBED_FILE_TEMPLATE_DIR}/generate_file.cmake.in)
    #message(STATUS ${files})
    target_sources(${target} PRIVATE ${output})
    target_include_directories(${target} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
    set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
    source_group(gen\\programs FILES ${output})
    
    add_custom_command(OUTPUT  ${output}
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake
        DEPENDS ${files} ${cu_files} ${cu_files_ptxs} ${CMAKE_CURRENT_BINARY_DIR}/generate_file_${target}.cmake)
endfunction()
# The following variables contains the files used by the different stages of the build process.
set(ModbusTCP_X_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${ModbusTCP_X_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${ModbusTCP_X_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(ModbusTCP_X_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${ModbusTCP_X_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${ModbusTCP_X_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(ModbusTCP_X_default_default_XC32_FILE_TYPE_compile "${CMAKE_CURRENT_SOURCE_DIR}/../../../ModbusTCP_X.X/ModbusTCP.c")
set_source_files_properties(${ModbusTCP_X_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(ModbusTCP_X_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${ModbusTCP_X_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(ModbusTCP_X_default_default_XC32_FILE_TYPE_archive)
set(CMAKE_STATIC_LIBRARY_PREFIX )
set(CMAKE_STATIC_LIBRARY_SUFFIX .elf)
set(ModbusTCP_X_default_image_name "default")
set(ModbusTCP_X_default_image_base_name "default")

# The output directory of the final image.
set(ModbusTCP_X_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/ModbusTCP_X")

# The full path to the final image.
set(ModbusTCP_X_default_full_path_to_image ${ModbusTCP_X_default_output_dir}/${ModbusTCP_X_default_image_name})

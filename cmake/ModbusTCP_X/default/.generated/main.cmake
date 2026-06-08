include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(ModbusTCP_X_default_library_list )

# Handle files with suffix s, for group default-XC32
if(ModbusTCP_X_default_default_XC32_FILE_TYPE_assemble)
add_library(ModbusTCP_X_default_default_XC32_assemble OBJECT ${ModbusTCP_X_default_default_XC32_FILE_TYPE_assemble})
    ModbusTCP_X_default_default_XC32_assemble_rule(ModbusTCP_X_default_default_XC32_assemble)
    list(APPEND ModbusTCP_X_default_library_list "$<TARGET_OBJECTS:ModbusTCP_X_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(ModbusTCP_X_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(ModbusTCP_X_default_default_XC32_assembleWithPreprocess OBJECT ${ModbusTCP_X_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    ModbusTCP_X_default_default_XC32_assembleWithPreprocess_rule(ModbusTCP_X_default_default_XC32_assembleWithPreprocess)
    list(APPEND ModbusTCP_X_default_library_list "$<TARGET_OBJECTS:ModbusTCP_X_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ModbusTCP_X_default_default_XC32_FILE_TYPE_compile)
add_library(ModbusTCP_X_default_default_XC32_compile OBJECT ${ModbusTCP_X_default_default_XC32_FILE_TYPE_compile})
    ModbusTCP_X_default_default_XC32_compile_rule(ModbusTCP_X_default_default_XC32_compile)
    list(APPEND ModbusTCP_X_default_library_list "$<TARGET_OBJECTS:ModbusTCP_X_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(ModbusTCP_X_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(ModbusTCP_X_default_default_XC32_compile_cpp OBJECT ${ModbusTCP_X_default_default_XC32_FILE_TYPE_compile_cpp})
    ModbusTCP_X_default_default_XC32_compile_cpp_rule(ModbusTCP_X_default_default_XC32_compile_cpp)
    list(APPEND ModbusTCP_X_default_library_list "$<TARGET_OBJECTS:ModbusTCP_X_default_default_XC32_compile_cpp>")

endif()

add_library(
    ModbusTCP_X_default_image_UCbtPTJB
    ${ModbusTCP_X_default_library_list})
set_target_properties(ModbusTCP_X_default_image_UCbtPTJB PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    ARCHIVE_OUTPUT_DIRECTORY "${ModbusTCP_X_default_output_dir}")
foreach(lib ${ModbusTCP_X_default_FILE_TYPE__link})
    target_link_libraries(ModbusTCP_X_default_image_UCbtPTJB
    PRIVATE
     ${lib})
endforeach()
# Add the archiver options from the rule file.
ModbusTCP_X_default_archiver_rule( ModbusTCP_X_default_image_UCbtPTJB)



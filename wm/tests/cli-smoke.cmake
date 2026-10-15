execute_process(COMMAND "${CORE}" --version RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR NOT output STREQUAL "hyprv4-core 0.1.0\n")
  message(FATAL_ERROR "version smoke failed: ${result} ${output} ${error}")
endif()

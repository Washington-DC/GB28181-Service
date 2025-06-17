#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CURL::libcurl" for configuration "Debug"
set_property(TARGET CURL::libcurl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(CURL::libcurl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libcurl-d.lib"
  )

list(APPEND _cmake_import_check_targets CURL::libcurl )
list(APPEND _cmake_import_check_files_for_CURL::libcurl "${_IMPORT_PREFIX}/lib/libcurl-d.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

# Find the source include for quickfix
find_path(QUICKFIX_INCLUDE_PATH quickfix /usr/local/include)
if (NOT QUICKFIX_INCLUDE_PATH)
    message(FATAL_ERROR "quickfix not found in /usr/local/include")
endif ()

# Find the installed dynamic lib for quickfix
find_library(QUICKFIX_DYLIB libquickfix.dylib /usr/local/lib)
if (NOT QUICKFIX_DYLIB)
    message(FATAL_ERROR "libquickfix.dylib not found in /usr/local/lib")
endif ()

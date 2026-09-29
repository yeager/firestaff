cmake_minimum_required(VERSION 3.21)

# Resolve the PE import tree, including SDL3's transitive MinGW dependencies.
# A package must fail here rather than ship an executable with a missing DLL.
foreach(required FIRESTAFF_EXE RUNTIME_DIR STAGE_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")
set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
find_program(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND NAMES objdump REQUIRED)
file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${FIRESTAFF_EXE}"
    DIRECTORIES "${RUNTIME_DIR}"
    PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
    POST_EXCLUDE_REGEXES ".*[/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\].*"
    RESOLVED_DEPENDENCIES_VAR runtime_dlls
    UNRESOLVED_DEPENDENCIES_VAR missing_dlls
)
if(missing_dlls)
    message(FATAL_ERROR "Unresolved Windows runtime DLLs: ${missing_dlls}")
endif()
file(MAKE_DIRECTORY "${STAGE_DIR}")
foreach(dll IN LISTS runtime_dlls)
    file(COPY "${dll}" DESTINATION "${STAGE_DIR}")
endforeach()

cmake_minimum_required(VERSION 3.16)
project(csb_startup_session_contract C)

enable_testing()

add_executable(test_csb_v1_startup_session_contract_pc34_compat
    ../test_csb_v1_startup_session_contract_pc34_compat.c
    ../../src/csb/csb_v1_startup_sequence_pc34_compat.c
    ../../src/csb/csb_v1_startup_session_contract_pc34_compat.c
)
target_include_directories(test_csb_v1_startup_session_contract_pc34_compat PRIVATE
    ../../include
)
target_compile_features(test_csb_v1_startup_session_contract_pc34_compat PRIVATE c_std_11)
if(NOT MSVC)
    target_compile_options(test_csb_v1_startup_session_contract_pc34_compat PRIVATE
        -Wall -Wextra -O2)
endif()
add_test(
    NAME csb_v1_startup_session_contract_pc34_compat
    COMMAND test_csb_v1_startup_session_contract_pc34_compat
)

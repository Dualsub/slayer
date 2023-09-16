cmake_minimum_required(VERSION 3.0.0)
project(slayer_bindings VERSION 0.1.0)

add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/pybind11)

pybind11_add_module(slayer_bindings module.cpp)

target_link_libraries(slayer_bindings PRIVATE assimp Slayer)

# Copy the Python module to the main directory
add_custom_command(TARGET slayer_bindings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:slayer_bindings>
    ${CMAKE_SOURCE_DIR}/Tools/slayer_bindings.pyd)

# MIT License

# Copyright (c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(client VERSION 1.0.0 DESCRIPTION "Client for NovusCore-Client")

file(GLOB_RECURSE CLIENT_FILES "*.cpp" "*.h")
set(CLIENT_DEPENDENCIES
    "Dependencies/amy"
    "Dependencies/json"
    "Dependencies/robin-hood-hashing"
    "${ZLIB_INCLUDE_DIRS}"
    "${CMAKE_SOURCE_DIR}/dep/asio"
    "${CMAKE_SOURCE_DIR}/dep/angelscript/include"
)

add_executable(client ${CLIENT_FILES})
set_property(TARGET client PROPERTY CXX_STANDARD 17)
find_assign_files(${CLIENT_FILES})

# Set VERSION & FOLDER Property
set_target_properties(client PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(client PROPERTIES FOLDER "server")

add_compile_definitions(NOMINMAX _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_include_directories(client PRIVATE ${CLIENT_DEPENDENCIES})
target_link_libraries(client asio openssl zlib angelscript)
install(TARGETS client DESTINATION bin)
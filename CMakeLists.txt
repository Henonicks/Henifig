##########################################################################
# Copyright 2025 Ramskyi Roman
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "henifig")

project(${PROJECT_NAME})

aux_source_directory("src" henifig_src)
add_library(${PROJECT_NAME} ${henifig_src})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set(CMAKE_CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_compile_definitions(${PROJECT_NAME} PUBLIC FMT_HEADER_ONLY)

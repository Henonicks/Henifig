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

find_path(HENIFIG_INCLUDE_DIR NAMES henifig/henifig.hpp HINTS "../include")

find_library(HENIFIG_LIBRARIES NAMES henifig "libhenifig" HINTS "../build")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Henifig DEFAULT_MSG HENIFIG_INCLUDE_DIR HENIFIG_LIBRARIES)

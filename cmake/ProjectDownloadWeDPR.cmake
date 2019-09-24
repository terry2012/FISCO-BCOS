#------------------------------------------------------------------------------
# install json cpp from git
# @ ${JSONCPP_INCLUDE_DIR}: jsoncpp include path
# @ ${JSONCPP_LIBRARY}: jsoncpp libbrary path
# ------------------------------------------------------------------------------
# This file is part of FISCO-BCOS.
#
# FISCO-BCOS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FISCO-BCOS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016-2019 fisco-dev contributors.
#------------------------------------------------------------------------------
include(ExternalProject)

if (APPLE)
    set(WEDPR_LIB_URL https://github.com/fqliao/LargeFiles/raw/master/libs/libffi_hidden_asset.tar.gz)
    set(WEDPR_LIB_SHA256 1a9f2d1ac827066d4f73a901a0bc4618299f2de463da79fb3a474e484787bc8b)
else()
    set(WEDPR_LIB_URL https://github.com/fqliao/LargeFiles/raw/master/libs/libffi_hidden_asset.tar.gz)
    set(WEDPR_LIB_SHA256 1a9f2d1ac827066d4f73a901a0bc4618299f2de463da79fb3a474e484787bc8b)
endif()

ExternalProject_Add(WEDPRLIB
    PREFIX ${CMAKE_SOURCE_DIR}/deps
    DOWNLOAD_NAME libffi_hidden_asset.tar.gz
    DOWNLOAD_NO_PROGRESS 1
    BUILD_IN_SOURCE 1
    URL ${WEDPR_LIB_URL}
    URL_HASH SHA256=${WEDPR_LIB_SHA256}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND bash -c "cp ${CMAKE_SOURCE_DIR}/deps/src/WEDPRLIB/libffi_hidden_asset.so ${CMAKE_SOURCE_DIR}/deps/lib/"
)
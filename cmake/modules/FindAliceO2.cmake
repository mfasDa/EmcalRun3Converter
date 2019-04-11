# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http:#alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.


# - Try to find the O2 framework package include dirs and libraries
# Author: Barthelemy von Haller
#
# This script will set the following variables:
#  AliceO2_FOUND - System has AliceO2
#  AliceO2_INCLUDE_DIRS - The AliceO2 include directories
#  AliceO2_LIBRARIES - The libraries needed to use AliceO2
#  AliceO2_DEFINITIONS - Compiler switches required for using AliceO2

# Init
include(FindPackageHandleStandardArgs)

# find includes
find_path(AliceO2_INCLUDE_DIR runDataProcessing.h
        HINTS ${O2_ROOT}/include ENV LD_LIBRARY_PATH PATH_SUFFIXES "../include/Framework" "../../include/Framework")

# Remove the final "Headers"
get_filename_component(AliceO2_INCLUDE_DIR ${AliceO2_INCLUDE_DIR} DIRECTORY)
set(AliceO2_INCLUDE_DIRS ${AliceO2_INCLUDE_DIR})

# find libraries
find_library(AliceO2_LIBRARY_EMCALBASE NAMES EMCALBase HINTS ${O2_ROOT}/lib ENV LD_LIBRARY_PATH)

set(AliceO2_LIBRARIES ${AliceO2_LIBRARY_EMCALBASE})
# TODO SEARCH *ALL* LIBRARIES --> AliceO2 should ideally provide the list !!!

# handle the QUIETLY and REQUIRED arguments and set AliceO2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(AliceO2  "AliceO2 could not be found. Install package AliceO2." AliceO2_LIBRARY_EMCALBASE AliceO2_INCLUDE_DIR)

if(${AliceO2_FOUND})
    message(STATUS "AliceO2 found, libraries: ${AliceO2_LIBRARIES}")

    mark_as_advanced(AliceO2_INCLUDE_DIRS AliceO2_LIBRARIES)

    # add target
    if(NOT TARGET AliceO2::AliceO2)
        add_library(AliceO2::AliceO2 INTERFACE IMPORTED)
        set_target_properties(AliceO2::AliceO2 PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${AliceO2_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${AliceO2_LIBRARIES}"
          )
    endif()
endif()


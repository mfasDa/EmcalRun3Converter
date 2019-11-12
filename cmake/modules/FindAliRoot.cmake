# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http:#alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.

include(FindPackageHandleStandardArgs)

find_program(ALIROOT_EXE NAMES aliroot PATHS ${ALICE_ROOT}/bin NO_DEFAULT_PATH)

find_path(AliRoot_LIBRARY_DIR libANALYSISalice.so
        HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
message(STATUS "AliRoot_LIBRARY_DIR: ${AliRoot_LIBRARY_DIR}")

get_filename_component(ALIROOT_DIR ${AliRoot_LIBRARY_DIR} DIRECTORY)
set(AliRoot_INCLUDE_DIR ${ALIROOT_DIR}/include)
message(STATUS "AliRoot_INCLUDE_DIR: ${AliRoot_INCLUDE_DIR}")

set(AliRoot_INCLUDE_DIRS ${AliRoot_INCLUDE_DIR})
set(AliRoot_LIBRARY_DIRS ${AliRoot_LIBRARY_DIR})

find_library(AliRoot_LIBRARY_ANALYSIS NAMES ANALYSIS HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_ANALYSISALICE NAMES ANALYSISalice HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_STEER NAMES STEER HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_STEERBASE NAMES STEERBase HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_ESD NAMES ESD HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_AOD NAMES AOD HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_RAWDatabase NAMES RAWDatabase HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)
find_library(AliRoot_LIBRARY_RAWDatarec NAMES RAWDatarec HINTS ${ALICE_ROOT}/lib ENV LD_LIBRARY_PATH)

set(AliRoot_LIBRARIES ${AliRoot_LIBRARY_ANALYSIS} ${AliRoot_LIBRARY_ANALYSISALICE} ${AliRoot_LIBRARY_STEER} ${AliRoot_LIBRARY_STEERBASE} ${AliRoot_LIBRARY_ESD} ${AliRoot_LIBRARY_AOD} ${AliRoot_LIBRARY_RAWDatabase} ${AliRoot_LIBRARY_RAWDatarec})

find_package_handle_standard_args(AliRoot  "AliRoot could not be found. Install package AliRoot." AliRoot_LIBRARIES AliRoot_LIBRARY_DIRS AliRoot_INCLUDE_DIR)

if(${AliRoot_FOUND})
    message(STATUS "AliRoot found, libraries: ${AliRoot_LIBRARIES}")

    mark_as_advanced(AliRoot_INCLUDE_DIRS AliRoot_LIBRARIES AliRoot_LIBRARY_DIRS)

    # add target
    if(NOT TARGET AliRoot::AliRoot)
        add_library(AliRoot::AliRoot INTERFACE IMPORTED)
        set_target_properties(AliRoot::AliRoot PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${AliRoot_INCLUDE_DIRS}"
          INTERFACE_LINK_LIBRARIES "${AliRoot_LIBRARIES}"
          INTERFACE_LINK_DIRECTORIES "${AliRoot_LIBRARY_DIRS}"
          )
    endif()
else()
  if(AliRoot_FIND_REQUIRED)
    message(FATAL_ERROR "Please point to the AliRoot Core installation using -DALIROOT=<ALIROOT_CORE_INSTALL_DIR>")
  endif(AliRoot_FIND_REQUIRED)
endif()

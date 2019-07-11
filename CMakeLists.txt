# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http:#alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.
project(EmcalRun3Converter CXX)
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

set(CMAKE_INSTALL_LIBDIR lib)

# Set the default build type to "RelWithDebInfo"
if(NOT CMAKE_BUILD_TYPE)
  set(
    CMAKE_BUILD_TYPE
    "RelWithDebInfo"
    CACHE
      STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel Coverage."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_SHARED_LIBS "Build shared libs" ON)

# Build targets with install rpath on Mac to dramatically speed up installation
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
          isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
  endif()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
unset(isSystemDir)

find_package(
    Boost 1.68
    COMPONENTS
			serialization
			program_options
  )

find_package(ROOT 6.16.00 REQUIRED)
find_package(AliRoot REQUIRED)
find_package(O2 MODULE REQUIRED)

include(CheckCXXCompilerFlag)

# ROOT dictionaries and maps
include(ROOTHelper)

#Determine ROOT c++ standard
execute_process(COMMAND root-config --has-cxx11 OUTPUT_VARIABLE ROOT_HAS_C++11)
string(REGEX REPLACE "\n$" "" ROOT_HAS_C++11 "${ROOT_HAS_C++11}")
execute_process(COMMAND root-config --has-cxx14 OUTPUT_VARIABLE ROOT_HAS_C++14)
string(REGEX REPLACE "\n$" "" ROOT_HAS_C++14 "${ROOT_HAS_C++14}")
execute_process(COMMAND root-config --has-cxx17 OUTPUT_VARIABLE ROOT_HAS_C++17)
string(REGEX REPLACE "\n$" "" ROOT_HAS_C++17 "${ROOT_HAS_C++17}")
MESSAGE(STATUS "C++11: ${ROOT_HAS_C++11}")
MESSAGE(STATUS "C++14: ${ROOT_HAS_C++14}")
MESSAGE(STATUS "C++17: ${ROOT_HAS_C++17}")
if(ROOT_HAS_C++11 STREQUAL "yes")
	MESSAGE(STATUS "ROOT compiled with C++11 standard")
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(COMPILER_SUPPORTS_CXX0X)
    		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
    		message(FATAL_ERROR "C++11 support required.")
	endif()
elseif(ROOT_HAS_C++14 STREQUAL "yes")
	MESSAGE(STATUS "ROOT compiled with C++14 standard")
	CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
	if(COMPILER_SUPPORTS_CXX14)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	elseif(COMPILER_SUPPORTS_CXX1Y)
    		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	else()
    		message(FATAL_ERROR "C++14 support required.")
	endif()
elseif(ROOT_HAS_C++17 STREQUAL "yes")
	MESSAGE(STATUS "ROOT compiled with C++17 standard")
	CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
	CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
	if(COMPILER_SUPPORTS_CXX17)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	elseif(COMPILER_SUPPORTS_CXX1Z)
    		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
	else()
    		message(FATAL_ERROR "C++17 support required.")
	endif()
endif()

# Add libc++ in case of clang
message (STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_CXX_COMPILER_ID  STREQUAL Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

add_subdirectory(EMCALRun3Converter)

install (DIRECTORY macros DESTINATION share)
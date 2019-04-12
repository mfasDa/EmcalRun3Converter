# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http:#alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.

set(MODULE EmcalRun3Converter)

get_target_property(ALICEO2_INCLUDE_DIR AliceO2::AliceO2 INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ALIROOT_INCLUDE_DIR AliRoot::AliRoot INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ALICEO2_LIBRARIES AliceO2::AliceO2 INTERFACE_LINK_LIBRARIES)
get_target_property(ALIROOT_LIBRARIES AliRoot::AliRoot INTERFACE_LINK_LIBRARIES)
get_target_property(boost_inc_dir Boost::serialization INTERFACE_INCLUDE_DIRECTORIES)

include_directories(
    ${ROOT_INCLUDE_DIR}
    ${ALIROOT_INCLUDE_DIR}
    ${ALICEO2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
link_directories(
    ${ROOT_LIBDIR}
)

set(SRCS
    AliAnalysisTaskEmcalRun3ConverterDigits.cxx
)

# Headers from sources
string(REPLACE ".cxx" ".h" HDRS "${SRCS}")

set(ROOT_DEPENDENCIES Core EG Geom Gpad Graf Hist MathCore Matrix Physics RIO Tree VMC)

get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary("${MODULE}" "${MODULE}LinkDef.h" "${HDRS}" "${incdirs}")

# Create an object to be reused in case of static libraries 
# Otherwise the sources will be compiled twice
add_library(${MODULE} SHARED ${SRCS} ${HDRS} ${MODULE}LinkDef.h G__${MODULE}.cxx)

target_link_libraries(${MODULE} ${ROOT_DEPENDENCIES} ${ALICEO2_LIBRARIES} ${ALIROOT_LIBRARIES} Boost::serialization)

install(TARGETS ${MODULE}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
install(FILES ${HDRS} DESTINATION include)

add_executable(runEmcalRun3DigitsConverter runEmcalRun3DigitsConverter.cxx )
target_link_libraries(runEmcalRun3DigitsConverter ${MODULE} ${ROOT_DEPENDENCIES} ${ALICEO2_LIBRARIES} ${ALIROOT_LIBRARIES} Boost::program_options)
install(TARGETS runEmcalRun3DigitsConverter
    RUNTIME DESTINATION bin
)
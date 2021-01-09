# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http:#alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.

add_library(EMCALRun3Converter)

target_sources(EMCALRun3Converter PRIVATE src/AliAnalysisTaskEmcalRun3ConverterDigits.cxx
                                          src/AliAnalysisTaskEmcalRun3ConverterCells.cxx
                                          src/AliEmcalRawConverter.cxx
                                          src/AliEmcalCalibDBConverter.cxx
                                          )

target_include_directories(
  EMCALRun3Converter
  PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(EMCALRun3Converter PUBLIC O2::DataFormatsEMCAL O2::EMCALSimulation O2::EMCALCalib O2::Headers O2::DetectorsRaw O2::CCDB O2::CommonConstants AliRoot::AliRoot Boost::serialization)

add_root_dictionary(EMCALRun3Converter
                    HEADERS 
                            include/EMCALRun3Converter/AliAnalysisTaskEmcalRun3ConverterDigits.h
                            include/EMCALRun3Converter/AliAnalysisTaskEmcalRun3ConverterCells.h
                            include/EMCALRun3Converter/AliEmcalRawConverter.h
                    LINKDEF 
                            include/EMCALRun3Converter/LinkDef.h
                    )

install(TARGETS EMCALRun3Converter
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#set(ROOT_DEPENDENCIES Core EG Geom Gpad Graf Hist MathCore Matrix Physics RIO Tree VMC)

add_executable(runEmcalRun3DigitsConverter src/runEmcalRun3DigitsConverter.cxx )
target_link_libraries(runEmcalRun3DigitsConverter EMCALRun3Converter O2::EMCALBase AliRoot::AliRoot Boost::program_options)
install(TARGETS runEmcalRun3DigitsConverter
    RUNTIME DESTINATION bin
)

add_executable(runEmcalRun3CellsConverter src/runEmcalRun3CellsConverter.cxx )
target_link_libraries(runEmcalRun3CellsConverter EMCALRun3Converter O2::EMCALBase AliRoot::AliRoot Boost::program_options)
install(TARGETS runEmcalRun3CellsConverter
    RUNTIME DESTINATION bin
)

add_executable(runEmcalRun3RawConverter src/runEmcalRun3RawConverter.cxx)
target_link_libraries(runEmcalRun3RawConverter EMCALRun3Converter O2::EMCALBase O2::EMCALSimulation O2::Headers AliRoot::AliRoot Boost::program_options)
install(TARGETS runEmcalRun3RawConverter
    RUNTIME DESTINATION bin
)

add_executable(runEmcalRun3ConverterCalib src/runEmcalRun3ConverterCalib.cxx)
target_link_libraries(runEmcalRun3ConverterCalib EMCALRun3Converter O2::EMCALCalib O2::CCDB AliRoot::AliRoot Boost::program_options)
install(TARGETS runEmcalRun3ConverterCalib
    RUNTIME DESTINATION bin
)
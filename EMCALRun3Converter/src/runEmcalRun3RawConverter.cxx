// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction
#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "EMCALRun3Converter/AliEmcalRawConverter.h"

int main(int argc, const char** argv)
{
  std::string inputfile, outputdir, segmentation;
  long starttime = -1;
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
            ("help", "produce help message")
            ("in", boost::program_options::value<std::string>(&inputfile)->default_value("raw.root"), "inputfile to be processed")
            ("out", boost::program_options::value<std::string>(&outputdir)->default_value(""), "output directory for converted raw files")
            ("segmentation", boost::program_options::value<std::string>(&segmentation)->default_value("emcal"), "output file segmentation (emcal, subdet, link)")
            ("starttime", boost::program_options::value<long>(&starttime)->default_value(-1), "fake run3 timestamp");
  boost::program_options::variables_map optionmap;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), optionmap);
  boost::program_options::notify(optionmap);

  if (optionmap.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }
  if (optionmap.count("in")) {
    inputfile = optionmap["in"].as<std::string>();
  }
  if (optionmap.count("out")) {
    outputdir = optionmap["out"].as<std::string>();
  }
  if (optionmap.count("segmentation")) {
    segmentation = optionmap["segmentation"].as<std::string>();
  }
  if (optionmap.count("starttime")) {
    starttime = optionmap["starttime"].as<long>();
  }

  if (!inputfile.length()) {
    std::cerr << "Please provide a vaild input file\n";
    return EXIT_FAILURE;
  }

  if (!outputdir.length()) {
    std::cerr << "Please provide a valid output directory";
    return EXIT_FAILURE;
  }

  o2::emcal::AliEmcalRawConverter::FileFor_t filefor = o2::emcal::AliEmcalRawConverter::FileFor_t::kFullDet;
  if(segmentation.length()) {
    bool hasSegmentation = false;
    if(segmentation == "emcal") {
      std::cout << "Creating a single raw file for the full EMCAL" << std::endl;
      filefor = o2::emcal::AliEmcalRawConverter::FileFor_t::kFullDet; 
      hasSegmentation = true;
    } else if(segmentation == "subdet") {
      std::cout << "Creating one raw file for per subdetector (EMCAL/DCAL)" << std::endl;
      filefor = o2::emcal::AliEmcalRawConverter::FileFor_t::kSubDet;  
      hasSegmentation = true;
    } else if(segmentation == "link") {
      std::cout << "Creating one raw file for per DDL link" << std::endl;
      filefor = o2::emcal::AliEmcalRawConverter::FileFor_t::kLink;  
      hasSegmentation = true;
    }
    if(!hasSegmentation) {
      std::cerr << "Unknown segmentation, select either emcal, subdet or link" << std::endl;
      return EXIT_FAILURE;
    }
  }

  o2::emcal::AliEmcalRawConverter rawconverter(inputfile, outputdir);
  if(starttime >= 0){
    std::cout << "Using start timestamp " << starttime << std::endl;
    rawconverter.setStartTimeRun(starttime);
  } else {
    std::cout << "Using BC and orbit from run2 interaction record" << std::endl;
  }
  rawconverter.setFileFor(filefor);
  rawconverter.convert();
  return EXIT_SUCCESS;
}
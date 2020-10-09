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
#include "EMCALRun3Converter/AliEmcalCalibDBConverter.h"

int main(int argc, const char** argv)
{
  std::string url;
  long timestamp(-1);
  int timeshift(0), runnumber(0);
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")("run", boost::program_options::value<int>(&runnumber)->default_value(0), "Run number to be converted")("timestamp", boost::program_options::value<long>(&timestamp)->default_value(-1), "Output timestamp in CCDB")("timeshift", boost::program_options::value<int>(&timeshift)->default_value(0), "Time shift to be applied in the converion of the time calib params");
  ("url", boost::program_options::value<std::string>(&url)->default_value("emcccdb-test.cern.ch:8080"), "inputfile to be produced with o2 digits");
  boost::program_options::variables_map optionmap;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), optionmap);
  boost::program_options::notify(optionmap);

  if (optionmap.count("help")) {
    std::cout << desc << std::endl;
  }
  if (optionmap.count("run")) {
    runnumber = optionmap["run"].as<int>();
  }
  if (optionmap.count("timestamp")) {
    timestamp = optionmap["timestamp"].as<long>();
  }
  if (optionmap.count("timeshift")) {
    timeshift = optionmap["timeshift"].as<int>();
  }
  if (optionmap.count("url")) {
    url = optionmap["url"].as<std::string>();
  }

  if (!runnumber) {
    std::cerr << "Please provide a vaild run number file\n";
    return EXIT_FAILURE;
  }

  std::cout << "Running calib converter ..." << std::endl
            << "====================================================================" << std::endl
            << "Run number:                       " << runnumber << std::endl
            << "Timestamp in CCDB:                " << timestamp << std::endl
            << "Time shift in time calib (ns):    " << timeshift << std::endl
            << "CCDB URL:                         " << url << std::endl
            << "====================================================================" << std::endl;

  o2::emcal::AliEmcalCalibDBConverter calibconverter(url.data());
  calibconverter.setTimeOffset(timeshift);
  calibconverter.convertAndStore(runnumber, timestamp);
  return EXIT_SUCCESS;
}
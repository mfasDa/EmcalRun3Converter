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

int main(int argc, const char **argv) {
    std::string inputfile, outputfile;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("in", boost::program_options::value<std::string>(&inputfile)->default_value("raw.root"), "inputfile to be processed") 
        ("out", boost::program_options::value<std::string>(&outputfile)->default_value("emcal.raw"), "inputfile to be produced with o2 digits");
    boost::program_options::variables_map optionmap;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), optionmap);
    boost::program_options::notify(optionmap);

    if(optionmap.count("help")){
        std::cout << desc << std::endl;
    }
    if(optionmap.count("in")) {
        inputfile = optionmap["in"].as<std::string>();
    }
    if(optionmap.count("out")) {
        outputfile = optionmap["out"].as<std::string>();
    }

    if(!inputfile.length()) {
        std::cerr << "Please provide a vaild input file\n";
        return EXIT_FAILURE;   
    }

    if(!outputfile.length()){
        std::cerr << "Please provide a valid output file";
        return EXIT_FAILURE;
    }

    o2::emcal::AliEmcalRawConverter rawconverter(inputfile, outputfile); 
    rawconverter.convert();
    return EXIT_SUCCESS;
}
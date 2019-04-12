#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>
#include <TFile.h>
#include <TTree.h>
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliESDInputHandler.h"
#include "AliAnalysisTaskEmcalRun3ConverterDigits.h"

int main(int argc, const char **argv) {
    std::string inputfile, outputfile, trigger;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("in", boost::program_options::value<std::string>(), "inputfile to be processed") 
        ("out", boost::program_options::value<std::string>(&outputfile)->default_value("o2sim.root"), "inputfile to be produced with o2 digits") 
        ("trigger", boost::program_options::value<std::string>(&trigger)->default_value("INT7"), "Trigger selection");
    ;
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
    if(optionmap.count("trigger")) {
        trigger = optionmap["trigger"].as<std::string>();
    }

    if(!inputfile.length()){
        std::cerr << "No inputfile provided" << std::endl;
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }

    bool isESD(false), isAOD(false);
    AliInputEventHandler *inputhandler(nullptr);
    std::string treename;
    if(inputfile.find("AOD") != std::string::npos) {
        isAOD = true;
        inputhandler = new AliAODInputHandler;
        treename = "aodTree";
    }
    if(inputfile.find("ESD") != std::string::npos) {
        isESD = true;
        inputhandler = new AliESDInputHandler;
        treename = "esdTree";
    }

    if(!(isESD || isAOD)) {
        std::cerr << "Inputfile neither ESD nor AOD file - please provide proper input file!" << std::endl;
        return EXIT_FAILURE;
    }

    AliAnalysisManager *mgr = new AliAnalysisManager;
    mgr->SetInputEventHandler(inputhandler);

    auto task = o2::emc::AliAnalysisTaskEmcalRun3ConverterDigits::AddTaskEmcalRun3ConverterDigits("digitsConverter", outputfile.data());
    task->SetTrigger(trigger.data());

    std::unique_ptr<TFile> reader(TFile::Open(inputfile.data(), "READ"));
    TTree *inputtree = dynamic_cast<TTree *>(reader->Get(treename.data()));
    if(!inputtree) {
        std::cerr << "Inputtree " << treename << " not found in inputfile " << inputfile << " ..." << std::endl;
        return EXIT_FAILURE;
    }
    
    if(mgr->InitAnalysis()) {
        mgr->PrintStatus();
        mgr->StartAnalysis("local", inputtree);   
    }
    std::cout << "Done ..." << std::endl;
    return EXIT_SUCCESS;
}
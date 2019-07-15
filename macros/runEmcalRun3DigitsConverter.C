R__ADD_INCLUDE_PATH($ALICE_ROOT)
#include "ANALYSIS/macros/train/AddAODHandler.C"
#include "ANALYSIS/macros/train/AddESDHandler.C"

#ifndef __CLING__
#include <fstream>
#include <string>
#include "AliAnalysisManager.h"
#include "TChain.h"
#include "EMCALRun3Converter/AliAnalysisTaskEmcalRun3ConverterDigits.h"
#endif

TChain *createInputChain(const char *filelist, const char *treename) {
    TChain *result = new TChain(treename, "");
    std::ifstream filestream(filelist);
    std::string filename;
    while(std::getline(filestream, filename)) {
	    std::cout << "Adding file " << filename << std::endl;
	    result->AddFile(filename.data());
    }
    return result;
}

void runEmcalRun3DigitsConverter(const char *filelist, const char *trigger, bool aod = true){
    AliAnalysisManager *mgr = new AliAnalysisManager;
    if(aod) AddAODHandler();
    else AddESDHandler();

    auto convertertask = o2::emcal::AliAnalysisTaskEmcalRun3ConverterDigits::AddTaskEmcalRun3ConverterDigits("convertertask");
    convertertask->SetTrigger(trigger);

    std::string treename = aod ? "aodTree" : "esdTree";
    auto inputtree = createInputChain(filelist, treename.data());
    if(mgr->InitAnalysis()) {
        mgr->PrintStatus();
        mgr->StartAnalysis("local", inputtree);   
    }
    std::cout << "Done ..." << std::endl;
}

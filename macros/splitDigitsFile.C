#if !defined(__CLING__)
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <TFile.h>
#include <TSystem.h>
#include <TTreeReader.h>
#include <TTree.h>

#include "DataFormatsEMCAL/Digit.h"
#include "DataFormatsEMCAL/TriggerRecord.h"
#endif

void splitDigitsFile(const char *inputfile = "emcal.digits.root", int ntimeframesFile = 1000) {
    const char *digitsBranchName = "EMCALDigit",
               *triggerBranchName = "EMCALDigitTRGR";
    std::unique_ptr<TFile> reader(TFile::Open(inputfile, "READ"));
    TTreeReader treereader(static_cast<TTree *>(reader->Get("o2sim")));
    TTreeReaderValue<std::vector<o2::emcal::Digit>> digitsin(treereader, digitsBranchName);
    TTreeReaderValue<std::vector<o2::emcal::TriggerRecord>> triggersin(treereader, triggerBranchName);

    // file information
    std::string dirname = gSystem->DirName(inputfile), 
                basename = gSystem->BaseName(inputfile), 
                filebase = basename.substr(0, basename.find(".root"));

    // output objects
    std::unique_ptr<TFile> writer;
    TTree *outputtree = nullptr;
    std::vector<o2::emcal::Digit> digitsout;
    std::vector<o2::emcal::TriggerRecord> triggersout;
    int ntimeframes = 0;
    int ndigitsfile = 0;
    std::cout << "Timeframes per file: " << ntimeframesFile << std::endl;
    while(treereader.Next()){
        if(!writer || ntimeframes % ntimeframesFile == 0){
            std::string outfilename(Form("%s/%s.%d.root", dirname.data(), filebase.data(), ndigitsfile)); 
            std::cout << "Timeframe " << ntimeframes << ", writing to file " << outfilename << std::endl;
            writer = std::unique_ptr<TFile>(TFile::Open(outfilename.data(), "RECREATE"));
            writer->cd();
            outputtree = new TTree(treereader.GetTree()->GetName(), treereader.GetTree()->GetTitle());
            outputtree->Branch(digitsBranchName, &digitsout);
            outputtree->Branch(triggerBranchName, &triggersout);
            outputtree->Write();
            ndigitsfile++;
        }
        digitsout = *digitsin;
        triggersout = *triggersin;
        ntimeframes++;
    }
    std::cout << "Processed " << ntimeframes << " timeframes" << std::endl;
}
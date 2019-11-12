// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#include <iostream>
#include "TTree.h"
#include "TString.h"
#include "AliAnalysisManager.h"
#include "EMCALRun3Converter/AliAnalysisTaskEmcalRun3ConverterCells.h"
#include "DataFormatsEMCAL/Constants.h"
#include "AliInputEventHandler.h"
#include "AliLog.h"
#include "AliVCaloCells.h"
#include "AliVEvent.h"

ClassImp(o2::emcal::AliAnalysisTaskEmcalRun3ConverterCells)

using namespace o2::emcal;

AliAnalysisTaskEmcalRun3ConverterCells::AliAnalysisTaskEmcalRun3ConverterCells(const char *name) :
    AliAnalysisTaskSE(name),
    fO2simtree(nullptr),
    fCellContainer(nullptr),
    fTrigger("INT7"),
    fTriggerBits(-1)
{
    DefineOutput(1, TTree::Class());
}

void AliAnalysisTaskEmcalRun3ConverterCells::UserCreateOutputObjects(){
    fCellContainer = new std::vector<o2::emcal::Cell>;

    if(fTrigger.find("INT7") != std::string::npos) {
        fTriggerBits = AliVEvent::kINT7;
    } else if(fTrigger.find("EG1") != std::string::npos || 
              fTrigger.find("EG2") != std::string::npos ||
              fTrigger.find("DG1") != std::string::npos ||
              fTrigger.find("DG2") != std::string::npos) {
        fTriggerBits = AliVEvent::kEMCEGA;
    } else if(fTrigger.find("EJ1") != std::string::npos || 
              fTrigger.find("EJ2") != std::string::npos ||
              fTrigger.find("DJ1") != std::string::npos ||
              fTrigger.find("DJ2") != std::string::npos) {
        fTriggerBits = AliVEvent::kEMCEJE;
    }

    OpenFile(1);
    fO2simtree = new TTree("o2sim", "o2sim");
    fO2simtree->Branch("EMCALCell", &fCellContainer);

    PostData(1, fO2simtree);
}

void AliAnalysisTaskEmcalRun3ConverterCells::UserExec(Option_t *){
    const double SECONDSTONANOSECONDS = 1e9;
    fCellContainer->clear();
    if(!(fInputHandler->IsEventSelected() & fTriggerBits)) return;
    if(!fInputEvent->GetFiredTriggerClasses().Contains(fTrigger.data())) return;
    AliDebugStream(1) << "Selecting trigger " << fTrigger << ": " << fInputEvent->GetFiredTriggerClasses() << std::endl;
    auto cells = fInputEvent->GetEMCALCells();
    for(int icell = 0; icell < cells->GetNumberOfCells(); icell++) {
        fCellContainer->emplace_back(cells->GetCellNumber(icell), cells->GetAmplitude(icell), cells->GetTime(icell) * SECONDSTONANOSECONDS);
        ChannelType_t celltype = cells->GetHighGain(icell) ? ChannelType_t::HIGH_GAIN : ChannelType_t::LOW_GAIN;
        fCellContainer->back().setType(celltype);
    }
    AliDebugStream(1) << "Cell container has " << fCellContainer->size() << " cell" << std::endl;

    fO2simtree->Fill();
    PostData(1, fO2simtree);
}

AliAnalysisTaskEmcalRun3ConverterCells *AliAnalysisTaskEmcalRun3ConverterCells::AddTaskEmcalRun3ConverterCells(const char *name, const char *outputfile) {
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if(!mgr) {
        std::cerr << "No analysis manager provided ..." << std::endl;
        return nullptr;
    }

    auto task = new AliAnalysisTaskEmcalRun3ConverterCells(name);
    mgr->AddTask(task);

    mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
    mgr->ConnectOutput(task, 1, mgr->CreateContainer("o2sim", TTree::Class(), AliAnalysisManager::kOutputContainer, outputfile));

    return task;
}
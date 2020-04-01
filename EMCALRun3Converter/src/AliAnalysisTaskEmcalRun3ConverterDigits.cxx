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
#include <TH1.h>
#include <TList.h>
#include <TRandom.h>
#include <TTree.h>
#include <TString.h>
#include "AliAnalysisManager.h"
#include "EMCALRun3Converter/AliAnalysisTaskEmcalRun3ConverterDigits.h"
#include "DataFormatsEMCAL/Constants.h"
#include "AliInputEventHandler.h"
#include "AliLog.h"
#include "AliVCaloCells.h"
#include "AliVEvent.h"

ClassImp(o2::emcal::AliAnalysisTaskEmcalRun3ConverterDigits)

using namespace o2::emcal;

AliAnalysisTaskEmcalRun3ConverterDigits::AliAnalysisTaskEmcalRun3ConverterDigits(const char *name) :
    AliAnalysisTaskSE(name),
    fO2simtree(nullptr),
    fTimeframeLengthCreator(nullptr),
    fQAHistos(nullptr),
    fHistHandler(),
    fDigitContainer(nullptr),
    fDigitTriggerRecords(nullptr),
    fTrigger("INT7"),
    fTriggerBits(-1),
    fCurrentEvent(0),
    fEventsTimeframe(0)

{
    DefineOutput(1, TTree::Class());
    DefineOutput(2, TList::Class());
}

AliAnalysisTaskEmcalRun3ConverterDigits::~AliAnalysisTaskEmcalRun3ConverterDigits() {
    if(fTimeframeLengthCreator) delete fTimeframeLengthCreator;
}

void AliAnalysisTaskEmcalRun3ConverterDigits::UserCreateOutputObjects(){
    fDigitContainer = new std::vector<o2::emcal::Digit>;
    fDigitTriggerRecords = new std::vector<o2::emcal::TriggerRecord>;

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

    fTimeframeLengthCreator = new TRandom();

    fCurrentEvent = 0;
    fEventsTimeframe = fTimeframeLengthCreator->Gaus(300, 10);

    fQAHistos = new TList;
    fQAHistos->SetOwner(true);
    fQAHistos->Add(new TH1D("nEventsAll", "Number of events", 1, 0.5, 1.5));
    fQAHistos->Add(new TH1D("nTimeframesAll", "Number of timeframes", 1, 0.5, 1.5));
    fQAHistos->Add(new TH1D("nEventsTimeframe", "Number of events per fimeframe", 500, 0., 500.));
    fQAHistos->Add(new TH1D("nTriggersTimeframe", "Number of triggers per fimeframe", 500, 0., 500.));
    fQAHistos->Add(new TH1D("nDigitsTimeframe", "Number of digits per timeframe", 1000, 0., 100000.));
    fQAHistos->Add(new TH1D("nDigitsTrigger", "Number of digits per trigger", 5000, 0., 5000.));
    for(auto en : TRangeDynCast<TH1>(fQAHistos)) fHistHandler[en->GetName()] = en;

    OpenFile(1);
    fO2simtree = new TTree("o2sim", "o2sim");
    fO2simtree->Branch("EMCALDigit", &fDigitContainer);
    fO2simtree->Branch("EMCALDigitTRGR", &fDigitTriggerRecords);

    PostData(1, fO2simtree);
    PostData(2, fQAHistos);
}

void AliAnalysisTaskEmcalRun3ConverterDigits::UserExec(Option_t *){
    const double SECONDSTONANOSECONDS = 1e9;
    if(!(fInputHandler->IsEventSelected() & fTriggerBits)) return;
    if(!fInputEvent->GetFiredTriggerClasses().Contains(fTrigger.data())) return;
    fHistHandler["nEventsAll"]->Fill(1.);
    AliDebugStream(1) << "Selecting trigger " << fTrigger << ": " << fInputEvent->GetFiredTriggerClasses() << std::endl;
    auto cells = fInputEvent->GetEMCALCells();
    int currentcell = fDigitContainer->size(), ndigitsevent = 0;
    for(int icell = 0; icell < cells->GetNumberOfCells(); icell++) {
        fDigitContainer->emplace_back(cells->GetCellNumber(icell), cells->GetAmplitude(icell), cells->GetTime(icell) * SECONDSTONANOSECONDS);
        ChannelType_t celltype = cells->GetHighGain(icell) ? ChannelType_t::HIGH_GAIN : ChannelType_t::LOW_GAIN;
        fDigitContainer->back().setType(celltype);
        ndigitsevent++;
    }
    fHistHandler["nDigitsTrigger"]->Fill(ndigitsevent);
    AliDebugStream(1) << "Cell container has " << fDigitContainer->size() << " cell" << std::endl;

    o2::InteractionRecord bcdata;
    bcdata.bc = fInputEvent->GetHeader()->GetTimeStamp();
    bcdata.orbit = fInputEvent->GetHeader()->GetOrbitNumber();
    fDigitTriggerRecords->emplace_back(bcdata, currentcell, ndigitsevent);
    fCurrentEvent++;

    if(fCurrentEvent >= fEventsTimeframe) WriteDigits();

    PostData(1, fO2simtree);
    PostData(2, fQAHistos);
}

void AliAnalysisTaskEmcalRun3ConverterDigits::FinishTaskOutput(){
    if(fDigitTriggerRecords->size()) WriteDigits();
}

void AliAnalysisTaskEmcalRun3ConverterDigits::WriteDigits() {
    AliInfoStream() << "Writing new time frame with " << fDigitTriggerRecords->size() << " events and " << fDigitContainer->size() << " digits " << std::endl;
    fO2simtree->Fill();
    // Fill some QA histograms
    fHistHandler["nTimeframesAll"]->Fill(1.);
    fHistHandler["nEventsTimeframe"]->Fill(fCurrentEvent);
    fHistHandler["nTriggersTimeframe"]->Fill(fDigitTriggerRecords->size());
    fHistHandler["nDigitsTimeframe"]->Fill(fDigitContainer->size());
    // prepare for new time frame
    fCurrentEvent = 0;
    fEventsTimeframe = fTimeframeLengthCreator->Gaus(300, 10);
    fDigitContainer->clear();
    fDigitTriggerRecords->clear();
}

AliAnalysisTaskEmcalRun3ConverterDigits *AliAnalysisTaskEmcalRun3ConverterDigits::AddTaskEmcalRun3ConverterDigits(const char *name, const char *outputfile) {
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if(!mgr) {
        std::cerr << "No analysis manager provided ..." << std::endl;
        return nullptr;
    }

    auto task = new AliAnalysisTaskEmcalRun3ConverterDigits(name);
    mgr->AddTask(task);

    mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
    mgr->ConnectOutput(task, 1, mgr->CreateContainer("o2sim", TTree::Class(), AliAnalysisManager::kOutputContainer, outputfile));
    mgr->ConnectOutput(task, 2, mgr->CreateContainer("DigitsConversionHistos", TList::Class(), AliAnalysisManager::kOutputContainer, mgr->GetCommonFileName()));

    return task;
}

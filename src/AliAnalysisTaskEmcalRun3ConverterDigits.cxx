// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#include "TTree.h"
#include "AliAnalysisTaskEmcalRun3ConverterDigits.h"
#include "AliVCaloCells.h"

ClassImp(o2::emc::AliAnalysisTaskEmcalRun3ConverterDigits)

using namespace o2::emc;

AliAnalysisTaskEmcalRun3ConverterDigits::AliAnalysisTaskEmcalRun3ConverterDigits(const char *name) :
    AliAnalysisTaskSE(name),
    fO2simtree(nullptr),
    fDigitContainer(nullptr)
{
    DefineOutput(1, TTree::Class());
}

void AliAnalysisTaskEmcalRun3ConverterDigits::UserCreateOutputObjects(){
    fDigitContainer = new std::vector<o2::EMCAL::Digit>;
    fO2simtree = new TTree("o2sim", "o2sim");
    fO2simtree->SetBranchAddress("EMCALDigit", &fDigitContainer);

    PostData(1, fO2simtree);
}

void AliAnalysisTaskEmcalRun3ConverterDigits::UserExec(Option_t *){
    fDigitContainer->clear();
    auto cells = fInputEvent->GetEMCALCells();
    for(int icell = 0; icell < cells->GetNumberOfCells(); icell++) fDigitContainer->emplace_back(cells->GetCellPosition(icell), cells->GetCellAmplitude(icell), cells->GetCellTime(icell));

    fO2simtree->Fill();
    PostData(1, fO2simtree);
}
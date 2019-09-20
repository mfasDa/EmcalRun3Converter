// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef ALIANALYSISTASKEMCALRUN3CONVERTERCELLS_H
#define ALIANALYSISTASKEMCALRUN3CONVERTERCELLS_H

#include "AliAnalysisTaskSE.h"
#include "DataFormatsEMCAL/Cell.h"
#include <string>
#include <vector>

class TTree;

namespace o2 {

namespace emcal {

class AliAnalysisTaskEmcalRun3ConverterCells : public AliAnalysisTaskSE {
public:
    AliAnalysisTaskEmcalRun3ConverterCells() = default;
    AliAnalysisTaskEmcalRun3ConverterCells(const char *name);
    AliAnalysisTaskEmcalRun3ConverterCells(const AliAnalysisTaskEmcalRun3ConverterCells &) = delete;
    AliAnalysisTaskEmcalRun3ConverterCells &operator=(const AliAnalysisTaskEmcalRun3ConverterCells &) = delete;
    virtual ~AliAnalysisTaskEmcalRun3ConverterCells() = default;

    void SetTrigger(const char *trigger) { fTrigger = trigger; }

    static AliAnalysisTaskEmcalRun3ConverterCells *AddTaskEmcalRun3ConverterCells(const char *name, const char *outputfile = "emcal.cells.root");

protected:
    virtual void UserCreateOutputObjects();
    virtual void UserExec(Option_t *);

private:
    TTree*                                  fO2simtree;
    std::vector<o2::emcal::Cell>*           fCellContainer;
    std::string                             fTrigger;
    UInt_t                                  fTriggerBits;

    ClassDef(AliAnalysisTaskEmcalRun3ConverterCells, 1);
};

}

}


#endif

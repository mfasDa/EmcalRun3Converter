// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef ALIANALYSISTASKEMCALRUN3CONVERTERDIGITS_H
#define ALIANALYSISTASKEMCALRUN3CONVERTERDIGITS_H

#include "AliAnalysisTaskSE.h"
#include "EMCALBase/Digit.h"
#include <string>
#include <vector>

class TTree;

namespace o2 {

namespace emc {

class AliAnalysisTaskEmcalRun3ConverterDigits : public AliAnalysisTaskSE {
public:
    AliAnalysisTaskEmcalRun3ConverterDigits() = default;
    AliAnalysisTaskEmcalRun3ConverterDigits(const char *name);
    AliAnalysisTaskEmcalRun3ConverterDigits(const AliAnalysisTaskEmcalRun3ConverterDigits &) = delete;
    AliAnalysisTaskEmcalRun3ConverterDigits &operator=(const AliAnalysisTaskEmcalRun3ConverterDigits &) = delete;
    virtual ~AliAnalysisTaskEmcalRun3ConverterDigits() = default;

    void SetTrigger(const char *trigger) { fTrigger = trigger; }

    static AliAnalysisTaskEmcalRun3ConverterDigits *AddTaskEmcalRun3ConverterDigits(const char *name, const char *outputfile = "o2sim.root");

protected:
    virtual void UserCreateOutputObjects();
    virtual void UserExec(Option_t *);

private:
    TTree*                                  fO2simtree;
    std::vector<o2::emcal::Digit>*          fDigitContainer;
    std::string                             fTrigger;
    UInt_t                                  fTriggerBits;

    ClassDef(AliAnalysisTaskEmcalRun3ConverterDigits, 1);
};

}

}


#endif
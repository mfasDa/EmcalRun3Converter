// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction
#include <iostream>
#include <vector>
#include "gsl/span"

#include "AliDAQ.h"
#include "AliRawReaderRoot.h"

#include "Headers/RAWDataHeader.h"
#include "EMCALRun3Converter/AliEmcalRawConverter.h"

using namespace o2::emcal;

AliEmcalRawConverter::AliEmcalRawConverter(const std::string_view filerawin, const std::string_view filerawout):
    mInputFile(filerawin),
    mOutputStream(filerawout.data())
    {}

void AliEmcalRawConverter::convert()
{
    const int MIN_DDL_EMCAL = AliDAQ::DdlIDOffset("EMCAL"),
              MAX_DDL_EMCAL = MIN_DDL_EMCAL + AliDAQ::GetFirstSTUDDL() -1; // Discard STU DDLs
    std::cout << "Using DDL range " << MIN_DDL_EMCAL << " to " << MAX_DDL_EMCAL << std::endl;
    AliRawReaderRoot inputStream(mInputFile.data());
    inputStream.Select("EMCAL",0 , AliDAQ::GetFirstSTUDDL() -1); 
    inputStream.RewindEvents();
    mOutputStream.open();
    unsigned char *dataptr(nullptr);
    while (inputStream.NextEvent())
    {
        std::cout << "Reading next event" << std::endl;
        inputStream.Reset();

        bool error = false;
        do {
            if (!inputStream.ReadNextData(dataptr)) {
                error = true;
                break;
            }
        } while (inputStream.GetDataSize() == 0);
        if(error){
            std::cerr << "Error decoding event" << std::endl;
            continue;
        }
        std::cout << "Converting data for Equipment " << inputStream.GetEquipmentId() << " (" << MIN_DDL_EMCAL << "," << MAX_DDL_EMCAL << ")\n";
        auto headerold = inputStream.GetDataHeaderV3();
        header::RAWDataHeaderV4 headernew;
        headernew.triggerBC = headerold->GetEventID1();
        headernew.heartbeatBC = headerold->GetEventID2();
        headernew.feeId = inputStream.GetEquipmentId() - MIN_DDL_EMCAL;
        headernew.triggerType = headerold->GetTriggerClasses();
        mOutputStream.writeData(headernew, gsl::span<char>(reinterpret_cast<char *>(dataptr), inputStream.GetDataSize()));
    }
}
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
    mOutputStream(filerawout.data()),
    mCurrentDataBuffer(),
    mCurrentHeader(nullptr),
    mCurrentEquipment(-1)
{

}

bool AliEmcalRawConverter::nextDDL(AliRawReaderRoot &reader) {
    unsigned char *dataptr(nullptr);
    do {
        if (!reader.ReadNextData(dataptr)) {
            mCurrentDataBuffer = gsl::span<char>();
            mCurrentHeader = nullptr;
            mCurrentEquipment = -1;
            return false;
        }
    } while (reader.GetDataSize() == 0);
    mCurrentDataBuffer = gsl::span<char>(reinterpret_cast<char *>(dataptr), reader.GetDataSize());
    mCurrentHeader = reader.GetDataHeaderV3();
    mCurrentEquipment =  reader.GetEquipmentId();
    return true;
}

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
        while(nextDDL(inputStream)) {
            std::cout << "Converting data for Equipment " << mCurrentEquipment << " (" << MIN_DDL_EMCAL << "," << MAX_DDL_EMCAL << ")\n";
            header::RAWDataHeaderV4 headernew;
            headernew.triggerBC = mCurrentHeader->GetEventID1();
            headernew.heartbeatBC = mCurrentHeader->GetEventID2();
            headernew.feeId = mCurrentEquipment - MIN_DDL_EMCAL;
            headernew.triggerType = mCurrentHeader->GetTriggerClasses();
            mOutputStream.writeData(headernew, mCurrentDataBuffer);
        }
    }
}
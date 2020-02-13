// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction
#ifndef ALIEMCALRAWCONVERTER_H
#define ALIEMCALRAWCONVERTER_H

#include <gsl/span>
#include <string>
#include "Rtypes.h"
#include "RStringView.h"
#include "EMCALSimulation/DMAOutputStream.h"

class AliRawReaderRoot;
class AliRawDataHeaderV3;

namespace o2
{

namespace emcal
{

class AliEmcalRawConverter
{
public:
    AliEmcalRawConverter() = default;
    AliEmcalRawConverter(const std::string_view filerawin, const std::string_view filerawout);
    ~AliEmcalRawConverter() = default;

    void setInputFile(const char *name) { mInputFile = name; }
    void convert();

private:
    bool nextDDL(AliRawReaderRoot &reader);
    std::string mInputFile;
    gsl::span<char> mCurrentDataBuffer;
    const AliRawDataHeaderV3 *mCurrentHeader;
    Int_t mCurrentEquipment;
    
    DMAOutputStream mOutputStream;

    ClassDefNV(AliEmcalRawConverter, 1)
}; // namespace emcal

} // namespace emcal

} // namespace o2

#endif
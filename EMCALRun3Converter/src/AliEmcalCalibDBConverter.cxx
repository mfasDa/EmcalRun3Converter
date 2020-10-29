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
#include <TObjArray.h>
#include <ROOT/TSeq.hxx>

#include "EMCALCalib/BadChannelMap.h"
#include "EMCALCalib/TimeCalibrationParams.h"
#include "EMCALRun3Converter/AliEmcalCalibDBConverter.h"

#include "AliCaloCalibPedestal.h"
#include "AliDataFile.h"
#include "AliOADBContainer.h"

using namespace o2::emcal;

AliEmcalCalibDBConverter::AliEmcalCalibDBConverter(const char* ccdbUrl) : mTimeOffset(0.),
                                                                          mCcdbApi()
{
  mCcdbApi = std::make_unique<o2::ccdb::CcdbApi>();
  mCcdbApi->init(ccdbUrl);
}

void AliEmcalCalibDBConverter::convertBadCells(int runnumber, long timestamp)
{
  std::cout << "Creating run3 equivalent bad channel map based on run " << runnumber << ", " << std::endl
            << "using input file " << AliDataFile::GetFileNameOADB("EMCAL/EMCALBadChannels_1D.root") << std::endl;
  AliOADBContainer bcmrun2("AliEMCALBadChannels", "AliEMCALBadChannels");
  bcmrun2.InitFromFile(AliDataFile::GetFileNameOADB("EMCAL/EMCALBadChannels_1D.root").data(), "AliEMCALBadChannels");
  auto badChannelData = static_cast<TH1*>((static_cast<TObjArray*>(bcmrun2.GetObject(runnumber)))->At(0));

  o2::emcal::BadChannelMap bcmrun3;
  for (auto icell : ROOT::TSeqI(0, 17664)) {
    auto statusRun2 = static_cast<char>(badChannelData->GetBinContent(icell));
    o2::emcal::BadChannelMap::MaskType_t statusRun3 = o2::emcal::BadChannelMap::MaskType_t::GOOD_CELL;
    switch (statusRun2) {
      case AliCaloCalibPedestal::kAlive:
        statusRun3 = o2::emcal::BadChannelMap::MaskType_t::GOOD_CELL;
        break;
      case AliCaloCalibPedestal::kDead:
        statusRun3 = o2::emcal::BadChannelMap::MaskType_t::DEAD_CELL;
        break;
      case AliCaloCalibPedestal::kHot:
        statusRun3 = o2::emcal::BadChannelMap::MaskType_t::BAD_CELL;
        break;
      case AliCaloCalibPedestal::kWarning:
        statusRun3 = o2::emcal::BadChannelMap::MaskType_t::WARM_CELL;
        break;

      default:
        statusRun3 = o2::emcal::BadChannelMap::MaskType_t::BAD_CELL;
        break;
    };
  }

  std::cout << "Bad channel map ready, storing under EMC/BadChannelMap with timestamp " << timestamp << std::endl;

  std::map<std::string, std::string> metadata;
  mCcdbApi->storeAsTFileAny<o2::emcal::BadChannelMap>(&bcmrun3, "EMC/BadChannelMap", metadata, timestamp);
}

void AliEmcalCalibDBConverter::convertTimeCalib(int runnumber, long timestamp)
{
  std::cout << "Creating run3 equivalent time calib object based on run " << runnumber << ", " << std::endl
            << "using input file " << AliDataFile::GetFileNameOADB("EMCAL/EMCALTimeCalibMergedBCs.root") << std::endl
            << "using time offset " << mTimeOffset << std::endl;
  AliOADBContainer timeCalibRun2("AliEMCALTimeCalib", "AliEMCALTimeCalib");
  timeCalibRun2.InitFromFile(AliDataFile::GetFileNameOADB("EMCAL/EMCALTimeCalibMergedBCs.root").data(), "AliEMCALTimeCalib");
  auto timeCalibHistos = static_cast<TObjArray*>(timeCalibRun2.GetObject(runnumber));
  auto pass1Histos = static_cast<TObjArray*>(timeCalibHistos->FindObject("pass1"));
  auto timeCalibHG = static_cast<TH1*>(pass1Histos->FindObject("hAllTimeAv")),
       timeCalibLG = static_cast<TH1*>(pass1Histos->FindObject("hAllTimeAvLG"));
  if (!timeCalibHG)
    std::cerr << "Did not find time calib histo for HG cells" << std::endl;
  if (!timeCalibLG)
    std::cerr << "Did not find time calib histo for LG cells" << std::endl;

  o2::emcal::TimeCalibrationParams timeCalibRun3;
  for (auto icell : ROOT::TSeqI(0, 17664)) {
    auto timeCalibValueHG = timeCalibHG->GetBinContent(icell),
         timeCalibValueLG = timeCalibLG->GetBinContent(icell);
    if (TMath::Abs(mTimeOffset) > DBL_EPSILON) {
      timeCalibValueHG += mTimeOffset;
      timeCalibValueLG += mTimeOffset;
    }
    timeCalibRun3.addTimeCalibParam(icell, timeCalibValueHG, false);
    timeCalibRun3.addTimeCalibParam(icell, timeCalibValueLG, true);
  }

  std::cout << "Time calib ready, storing under EMC/TimeCalibParams with timestamp " << timestamp << std::endl;

  std::map<std::string, std::string> metadata;
  mCcdbApi->storeAsTFileAny<o2::emcal::TimeCalibrationParams>(&timeCalibRun3, "EMC/TimeCalibrationParams", metadata, timestamp);
}
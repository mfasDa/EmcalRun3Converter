// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <memory>
#include "CCDB/CcdbApi.h"

namespace o2
{

namespace emcal
{

class AliEmcalCalibDBConverter
{
 public:
  AliEmcalCalibDBConverter() = default;
  AliEmcalCalibDBConverter(const char* ccdbUrl);
  ~AliEmcalCalibDBConverter() = default;

  void convertAndStore(int runnumber, long timestamp)
  {
    convertBadCells(runnumber, timestamp);
    convertTimeCalib(runnumber, timestamp);
  }

  void setPathBadChannels(const char *path) { mBadChannelPath = path; }
  void setPathTime(const char *path) { mTimePath = path; }

  void convertBadCells(int runnumber, long timestamp);
  void convertTimeCalib(int runnumber, long timestamp);

  void setTimeOffset(double timeoffset) { mTimeOffset = timeoffset; }

  void setCcdbUrl(const char* ccdbUrl)
  {
    if (!mCcdbApi) {
      mCcdbApi = std::make_unique<o2::ccdb::CcdbApi>();
    }
    mCcdbApi->init(ccdbUrl);
  }

 private:
  double mTimeOffset = 0.;
  std::unique_ptr<o2::ccdb::CcdbApi> mCcdbApi;
  std::string mBadChannelPath = "BadChannels";
  std::string mTimePath = "Time";
};

} // namespace emcal

} // namespace o2

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
#include "CommonDataFormat/InteractionRecord.h"
#include "DetectorsRaw/RawFileWriter.h"

class AliRawReaderRoot;
class AliRawDataHeaderV3;

namespace o2
{

namespace emcal
{

class AliEmcalRawConverter
{
 public:
  enum class FileFor_t {
    kFullDet,
    kSubDet,
    kLink
  };
  AliEmcalRawConverter() = default;
  AliEmcalRawConverter(const std::string_view filerawin, const std::string_view outputloc);
  ~AliEmcalRawConverter() = default;

  void setStartTimeRun(long timestamp) { mStartTimeRun = timestamp; }
  void setInputFile(const char* name) { mInputFile = name; }
  void setOuputLocation(const char* outputloc) { mOutputLocation = outputloc; }
  void setFileFor(FileFor_t filefor) { mFileFor = filefor; }
  void convert();

  int carryOverMethod(const header::RDHAny* rdh, const gsl::span<char> data,
                      const char* ptr, int maxSize, int splitID,
                      std::vector<char>& trailer, std::vector<char>& header) const;

 protected:
  void initRawWriter();
  std::tuple<int, int> getLinkAssignment(int ddlID);

 private:
  bool nextDDL(AliRawReaderRoot& reader);
  std::string mInputFile;
  std::string mOutputLocation;
  FileFor_t mFileFor;
  gsl::span<char> mCurrentDataBuffer;
  const AliRawDataHeaderV3* mCurrentHeader;
  Int_t mCurrentEquipment;
  Long_t mStartTimeRun;
  bool mRawWriterInitialized;
  InteractionRecord mCurrentIR;
  raw::RawFileWriter mOutputWriter;

  ClassDefNV(AliEmcalRawConverter, 1)
}; // namespace emcal

} // namespace emcal

} // namespace o2

#endif
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
#include <fmt/core.h>
#include "gsl/span"

#include "AliDAQ.h"
#include "AliRawReaderRoot.h"

#include "EMCALBase/RCUTrailer.h"
#include "EMCALRun3Converter/AliEmcalRawConverter.h"

using namespace o2::emcal;

AliEmcalRawConverter::AliEmcalRawConverter(const std::string_view filerawin, const std::string_view outputloc) : mInputFile(filerawin),
                                                                                                                 mOutputLocation(outputloc),
                                                                                                                 mFileFor(FileFor_t::kFullDet),
                                                                                                                 mCurrentIR(),
                                                                                                                 mCurrentDataBuffer(),
                                                                                                                 mCurrentHeader(nullptr),
                                                                                                                 mCurrentEquipment(-1),
                                                                                                                 mOutputWriter(o2::header::gDataOriginEMC, false)
{
}

bool AliEmcalRawConverter::nextDDL(AliRawReaderRoot& reader)
{
  unsigned char* dataptr(nullptr);
  do {
    if (!reader.ReadNextData(dataptr)) {
      mCurrentDataBuffer = gsl::span<char>();
      mCurrentHeader = nullptr;
      mCurrentEquipment = -1;
      return false;
    }
  } while (reader.GetDataSize() == 0);
  mCurrentDataBuffer = gsl::span<char>(reinterpret_cast<char*>(dataptr), reader.GetDataSize());
  mCurrentHeader = reader.GetDataHeaderV3();
  mCurrentEquipment = reader.GetEquipmentId();
  mCurrentIR.clear();
  mCurrentIR.orbit = reader.GetOrbitID();
  mCurrentIR.bc = reader.GetBCID();
  return true;
}

void AliEmcalRawConverter::initRawWriter()
{
  for (int iddl = 0; iddl < 40; iddl++) {
    // For EMCAL set
    // - FEE ID = DDL ID
    // - C-RORC and link increasing with DDL ID
    // @TODO replace with link assignment on production FLPs,
    // eventually storing in CCDB
    auto [crorc, link] = getLinkAssignment(iddl);
    std::string rawfilename = mOutputLocation;
    switch (mFileFor) {
      case FileFor_t::kFullDet:
        rawfilename += "/emcal.raw";
        break;
      case FileFor_t::kSubDet: {
        std::string detstring;
        if (iddl < 22)
          detstring = "emcal";
        else
          detstring = "dcal";
        rawfilename += fmt::format("/{:s}.raw", detstring.data());
        break;
      };
      case FileFor_t::kLink:
        rawfilename += fmt::format("/emcal_{:d}_{:d}.raw", crorc, link);
    }
    mOutputWriter.registerLink(iddl, crorc, link, 0, rawfilename.data());
  }
}

void AliEmcalRawConverter::convert()
{
  const int MIN_DDL_EMCAL = AliDAQ::DdlIDOffset("EMCAL"),
            MAX_DDL_EMCAL = MIN_DDL_EMCAL + AliDAQ::GetFirstSTUDDL() - 1; // Discard STU DDLs
  std::cout << "Using DDL range " << MIN_DDL_EMCAL << " to " << MAX_DDL_EMCAL << std::endl;
  AliRawReaderRoot inputStream(mInputFile.data());
  inputStream.Select("EMCAL", 0, AliDAQ::GetFirstSTUDDL() - 1);
  inputStream.RewindEvents();
  unsigned char* dataptr(nullptr);
  bool initIR(true);

  while (inputStream.NextEvent()) {
    std::cout << "Reading next event" << std::endl;
    inputStream.Reset();
    bool initTrigger(true);
    while (nextDDL(inputStream)) {
      std::cout << "Converting data for Equipment " << mCurrentEquipment << " (" << MIN_DDL_EMCAL << "," << MAX_DDL_EMCAL << ")\n";
      std::vector<char> pagebuffer(mCurrentDataBuffer.size());
      memcpy(pagebuffer.data(), mCurrentDataBuffer.data(), mCurrentDataBuffer.size());
      int iddl = mCurrentEquipment - MIN_DDL_EMCAL;
      auto [crorc, link] = getLinkAssignment(iddl);
      mOutputWriter.addData(iddl, crorc, link, 0, mCurrentIR, pagebuffer);
    }
  }
}

std::tuple<int, int> AliEmcalRawConverter::getLinkAssignment(int ddlID)
{
  // Temporary link assignment (till final link assignment is known -
  // eventually taken from CCDB)
  // - Link (0-5) and C-RORC ID linear with ddlID
  return std::make_tuple(ddlID / 6, ddlID % 6);
}

int AliEmcalRawConverter::carryOverMethod(const header::RDHAny* rdh, const gsl::span<char> data,
                                          const char* ptr, int maxSize, int splitID,
                                          std::vector<char>& trailer, std::vector<char>& header) const
{
  int offs = ptr - &data[0]; // offset wrt the head of the payload
  // make sure ptr and end of the suggested block are within the payload
  assert(offs >= 0 && size_t(offs + maxSize) <= data.size());

  // Read trailer template from the end of payload
  gsl::span<const uint32_t> payloadwords(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t));
  auto rcutrailer = RCUTrailer::constructFromPayloadWords(payloadwords);

  int sizeNoTrailer = maxSize - rcutrailer.getTrailerSize() * sizeof(uint32_t);
  // calculate payload size for RCU trailer:
  // assume actualsize is in byte
  // Payload size is defined as the number of 32-bit payload words
  // -> actualSize to be converted to size of 32 bit words
  auto payloadsize = sizeNoTrailer / sizeof(uint32_t);
  rcutrailer.setPayloadSize(payloadsize);
  auto trailerwords = rcutrailer.encode();
  trailer.resize(trailerwords.size() * sizeof(uint32_t));
  memcpy(trailer.data(), trailerwords.data(), trailer.size());
  // Size to return differs between intermediate pages and last page
  // - intermediate page: Size of the trailer needs to be removed as the trailer gets appended
  // - last page: Size of the trailer needs to be included as the trailer gets replaced
  int bytesLeft = data.size() - (ptr - &data[0]);
  bool lastPage = bytesLeft <= maxSize;
  int actualSize = maxSize;
  if (!lastPage) {
    actualSize = sizeNoTrailer;
  }
  return actualSize;
}
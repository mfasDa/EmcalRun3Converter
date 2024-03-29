// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction
#include <bitset>
#include <iostream>
#include <vector>
#include <fmt/core.h>
#include "gsl/span"

#include "AliDAQ.h"
#include "AliRawReaderRoot.h"
#include "AliRawEventHeaderBase.h"

#include "CommonConstants/Triggers.h"
#include "EMCALBase/RCUTrailer.h"
#include "EMCALBase/Mapper.h"
#include "EMCALBase/Geometry.h"
#include "DetectorsRaw/RDHUtils.h"
#include "EMCALRun3Converter/AliEmcalRawConverter.h"

using namespace o2::emcal;

AliEmcalRawConverter::AliEmcalRawConverter(const std::string_view filerawin, const std::string_view outputloc) : mInputFile(filerawin),
                                                                                                                 mOutputLocation(outputloc),
                                                                                                                 mFileFor(FileFor_t::kFullDet),
                                                                                                                 mCurrentIR(),
                                                                                                                 mCurrentDataBuffer(),
                                                                                                                 mCurrentHeader(nullptr),
                                                                                                                 mCurrentEquipment(-1),
                                                                                                                 mSelectTriggerClassIndex(-1),
                                                                                                                 mStartTimeRun(-1),
                                                                                                                 mRawWriterInitialized(false),
                                                                                                                 mOutputWriter(o2::header::gDataOriginEMC, false),
                                                                                                                 mGeom(nullptr)
{
  mOutputWriter.setRORCDetector();
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
  if(!mGeom) mGeom = Geometry::GetInstanceFromRunNumber(300000);
  for (int iddl = 0; iddl < 40; iddl++) {
    // For EMCAL set
    // - FEE ID = DDL ID
    // - C-RORC and link increasing with DDL ID
    // @TODO replace with link assignment on production FLPs,
    // eventually storing in CCDB
    auto [crorc, link] = mGeom->getLinkAssignment(iddl);
    if(link < 0) continue;
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
        rawfilename += fmt::format("/emcal_{:d}.raw", iddl);
    }
    mOutputWriter.registerLink(iddl, crorc, link, getEndpoint(iddl), rawfilename.data());
  }
  mOutputWriter.setCarryOverCallBack(this);
  mOutputWriter.setApplyCarryOverToLastPage(true);
  mRawWriterInitialized = true;
}

void AliEmcalRawConverter::convert()
{
  const int MIN_DDL_EMCAL = AliDAQ::DdlIDOffset("EMCAL"),
            MAX_DDL_EMCAL = MIN_DDL_EMCAL + AliDAQ::GetFirstSTUDDL() - 1; // Discard STU DDLs
  if(!mRawWriterInitialized) {
    initRawWriter();
  }
  std::cout << "Using DDL range " << MIN_DDL_EMCAL << " to " << MAX_DDL_EMCAL << std::endl;
  AliRawReaderRoot inputStream(mInputFile.data());
  inputStream.Select("EMCAL", 0, AliDAQ::GetFirstSTUDDL() - 1);
  inputStream.RewindEvents();
  unsigned char* dataptr(nullptr);
  bool initIR(true);

  const int MUS_TO_NS = 1000;
  int busytime = 100 * MUS_TO_NS;

  long currenttime = mStartTimeRun;
  uint32_t trigger;
  while (inputStream.NextEvent()) {
    auto type = inputStream.GetType();
    if(type == AliRawEventHeaderBase::kCalibrationEvent) {
         trigger = o2::trigger::Cal;
    } else if (type == AliRawEventHeaderBase::kPhysicsEvent) {
      trigger = o2::trigger::PhT;
    } else {
      std::cerr << "Unknown trigger type " << type << ", skipping ..." << std::endl;
      continue;
    }
  
    if(!selectTrigger(inputStream.GetClassMask(), inputStream.GetClassMaskNext50())) continue;
    std::cout << "Reading next event" << std::endl;
    inputStream.Reset();
    bool initTrigger(true);
    while (nextDDL(inputStream)) {
      std::cout << "Converting data for Equipment " << mCurrentEquipment << " (" << MIN_DDL_EMCAL << "," << MAX_DDL_EMCAL << ")\n";
      int iddl = mCurrentEquipment - MIN_DDL_EMCAL;
      auto [crorc, link] = mGeom->getLinkAssignment(iddl);
      gsl::span<const uint32_t> payloadwords(reinterpret_cast<const uint32_t*>(mCurrentDataBuffer.data()), mCurrentDataBuffer.size() / sizeof(uint32_t));
      auto rcutrailer = RCUTrailer::constructFromPayloadWords(payloadwords);
      std::cout << rcutrailer << std::endl;
      auto endpoint = getEndpoint(iddl);
      std::cout << "Adding data for ddl " << iddl << ", c-rorc " << crorc << ", link " << link << ", FLP " << endpoint << " with size " <<  mCurrentDataBuffer.size() << std::endl;
      o2::InteractionRecord currentIR;
      if(currenttime >= 0) {
        // Set Run3 fake timestamps
        currentIR.setFromNS(currenttime);
        currenttime += busytime;
      } else {
        currentIR = mCurrentIR;
      }
      mOutputWriter.addData(iddl, crorc, link, endpoint, currentIR, mCurrentDataBuffer, false, trigger);
      std::cout << "Adding data done" << std::endl;
    }
  }
}

bool AliEmcalRawConverter::selectTrigger(uint64_t triggerClassesFirst, uint64_t triggerClassesNext50) const {
  if(mSelectTriggerClassIndex < 0) return true; // no selection required
  std::bitset<100> triggerbits;
  for(int ibit = 0; ibit < 50; ibit++) {
    triggerbits.set(ibit, TESTBIT(triggerClassesFirst, ibit));
    triggerbits.set(ibit+50, TESTBIT(triggerClassesNext50, ibit));
  }
  return triggerbits.test(mSelectTriggerClassIndex);
}

int AliEmcalRawConverter::getEndpoint(int ddlID) {
  // FLP ID is endpoint;
  if(ddlID < 24) return 146;
  return 147;
}

int AliEmcalRawConverter::carryOverMethod(const header::RDHAny* rdh, const gsl::span<char> data,
                                          const char* ptr, int maxSize, int splitID,
                                          std::vector<char>& trailer, std::vector<char>& header) const
{
  std::cout << "Calling carry-over method with data size " << data.size() << ", max size " << maxSize << ", splitID " << splitID << std::endl;
  int offs = ptr - &data[0]; // offset wrt the head of the payload
  // make sure ptr and end of the suggested block are within the payload
  assert(offs >= 0 && size_t(offs + maxSize) <= data.size());

  // Read trailer template from the end of payload
  gsl::span<const uint32_t> payloadwords(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t));
  auto rcutrailer = RCUTrailer::constructFromPayloadWords(payloadwords);
  auto trailersize = rcutrailer.getTrailerSize() * sizeof(uint32_t);
  // Cannot split trailer, suggest new page
  if(maxSize < trailersize)
    return 0;

  int sizeNoTrailer = maxSize - trailersize;
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
  std::cout << "Carry-over method: max size: " << maxSize << ", sizeNoTrailer " << sizeNoTrailer << ", actualSize " << actualSize << std::endl;
  return actualSize;
}
#ifndef _WATERTANK_H_
#define _WATERTANK_H_

#include <SinricProDevice.h>
#include <Capabilities/RangeController.h>

class WaterTank 
: public SinricProDevice
, public RangeController<WaterTank> {
  friend class RangeController<WaterTank>;
public:
  WaterTank(const String &deviceId) : SinricProDevice(deviceId, "WaterTank") {};
};

#endif

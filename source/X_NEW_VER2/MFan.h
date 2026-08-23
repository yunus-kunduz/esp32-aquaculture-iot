/*
 *  Copyright (c) 2019 Sinric. All rights reserved.
 *  Licensed under Creative Commons Attribution-Share Alike (CC BY-SA)
 *
 *  This file is part of the Sinric Pro (https://github.com/sinricpro/)
 */

#pragma once

#include "SinricProDevice.h"
#include "Capabilities/SettingController.h"
#include "Capabilities/PowerStateController.h"
#include "Capabilities/RangeController.h"
#include "Capabilities/TemperatureSensor.h"

 

/**
 * @class MFan
 * @brief Device to control a fan with on / off commands and its speed by a range value
 * @ingroup Devices
 */
class MFan :  public SinricProDevice,
                        public SettingController<MFan>,
                        public PowerStateController<MFan>,
                        public RangeController<MFan>,
                        public TemperatureSensor<MFan> {
                        friend class SettingController<MFan>;
                        friend class PowerStateController<MFan>;
                        friend class RangeController<MFan>;
                        friend class TemperatureSensor<MFan>;
  public:
	  MFan(const String &deviceId) : SinricProDevice(deviceId, "FAN") {}
};

 

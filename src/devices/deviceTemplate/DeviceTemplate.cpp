/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "DeviceTemplate.h"

#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>

#include <cmath>

using namespace yarp::os;
using namespace yarp::dev;


YARP_LOG_COMPONENT(DEVICETEMPLATE, "yarp.deviceTemplate", yarp::os::Log::TraceType);


DeviceTemplate::DeviceTemplate()
{

}

bool DeviceTemplate::open(yarp::os::Searchable &config)
{
    if (!parseParams(config))  { return false; }

    yCInfo(DEVICETEMPLATE) << "Open";
    return true;
}

bool DeviceTemplate::close()
{
    yCInfo(DEVICETEMPLATE) << "Close";
    return true;
}

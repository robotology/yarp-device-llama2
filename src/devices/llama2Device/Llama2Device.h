/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef YARP_DEVICETEMPLATE_H
#define YARP_DEVICETEMPLATE_H

#include <yarp/dev/DeviceDriver.h>
#include "Llama2Device_ParamsParser.h"

class Llama2Device :
        public yarp::dev::DeviceDriver,
        public Llama2Device_ParamsParser
{
public:
    Llama2Device();
    Llama2Device(const Llama2Device&) = delete;
    Llama2Device(Llama2Device&&) noexcept = delete;
    Llama2Device& operator=(const Llama2Device&) = delete;
    Llama2Device& operator=(Llama2Device&&) noexcept = delete;
    ~Llama2Device() override = default;

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;
};

#endif // YARP_DEVICETEMPLATE_H

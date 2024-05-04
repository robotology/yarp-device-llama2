/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef YARP_LLAMA2DEVICE_H
#define YARP_LLAMA2DEVICE_H

#include <yarp/dev/ILLM.h>
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

~Llama2Device() override = default;

    // Rpc methods
    bool setPrompt(const std::string &prompt) override;

    bool readPrompt(std::string &oPrompt) override;

    bool ask(const std::string &question, std::string &oAnswer) override;

    bool getConversation(std::vector<std::pair<Author, Content>> &oConversation) override;

    bool deleteConversation() noexcept override;

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;
};

#endif // YARP_LLAMA2DEVICE_H

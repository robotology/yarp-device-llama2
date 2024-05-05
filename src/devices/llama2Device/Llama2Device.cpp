/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "common.h"
#include "llama.h"
#include "Llama2Device.h"

#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace yarp::os;
using namespace yarp::dev;


YARP_LOG_COMPONENT(LLAMA2DEVICE, "yarp.llama2Device", yarp::os::Log::TraceType);


Llama2Device::Llama2Device()
{

}

bool Llama2Device::open(yarp::os::Searchable &config)
{
    if (!parseParams(config))  { return false; }

    yCInfo(LLAMA2DEVICE) << "Open";
    return false;
}

bool Llama2Device::ask(const std::string &question, std::string &oAnswer)
{
    return false;
}

bool Llama2Device::setPrompt(const std::string &prompt)
{
    //setting up the command for the prompt setting
    std::string aPrompt;
    gpt_params params;

    if(readPrompt(aPrompt))
    {
        yError() << "A prompt already set";
        return false;
    }

    try
    {
        params.prompt = prompt;
    }
    catch(const std::exception& e)
    {
        yError() << e.what() << '\n';
        return false;
    }
    
    return true;
}

bool Llama2Device::readPrompt(std::string &oPrompt)
{
    return false;
}

bool Llama2Device::getConversation(std::vector<std::pair<Author, Content>> &oConversation)
{
    return false;
}

bool Llama2Device::deleteConversation() noexcept
{
    return false;
}

bool Llama2Device::close()
{
    yCInfo(LLAMA2DEVICE) << "Close";
    return false;
}

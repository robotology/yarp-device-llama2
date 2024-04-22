/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "Llama2Device.h"

#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>

#include <cmath>

using namespace yarp::os;
using namespace yarp::dev;


YARP_LOG_COMPONENT(DEVICETEMPLATE, "yarp.llama2Device", yarp::os::Log::TraceType);


Llama2Device::Llama2Device()
{

}

bool Llama2Device::open(yarp::os::Searchable &config)
{
    if (!parseParams(config))  { return false; }

    yCInfo(DEVICETEMPLATE) << "Open";
    return true;
}

bool Llama2Device::ask(const std::string &question, std::string &oAnswer)
{
    
}

bool Llama2Device::setPrompt(const std::string &prompt)
{
    
}

bool Llama2Device::readPrompt(std::string &oPrompt)
{
    
}

bool Llama2Device::getConversation(std::vector<std::pair<Author, Content>> &oConversation)
{

}

bool Llama2Device::deleteConversation() noexcept
{
    
}

bool Llama2Device::close()
{
    yCInfo(DEVICETEMPLATE) << "Close";
    return true;
}

/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "Llama2Device.h"
#include <fstream>
#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>

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

    std::string cfg_string = config.toString();
    yarp::os::Property cfg;
    cfg.fromString(cfg_string);

    std::string configuration_to_open;
    std::string innerFilePath="config_xml/ftc_local_only.xml";
    //textxml_from specifies the name of the file
    //textxml_context specifies a folder which yarp resource finder can search and access, this is the context
    //findXml is a resource finder
    if(cfg.check("testxml_from"))
    {
        yarp::os::ResourceFinder findXml;
        if(cfg.check("testxml_context"))
        {
            findXml.setDefaultContext(cfg.find("testxml_context").asString()); //here the default context is set
        }
        innerFilePath = findXml.findFileByName(cfg.find("testxml_from").asString()); //file is found simply passing the context and its name, no need for full file path
        std::ifstream xmlFile(innerFilePath);
    }

    yCInfo(DEVICETEMPLATE) << "Open";
    return true;
}

bool Llama2Device::close()
{
    yCInfo(DEVICETEMPLATE) << "Close";
    return true;
}

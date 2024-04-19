/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <yarp/os/Network.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/WrapperSingle.h>

#include <catch2/catch_amalgamated.hpp>
#include <harness.h>

using namespace yarp::dev;
using namespace yarp::os;

TEST_CASE("dev::llama2Device_test", "[yarp::dev]")
{
    YARP_REQUIRE_PLUGIN("llama2Device", "device");

    Network::setLocalMode(true);

    SECTION("Checking the device")
    {
        PolyDriver dd;

        //"Checking opening device"
        {
            Property pcfg;
            pcfg.put("device", "llama2Device");
            REQUIRE(dd.open(pcfg));
        }

        //Do something
        //...

        //"Close all polydrivers and check"
        {
            CHECK(dd.close());
        }
    }

    Network::setLocalMode(false);
}

/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <yarp/os/Network.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/WrapperSingle.h>
#include <yarp/dev/ILLM.h>

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
        ILLM* illm = nullptr;

        //"Checking opening device"
        {
            Property pcfg;
            pcfg.put("device", "llama2Device");
            pcfg.put("model_name", "/home/leonardo/Repos/yarp-device-llama2/models/gemma/gemma-2b-it.gguf");
            REQUIRE(dd.open(pcfg));
        }

        //Do something
        {
            dd.view(illm);

            std::string prompt = "This is a prompt";
            std::string out_prompt;

            bool ret = illm->readPrompt(out_prompt);
            CHECK_FALSE(ret); //Prompt is not set yet

            ret = illm->setPrompt(prompt);
            CHECK(ret);

            ret = illm->readPrompt(out_prompt);
            CHECK(out_prompt == prompt);

            //Since a prompt is already set you cannot set another one
            //The user is expected to delete the conversation before starting a new one.
            ret = illm->setPrompt(prompt);
            CHECK(!ret);

            yarp::dev::LLM_Message answer;
            ret = illm->ask("This is a question",answer);
            CHECK(ret); 

            // check if roles and content are set correctly
            std::vector<yarp::dev::LLM_Message> out_conversation;
            ret = illm->getConversation(out_conversation);
            CHECK(ret); //We have the system message
            CHECK(out_conversation[0].type == "system");
            CHECK(out_conversation[0].content == prompt);

            // check if deleteConversation returns true
            ret = illm->deleteConversation();
            CHECK(ret);

            // Check that conversation is actually deleted
            ret = illm->getConversation(out_conversation);
            CHECK(!ret);
        }

        //"Close all polydrivers and check"
        {
            CHECK(dd.close());
        }
    }

    Network::setLocalMode(false);
}

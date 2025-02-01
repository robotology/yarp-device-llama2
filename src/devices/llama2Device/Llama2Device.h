/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef YARP_LLAMA2DEVICE_H
#define YARP_LLAMA2DEVICE_H
//https://github.com/robotology/yarp/tree/master/src/libYARP_dev/src/idl_generated_code/yarp/dev
#include <yarp/dev/ILLM.h>
#include <yarp/dev/LLM_Message.h>
#include <yarp/dev/DeviceDriver.h>
#include "Llama2Device_ParamsParser.h"
#include <llama.h>
#include <common.h>
#include "sampling.h"
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>

enum class Author
{
    User,
    Model
};

using Content = std::string;
using Question = std::string;
using Answer = yarp::dev::LLM_Message;

class Llama2Device :
        public yarp::dev::DeviceDriver,
        public Llama2Device_ParamsParser,
        public yarp::dev::ILLM
{
public:
    Llama2Device();
    Llama2Device(const Llama2Device&) = delete;
    Llama2Device(Llama2Device&&) noexcept = delete;
    Llama2Device& operator=(const Llama2Device&) = delete;
    Llama2Device& operator=(Llama2Device&&) noexcept = delete;
    ~Llama2Device() override = default;

    // Rpc methods
    bool setPrompt(const std::string &prompt) override;

    bool readPrompt(std::string &oPrompt) override;

    bool ask(const std::string &question, yarp::dev::LLM_Message &oAnswer) override;

    bool getConversation(std::vector<yarp::dev::LLM_Message> &oConversation) override;

    bool deleteConversation() noexcept override;

    bool refreshConversation() noexcept override;

    // ILLM methods
    bool init_LLM(const std::string &model_path);

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;
    void help();

    void convAdd(std::vector<yarp::dev::LLM_Message>& conversation, const std::string& type, const std::string& content);

private:
    std::vector<llama_token> tokens_list;
    std::string model_path;
    llama_model *model;
    llama_batch batch = llama_batch_init(512, 0, 1);
    std::vector<std::pair<Author, Content>> conversation_log;
    llama_model_params model_params;
    common_params params;
    common_sampler * smpl = nullptr;
    llama_context * ctx = nullptr;
    std::vector<common_chat_msg> chat_msgs;
    std::string conversation;
    std::string model_question;
    std::string ask_question;
    int ngl;
    int n_predict;
    bool prompt_set = false;
    bool first_prompt_set = false;
    bool initialized = false;
    std::vector<yarp::dev::LLM_Message> m_conversation;
    yarp::dev::LLM_Message m_prompt;
};



#endif // YARP_LLAMA2DEVICE_H

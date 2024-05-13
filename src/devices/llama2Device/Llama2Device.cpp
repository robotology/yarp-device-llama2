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

    // finding the model
    gpt_params params;
    params.model = config.find("model").asString();
    // init LLM
    llama_backend_init();
    // initialize the model
    llama_model_params model_params = llama_model_default_params();
    // model_params.n_gpu_layers = 99; // offload all layers to the GPU
    llama_model * model = llama_load_model_from_file(params.model.c_str(), model_params);

    if (model == NULL) {
        fprintf(stderr , "%s: error: unable to load model\n" , __func__);
        return 1;
    }

    // initialize the context
    llama_context_params ctx_params = llama_context_default_params();

    ctx_params.seed  = 1234;
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = params.n_threads;
    ctx_params.n_threads_batch = params.n_threads_batch == -1 ? params.n_threads : params.n_threads_batch;
    
    if (ctx == NULL) {
        fprintf(stderr , "%s: error: unable to create context\n" , __func__);
        return 1;
    }
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
    // tokenize the prompt

    std::vector<llama_token> tokens_list;
    tokens_list = ::llama_tokenize(ctx, params.prompt, true);

    const int n_ctx    = llama_n_ctx(ctx);
    const int n_kv_req = tokens_list.size() + (n_len - tokens_list.size());

    LOG_TEE("\n%s: n_len = %d, n_ctx = %d, n_kv_req = %d\n", __func__, n_len, n_ctx, n_kv_req);

    // make sure the KV cache is big enough to hold all the prompt and generated tokens
    if (n_kv_req > n_ctx) {
        LOG_TEE("%s: error: n_kv_req > n_ctx, the required KV cache size is not big enough\n", __func__);
        LOG_TEE("%s:        either reduce n_len or increase n_ctx\n", __func__);
        return 1;
    }
    // llama_decode will output logits only for the last token of the prompt
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        LOG_TEE("%s: llama_decode() failed\n", __func__);
        return 1;
    }

    return true;
}

bool Llama2Device::readPrompt(std::string &oPrompt)
{
    // print the prompt token-by-token

    fprintf(stderr, "\n");

    for (auto id : tokens_list) {
        fprintf(stderr, "%s", llama_token_to_piece(ctx, id).c_str());
    }

    fflush(stderr);

    return true;
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

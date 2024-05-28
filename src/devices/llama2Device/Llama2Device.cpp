/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <common/common.h>
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

    // to do: look for  the model path in the configuration file
    model_path = config.find("model").asString(); // placeholder
    if(init_LLM(model_path) == false){
        fprintf(stderr , "%s: error: unable to load model\n" , __func__);
        return false; // return false if the model is not found
    }
    return false;
}

// method for the initialization of the LLM model
bool Llama2Device::init_LLM(const std::string &model_path)
{
    // finding the model
    gpt_params params;
    // init LLM
    llama_backend_init();
    // initialize the model
    llama_model_params model_params = llama_model_default_params();
    // model_params.n_gpu_layers = 99; // offload all layers to the GPU
    llama_model * model = llama_load_model_from_file(model_path.c_str(), model_params);

    // initialize the context
    llama_context_params ctx_params = llama_context_default_params();

    ctx_params.seed  = 1234;
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = params.n_threads;
    ctx_params.n_threads_batch = params.n_threads_batch == -1 ? params.n_threads : params.n_threads_batch;
    
    llama_context * ctx = llama_new_context_with_model(model, ctx_params);

    if (ctx == NULL) {
        fprintf(stderr , "%s: error: unable to create context\n" , __func__);
        return 1;
    }
    return true;
}

bool Llama2Device::ask(const std::string &question, std::string &oAnswer)
{
    // tokenize the question
    std::vector<llama_token> token_list = ::llama_tokenize(ctx, question, true);

    // prepare input tokens
    std::vector<llama_token> input_tokens = tokens_list;
    input_tokens.push_back(llama_token_eos(model));

    int n_past = 0;
    int n_remain = params.n_predict;

    std::vector<llama_token> embd;
    std::ostringstream output_ss;

    while(n_remain > 0){
        if (!embd.empty()){
            // evaluate the current batch of tokens
            llama_batch batch;
            batch.token = embd.data();
            batch.logits = nullptr;
            batch.n_tokens = static_cast<int32_t>(embd.size());

            if(llama_decode(ctx, batch) != 0){
                std::cerr << "Failed to decode tokens." << std::endl;
                return false;
            }
            n_past += embd.size();
            embd.clear();
        }

        float *logits = llama_get_logits(ctx);
        if(!logits){
            std::cerr << "Failed to get logits." << std::endl;
            return false;
        }

        int n_vocab =  llama_n_vocab(model);
        std::vector<llama_token_data> candidates_data(n_vocab);
        for(int i = 0; i < n_vocab; ++i){
            candidates_data[i]  = {i,logits[i], 0.0f};
        }

        // get the logits and sample the next token
        llama_token_data_array candidates = {candidates_data.data(), candidates_data.size(), false};

        llama_sample_softmax(ctx, &candidates);  // Compute softmax probabilities
        llama_token new_token_id = llama_sample_token(ctx, &candidates);

        // append the new token to the embedding
        embd.push_back(new_token_id);
        output_ss << llama_token_to_piece(ctx, new_token_id);

        // decrement the remaining tokens and predict
        --n_remain;

        // stop if eos token is generated
        if(new_token_id == llama_token_eos(model)){
            break;
        }
    }

    oAnswer = output_ss.str();
    conversation_log.emplace_back(Author::User, question);
    conversation_log.emplace_back(Author::Model, oAnswer);
    return true;
}

bool Llama2Device::setPrompt(const std::string &prompt)
{
    //setting up the command for the prompt setting
    std::string aPrompt;
    gpt_params params;
    // total lenght of the sequence including the prompt
    const int n_len = 32;

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
    std::vector<llama_token> token_list;

    tokens_list = ::llama_tokenize(ctx, params.prompt, true);

    const int n_ctx = llama_n_ctx(ctx);
    const int n_kv_req = tokens_list.size() + (n_len - tokens_list.size());

    LOG_TEE("\n%s: n_len = %d, n_ctx = %d, n_kv_req = %d\n", __func__, n_len, n_ctx, n_kv_req);

    // make sure the KV cache is big enough to hold all the prompt and generated tokens
    if (n_kv_req > n_ctx) {
        LOG_TEE("%s: error: n_kv_req > n_ctx, the required KV cache size is not big enough\n", __func__);
        LOG_TEE("%s: either reduce n_len or increase n_ctx\n", __func__);
        return 1;
    }
    // llama_decode will output logits only for the last token of the prompt
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        LOG_TEE("%s: llama_decode() failed\n", __func__);
        return 1;
    }
    conversation_log.emplace_back(Author::User, prompt);

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
    oConversation = conversation_log;
    return true;
}

bool Llama2Device::deleteConversation() noexcept
{
    try {
        conversation_log.clear();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool Llama2Device::close()
{
    yCInfo(LLAMA2DEVICE) << "Close";
    return false;
}
void Llama2Device::help() {
    std::cout << "Llama2Device Methods:" << std::endl;
    std::cout << "1. setPrompt" << std::endl;
    std::cout << "   - Set the initial prompt for the model." << std::endl;
    std::cout << "2. ask" << std::endl;
    std::cout << "   - Ask a question to the model and get an answer." << std::endl;
    std::cout << "3. getConversation" << std::endl;
    std::cout << "   - Prints the current conversation with the model." << std::endl;
    std::cout << "4. deleteConversation" << std::endl;
    std::cout << "   - Deletes the current conversation" << std::endl;
    std::cout << "5. help" << std::endl;
    std::cout << "   - Prints the list of possible commands and their description." << std::endl;
}
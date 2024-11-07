/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
//#include <common/common.h>
//#include "llama.h"
#include "Llama2Device.h"
#include <fstream>
#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace yarp::os;
using namespace yarp::dev;

//llama_context *ctx = nullptr;

YARP_LOG_COMPONENT(LLAMA2DEVICE, "yarp.llama2Device", yarp::os::Log::TraceType);


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

    yCInfo(LLAMA2DEVICE) << "Open method";

    std::string model_path = "/home/leonardo/Repos/yarp-device-llama2/models/contexts/llama2/llama-2-7b-chat.Q2_K.gguf"; 
    init_LLM(model_path);

    std::string prompt = "Hello my name is";

    yarp::dev::LLM_Message answer;
    ask(prompt, answer);

    return true;
}

// method for the initialization of the LLM model
bool Llama2Device::init_LLM(const std::string &model_path)
{
    // number of layers to offload to the GPU
    int ngl = 99;
    // number of tokens to predict
    int n_predict = 32;
    // initialize the model
    model_params = llama_model_default_params();
    model_params.n_gpu_layers = ngl;

    model = llama_load_model_from_file(model_path.c_str(), model_params);

    // check if model is found
    if (model == NULL){
        yCError(LLAMA2DEVICE) << "Error: unable to find model!";
        return false;
    }
    else{
        yCInfo(LLAMA2DEVICE) << "Model correctly intialized";
    }

    return true;
}

bool Llama2Device::ask(const std::string &question, yarp::dev::LLM_Message &oAnswer)
{
    // debug parameters, these will need to be passed to the function
    std::string prompt = "Hello my name is";
    int ngl = 99;
    int n_predict = 32;
    
    // tokenize the prompt
    // find the number of tokens in the prompt
    yCInfo(LLAMA2DEVICE) << "line 104";
    yCInfo(LLAMA2DEVICE) << prompt;
    yCInfo(LLAMA2DEVICE) << prompt.size();
    const int n_prompt = -llama_tokenize(model, prompt.c_str(), prompt.size(), NULL, 0, true, true);
    yCInfo(LLAMA2DEVICE) << "line 105";
    // allocate space for the tokens and tokenize the prompt
    std::vector<llama_token> prompt_tokens(n_prompt);
    if(llama_tokenize(model, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0){
        yCError(LLAMA2DEVICE) << "Error: failed to tokenize the prompt";
        return false;
    }
    else{
        yCInfo(LLAMA2DEVICE) << "Prompt tokenized correctly";
    }

    // initialize the context
    llama_context_params ctx_params = llama_context_default_params();
    // n_ctx is the context size
    ctx_params.n_ctx = n_prompt + n_predict -1;
    // n_batch is the maximum number of tokens that can be processed in a single call to llama_decode
    ctx_params.n_batch = n_prompt;
    // enable performance counters
    ctx_params.no_perf = false;

    llama_context * ctx = llama_new_context_with_model(model, ctx_params);

    // check if context has been initialized correctly
    if(ctx == NULL){
        yCError(LLAMA2DEVICE) << "Error: failed to create the llama_context";
        return false;
    }
    else{
        yCInfo(LLAMA2DEVICE) << "Context correctly initialized";
    }

    // initialize the sampler
    auto sparams = llama_sampler_chain_default_params();
    sparams.no_perf = false;
    llama_sampler * smpl = llama_sampler_chain_init(sparams);

    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    //print the prompt token by token
    for(auto id: prompt_tokens){
        char buf[128];
        int n = llama_token_to_piece(model, id, buf, sizeof(buf), 0, true);
        if(n < 0){
            yCError(LLAMA2DEVICE) << "Error: failed to convert token to piece";
            return false;
        }
        std::string s(buf, n);
        yCInfo(LLAMA2DEVICE) << s;
    }

    // prepare a batch for the prompt
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());

    // main loop
    const auto t_main_start = ggml_time_us();
    int n_decode = 0;
    llama_token new_token_id;

    for (int n_pos = 0; n_pos + batch.n_tokens < n_prompt + n_predict;) {
        // evaluate the current batch with the transformer model
        if(llama_decode(ctx, batch)){
            yCError(LLAMA2DEVICE) << "Error: failed to eval";
            return false;
        }

        n_pos += batch.n_tokens;

        // sample the next token
        {
            new_token_id = llama_sampler_sample(smpl, ctx, -1);

            // check if it is the end of a generation
            if(llama_token_is_eog(model, new_token_id)){
                break;
            }

            char buf[128];
            int n = llama_token_to_piece(model, new_token_id, buf, sizeof(buf), 0, true);
            if(n < 0){
                yCError(LLAMA2DEVICE) << "Error: failed to convert token to piece";
                return false;
            }
            std::string s(buf, n);
            yCInfo(LLAMA2DEVICE) << s;
            //fflush(stdout);

            // prepare the next batch with the sampled token
            batch = llama_batch_get_one(&new_token_id, 1);

            n_decode += 1;
        }
    }

    yCInfo(LLAMA2DEVICE) << "\n";

    const auto t_main_end = ggml_time_us();

    yCInfo(LLAMA2DEVICE) << "%s: decoded %d tokens in %.2f s, speed: %.2f t/s\n",
            __func__, n_decode, (t_main_end - t_main_start) / 1000000.0f, n_decode / ((t_main_end - t_main_start) / 100000.0f);

    yCInfo(LLAMA2DEVICE) << "\n";
    llama_perf_sampler_print(smpl);
    llama_perf_context_print(ctx);
    yCInfo(LLAMA2DEVICE) << "\n";

    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_free_model(model);

    return true;



    /*
    // tokenize the question
    std::vector<llama_token> token_list = ::llama_tokenize(ctx, question, true);
    // prepare input tokens
    std::vector<llama_token> input_tokens = tokens_list;
    yCInfo(LLAMA2DEVICE) << "Question: " << question;
    input_tokens.push_back(llama_token_eos(model));
    int n_past = 0;
    //int n_remain = params.n_predict;
    int n_remain = 32;
    yCInfo(LLAMA2DEVICE) << n_remain;

    std::vector<llama_token> embd;
    std::ostringstream output_ss;
    yCInfo(LLAMA2DEVICE) << embd;

    while(n_remain > 0){
        if (!embd.empty()){
            // evaluate the current batch of tokens
            llama_batch batch;
            batch.token = embd.data();
            batch.logits = nullptr;
            batch.n_tokens = static_cast<int32_t>(embd.size());
             yCInfo(LLAMA2DEVICE) << llama_decode(ctx, batch);
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
        yCInfo(LLAMA2DEVICE) << embd;
        // decrement the remaining tokens and predict
        --n_remain;
        yCInfo(LLAMA2DEVICE) << n_remain;
        // stop if eos token is generated
        if(new_token_id == llama_token_eos(model)){
            break;
        }
    }
    */

    /*
    oAnswer.type =  "assistant";
    oAnswer.content = output_ss.str();
    oAnswer.parameters.clear();
    oAnswer.arguments.clear();

    oAnswer = yarp::dev::LLM_Message{"assistant", output_ss.str(), std::vector<std::string>(), std::vector<std::string>()};
    std::pair log{Author::User, question};
    std::pair log2{Author::Model, oAnswer.content};
    // oAnswer = output_ss.str();
    //conversation_log.emplace_back(Author::User, Content(question));
    //conversation_log.emplace_back(Author::Model, oAnswer);
    yCInfo(LLAMA2DEVICE) << "Answer: " << oAnswer.content;
    //yCInfo(LLAMA2DEVICE) << output_ss.str();
    yCInfo(LLAMA2DEVICE) << n_remain;
    conversation_log.emplace_back(log);
    conversation_log.emplace_back(log2);
    return true;
    */
}

bool Llama2Device::setPrompt(const std::string &prompt)
{
    /*
    //setting up the command for the prompt setting
    std::string aPrompt;
    //gpt_params params;
    // total lenght of the sequence including the prompt
    const int n_len = 32;

    if(readPrompt(aPrompt))
    {
        yError() << "A prompt already set";
        return false;
    }

    try
    {
        model_params.prompt = prompt;
    }
    catch(const std::exception& e)
    {
        yError() << e.what() << '\n';
        return false;
    }
    // tokenize the prompt
    std::vector<llama_token> token_list;

    tokens_list = ::llama_tokenize(ctx, model_params.prompt, true);

    const int n_ctx = llama_n_ctx(ctx);
    const int n_kv_req = tokens_list.size() + (n_len - tokens_list.size());

    //LOG_TEE("\n%s: n_len = %d, n_ctx = %d, n_kv_req = %d\n", __func__, n_len, n_ctx, n_kv_req);

    // make sure the KV cache is big enough to hold all the prompt and generated tokens
    if (n_kv_req > n_ctx) {
        //LOG_TEE("%s: error: n_kv_req > n_ctx, the required KV cache size is not big enough\n", __func__);
        //LOG_TEE("%s: either reduce n_len or increase n_ctx\n", __func__);
        return 1;
    }
    // llama_decode will output logits only for the last token of the prompt
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        LOG_TEE("%s: llama_decode() failed\n", __func__);
        return 1;
    }
    conversation_log.emplace_back(Author::User, prompt);
    */
    return true;
}

bool Llama2Device::readPrompt(std::string &oPrompt)
{   /*
    // print the prompt token-by-token

    fprintf(stderr, "\n");

    for (auto id : tokens_list) {
        fprintf(stderr, "%s", llama_token_to_piece(ctx, id).c_str());
    }

    fflush(stderr);
    */
    return true;
}

bool Llama2Device::getConversation(std::vector<yarp::dev::LLM_Message> &oConversation)
{
    std::vector<yarp::dev::LLM_Message> conversation;
    oConversation = conversation;
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
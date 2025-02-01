/*
 * SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "llama.h"
#include "Llama2Device.h"
#include <fstream>
#include <yarp/os/LogComponent.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>
#include "common.h"
#include "arg.h"
#include "sampling.h"
#include <iostream>
#include "json.hpp"
#include <curl/curl.h>

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

using json = nlohmann::json;

std::string extractContent(const std::string& jsonResponse) {
    try {
        json parsedJson = json::parse(jsonResponse);  // Parsing del JSON
        return parsedJson["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Errore nel parsing del JSON: " << e.what() << std::endl;
        return "";
    }
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string sendPostRequest(const std::string& jsonData) {
    yCInfo(LLAMA2DEVICE) << "Inside sendPostRequest";
    CURL *curl;
    CURLcode res;
    std::string responseString; 

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        const char* url = "http://localhost:8080/v1/chat/completions";
        yCInfo(LLAMA2DEVICE) << "Invio richiesta a: " << url;
        yCInfo(LLAMA2DEVICE) << "Dati JSON: " << jsonData;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // DEBUG: Abilita il logging di cURL
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            yCInfo(LLAMA2DEVICE) << "Errore nella richiesta: " << curl_easy_strerror(res);
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        yCInfo(LLAMA2DEVICE) << "Codice risposta HTTP: " << response_code;

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return responseString;
}


bool Llama2Device::open(yarp::os::Searchable &config)
{
    if (!parseParams(config))  { return false; }
    // initialize LLM
    init_LLM(m_model_name);

    return true;
}

// method for the initialization of the LLM model
bool Llama2Device::init_LLM(const std::string &model_path)
{
    // number of layers to offload to the GPU
    ngl = 99;
    // number of tokens to predict
    n_predict = 64;
    // initialize the model
    /*g_params = &params;
    params.model = m_model_name;
    params.cpuparams.n_threads = 6;
    params.n_ctx = 131072;
    params.n_batch = 2048;
    params.n_ubatch = 512;
    params.n_predict = 15;
    params.rope_freq_base = 500000.0;
    params.cpuparams_batch.n_threads = 6;*/
    

    // Comando per avviare llama-server
    std::string command = "./build/bin/llama-server -m " + model_path + " -c 2048 &";
    
    // Avvia il server in background
    int status = std::system(command.c_str());
    
    if (status == 0) {
        yCInfo(LLAMA2DEVICE) << "llama-server avviato con successo!";
    } else {
        yCInfo(LLAMA2DEVICE) << "Errore nell'avvio di llama-server";
        return 1;
    }

    //std::string risposta = sendPostRequest();
    //yCInfo(LLAMA2DEVICE) << risposta;

    return true;
}

bool Llama2Device::ask(const std::string &question, yarp::dev::LLM_Message &oAnswer)
{
    /*ask_question = question;
    model_question = question;
    // if prompt is set, add it to the question
    if(prompt_set == true){
        if(first_prompt_set == true){
            model_question = m_prompt.content;
            model_question += " " + ask_question;
        }
        first_prompt_set = false;
    }
    else {
        model_question = question;
    }
    nlohmann::json messages = {
        {{"role", "system"}, {"content", "You are an automated answerer. You can only answer yes or no."}},
        {{"role", "user"}, {"content", "Is the sun a star?"}},
        {{"role", "assistant"}, {"content", "Yes."}},
        {{"role", "user"}, {"content", "Is the moon a star?"}},
        {{"content", "No"},{"role", "assistant"}},
    };

    model_question = messages.dump();

    yCInfo(LLAMA2DEVICE) << model_question;*/

    nlohmann::json request_json;
    request_json["model"] = "default";
    request_json["max_tokens"] = 128;

    // Inserisci la conversazione precedente
    nlohmann::json messages_array = nlohmann::json::array();
    for (const auto& msg : m_conversation) {
        messages_array.push_back({{"role", msg.type}, {"content", msg.content}});
    }

    // Aggiungi la nuova domanda alla conversazione
    messages_array.push_back({{"role", "user"}, {"content", question}});

    // Inserisci l'array di messaggi nella richiesta
    request_json["messages"] = messages_array;

    // Stampa per debug
    yCInfo(LLAMA2DEVICE) << "JSON Request: " << request_json.dump();

    // Converte il JSON in stringa e invia la richiesta
    std::string risposta = sendPostRequest(request_json.dump());

    // Analizza la risposta del modello
    std::string final_output = extractContent(risposta);

    // add question to the conversation
    yarp::dev::LLM_Message message;
    message.type = "user";
    message.content = question;
    m_conversation.push_back(message);

    // variable used to store the complete answer generated by the model
    final_output = extractContent(risposta);

    
    yCInfo(LLAMA2DEVICE) << final_output;
    yCInfo(LLAMA2DEVICE) << risposta;


    // add model answer to the conversation 
    message.type = "assitant";
    message.content = final_output;
    m_conversation.push_back(message);

    // write model answer inside oAnswer
    oAnswer.type = "assistant";
    oAnswer.content = final_output;

    return true;
}


bool Llama2Device::setPrompt(const std::string &prompt)
{
    // check if a prompt is already set
    if(prompt_set == true){
        yCError(LLAMA2DEVICE)<< "A prompt is already set. You must delete conversation first";
        return false;
    }
    // if the prompt is not already set, set the prompt
    prompt_set = true;
    first_prompt_set = true;

    m_prompt.type = "system";
    m_prompt.content = prompt;

    // add prompt to the conversation
    yarp::dev::LLM_Message message;
    message.type = "system";
    message.content = prompt;
    m_conversation.push_back(message);
    
    return true;
}


bool Llama2Device::readPrompt(std::string &oPrompt)
{   
    // check if the prompt is set
    if(prompt_set == false){
        yCError(LLAMA2DEVICE) << "Prompt is not set";
        return false;
    }
    // if prompt is set, read it
    oPrompt = m_prompt.content;

    return true;
}


bool Llama2Device::getConversation(std::vector<yarp::dev::LLM_Message> &oConversation)
{
    // check if the conversation is empty
    if(m_conversation.empty()){
        yCInfo(LLAMA2DEVICE) << "The conversation is empty";
        return false;
    }

    // if the conversation is not empty, print it
    for (const auto& msg : oConversation) {
        yCInfo(LLAMA2DEVICE) << "Role: " << msg.type;
        yCInfo(LLAMA2DEVICE) << "Content: " << msg.content;
        yCInfo(LLAMA2DEVICE) << "-------------------";
    }
    
    // set the value of the conversation
    oConversation = m_conversation;
    return true;
}


bool Llama2Device::deleteConversation() noexcept
{
    // check if the conversation is empty
    if (m_conversation.empty()){
        yCInfo(LLAMA2DEVICE) << "Conversation is already empty";
        return false;
    }
    // if not empty, clear the conversation
    m_conversation.clear();
    prompt_set = false;
    first_prompt_set = false;

    yCInfo(LLAMA2DEVICE) << "Conversation deleted";
    return true;
}

bool Llama2Device::refreshConversation() noexcept
{
    return true;
}

bool Llama2Device::close()
{
    yCInfo(LLAMA2DEVICE) << "Closing";
    return true;
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
    std::cout << "5. readPrompt" << std::endl;
    std::cout << "   - Prints the last prompt provided to the model" << std::endl;
    std::cout << "6. help" << std::endl;
    std::cout << "   - Prints the list of possible commands and their description." << std::endl;
}

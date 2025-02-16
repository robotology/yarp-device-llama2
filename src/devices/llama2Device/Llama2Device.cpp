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
 
 #include <sys/types.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <cstdlib>
 #include <cstring>
 #include <signal.h>
 
 
 using namespace yarp::dev;
 
 YARP_LOG_COMPONENT(LLAMA2DEVICE, "yarp.llama2Device", yarp::os::Log::TraceType);
 
 Llama2Device::Llama2Device()
 {
 
 }
 
 using json = nlohmann::json;
 
 std::string extractContent(const std::string& jsonResponse) {
     try {
         json parsedJson = json::parse(jsonResponse);  // JSON parsing
         return parsedJson["choices"][0]["message"]["content"].get<std::string>();
     } catch (const std::exception& e) {
         std::cerr << "Error in parsing JSON: " << e.what() << std::endl;
         return "";
     }
 }
 
 size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
     ((std::string*)userp)->append((char*)contents, size * nmemb);
     return size * nmemb;
 }
 
 std::string sendPostRequest(const std::string& jsonData) {
     CURL *curl;
     CURLcode res;
     std::string responseString;
 
     curl_global_init(CURL_GLOBAL_ALL);
     curl = curl_easy_init();
 
     if (curl) {
         const char* url = "http://localhost:8080/v1/chat/completions";
         yCInfo(LLAMA2DEVICE) << "Sending request to: " << url;
         yCInfo(LLAMA2DEVICE) << "JSON data: " << jsonData;
 
         curl_easy_setopt(curl, CURLOPT_URL, url);
         curl_easy_setopt(curl, CURLOPT_POST, 1);
         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
 
         struct curl_slist *headers = NULL;
         headers = curl_slist_append(headers, "Content-Type: application/json");
         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
 
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
 
         // DEBUG: enable cURL logging
         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
         res = curl_easy_perform(curl);
 
         if (res != CURLE_OK) {
             yCInfo(LLAMA2DEVICE) << "Error in the request: " << curl_easy_strerror(res);
         }
 
         long response_code;
         curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
         yCInfo(LLAMA2DEVICE) << "HTTP response code: " << response_code;
 
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
 
 
 void startLlama2Server(const std::string &model_path, int m_context, int m_gpulayers) {
     pid_t pid = fork();
 
     if (pid == 0) { // Child process
         prctl(PR_SET_PDEATHSIG, SIGKILL); // Auto-kill if parent exits
 
         // Prepare arguments for execvp
         std::string exe_path = "llama-server";
         std::string arg_m = "-m";
         std::string arg_c = "-c";
         std::string arg_ngl = "-ngl";
         std::string arg_context = std::to_string(m_context);
         std::string arg_nglvalue = std::to_string(m_gpulayers);

         char *args[] = {
             (char *)exe_path.c_str(),
             (char *)arg_m.c_str(),
             (char *)model_path.c_str(),
             (char *)arg_c.c_str(),
             (char *)arg_context.c_str(),
             (char *)arg_ngl.c_str(),
             (char *)arg_nglvalue.c_str(),
             nullptr
         };
 
         execvp(args[0], args);
         std::cerr << "Error: Failed to start llama2-server!" << std::endl;
         std::exit(1);
     } else if (pid > 0) { // Parent process
         std::cout << "llama2-server started in background with PID: " << pid << std::endl;
     } else {
         std::cerr << "Error: fork() failed!" << std::endl;
     }
 }
 
 // method for the initialization of the LLM model
 bool Llama2Device::init_LLM(const std::string &model_path)
 {
     std::string command;
     // if(m_offload_gpu == true){
     //     // command to start llama-server with gpu offload enabled
     //     command = "./build/bin/llama-server -m " + model_path + " -c " + std::to_string(m_context) + " -ngl " + std::to_string(m_ngl) + " &";
     //     yCInfo(LLAMA2DEVICE) << command;
     // }
     // else{
     //     // command to start llama-server without gpu offload enabled
     //     command = "./build/bin/llama-server -m " + model_path + " -c " + std::to_string(m_context) + " &";
     //     yCInfo(LLAMA2DEVICE) << command;
     // }
 
     // start llama-server in background
     startLlama2Server(model_path, m_context, m_ngl);
 
     // launch llama-server in background
     int status = std::system(command.c_str());
 
     if (status == 0) {
         yCInfo(LLAMA2DEVICE) << "llama-server started succesfully!";
     } else {
         yCError(LLAMA2DEVICE) << "Error starting llama-server";
         return 1;
     }
 
     return true;
 }
 
 bool Llama2Device::ask(const std::string &question, yarp::dev::LLM_Message &oAnswer)
 {
     nlohmann::json request_json;
     request_json["model"] = "default";
     request_json["max_tokens"] = m_npredict;
 
     // Pass the conversation to the model
     nlohmann::json messages_array = nlohmann::json::array();
     for (const auto& msg : m_conversation) {
         messages_array.push_back({{"role", msg.type}, {"content", msg.content}});
     }
 
     // Add new question to the conversation
     messages_array.push_back({{"role", "user"}, {"content", question}});
 
     // Insert array messages into the request
     request_json["messages"] = messages_array;
 
     // Debug print
     yCInfo(LLAMA2DEVICE) << "JSON Request: " << request_json.dump();
 
     // Convert JSON to string and send the request to the model
     std::string risposta = sendPostRequest(request_json.dump());
 
     // extract model's answer
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
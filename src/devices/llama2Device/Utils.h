#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include "json.hpp"
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>

using json = nlohmann::json;

void startLlama2Server(const std::string &model_path, int m_context, int m_gpulayers);
std::string extractContent(const std::string& jsonResponse);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string sendPostRequest(const std::string& jsonData);

#endif // UTILS_H

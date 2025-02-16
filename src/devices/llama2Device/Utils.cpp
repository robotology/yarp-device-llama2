#include "Utils.h"

std::string extractContent(const std::string& jsonResponse) {
    try {
        json parsedJson = json::parse(jsonResponse);
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
        std::cout << "Sending request to: " << url << std::endl;
        std::cout << "JSON data: " << jsonData << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // DEBUG mode

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error in the request: " << curl_easy_strerror(res) << std::endl;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        std::cout << "HTTP response code: " << response_code << std::endl;

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return responseString;
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
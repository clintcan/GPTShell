#include <iostream>
#include <cstring>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include <unistd.h>
#include <limits.h>
#include <fstream>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <csignal>

using namespace std;
using namespace rapidjson;

int exitflag = 0;

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realsize = size * nmemb;
    string* response = reinterpret_cast<std::string*>(userdata);
    response->append(ptr, realsize);
    return realsize;
}

void sig_handler(int sig) {
    if (exitflag > 0) {
        exit(0);
    }
    cout << endl << "CTRL+C pressed with sig" << sig << ". Press again to end program." << endl;
    exitflag++;
}

string chat_gpt(const std::string& prompt, const std::string& api_key, const std::string& api_model) {
    CURL* curl = curl_easy_init();
    string response;

    if (curl) {
        const char* url = "https://api.openai.com/v1/completions";

        rapidjson::Document json_payload;
        json_payload.SetObject();
        rapidjson::Document::AllocatorType& allocator = json_payload.GetAllocator();
        json_payload.AddMember("prompt", rapidjson::StringRef(prompt.c_str()), allocator);
        json_payload.AddMember("max_tokens", 2048, allocator);
        json_payload.AddMember("model", rapidjson::StringRef(api_model.c_str()), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json_payload.Accept(writer);

        struct curl_slist* headers = NULL;
        char auth_header[500];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key.c_str());
        headers = curl_slist_append(headers, auth_header);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer.GetString());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.78.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Error: curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return response;
}

int main() {
    string prompt;
    string api_key;
    string api_model;
    char cwd[PATH_MAX];
    char* user;
    string user_str;
    char host[HOST_NAME_MAX];
    char* gptapikey;
    char* gptmodel;
    string gpt_prompt;
    string response;
    rapidjson::Document doc;
    string command;
    string bash_prompt;

    // We will catch CTRL+C here 
    signal(SIGINT, sig_handler);

    while(true) {
        exitflag = 0;
        gptapikey = getenv("GPTAPIKEY");
        if (gptapikey != nullptr) {
            api_key = gptapikey;
        } else {
            ifstream infile("/etc/gptshell");
            if (infile.good()) {
                string line;
                while (getline(infile, line)) {
                    if (line.find("GPTAPIKEY=") == 0) {
                        api_key = line.substr(strlen("GPTAPIKEY="));
                        break;
                    }
                }
            }
        }

        // This is for GPT model
        gptmodel = getenv("GPTMODEL");
        api_model = "";
        if (gptmodel != nullptr) {
            api_model = gptmodel;
        } else {
            ifstream infile("/etc/gptshell");
            if (infile.good()) {
                string line;
                while (getline(infile, line)) {
                    if (line.find("GPTMODEL=") == 0) {
                        api_model = line.substr(strlen("GPTMODEL="));
                        break;
                    }
                }
            }
        }
        if(api_model.empty()) {
            api_model = "text-davinci-003"; // if you need turbo, use gpt-3.5-turbo-0301, or text-davinci-003 for the legacy version
        }

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            std::cerr << "Error: getcwd() failed" << std::endl;
        }
        user = getenv("USER"); //getlogin
        user_str = (user != NULL) ? string(user) : "unknown";
        if (gethostname(host, sizeof(host)) != 0) {
            cerr << "Error: gethostname() failed" << endl;
        }

        bash_prompt = user_str + "@" + std::string(host) + ":" + std::string(cwd) + " $ ";
        std::cout << "\033[1m" << bash_prompt << "\033[0m";
        std::getline(std::cin, prompt);

        if(prompt == "exit") {
            break;
        }

        if (strncmp(prompt.c_str(), "/prompt ", 7) == 0) {
            if (api_key.empty()) {
                std::cout << "Error: could not find GPTAPIKEY" << std::endl;
                continue;
            }
            gpt_prompt = prompt.substr(7);
            response = chat_gpt(gpt_prompt, api_key, api_model);
            doc.Parse(response.c_str());

            if (doc.HasParseError()) {
                std::cerr << "Error: JSON parse error" << std::endl;
                continue;
            }

            if (doc.HasMember("choices") && doc["choices"].IsArray() &&
                !doc["choices"].Empty() &&
                doc["choices"][0].IsObject() &&
                doc["choices"][0].HasMember("text") &&
                doc["choices"][0]["text"].IsString()) {

                command = doc["choices"][0]["text"].GetString();
                std::cout << command << std::endl;
            } else {
                try {
                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    doc["choices"].Accept(writer);
                    cout << "Choices: " << buffer.GetString() << endl;
                    cerr << "Error: JSON response is not in the expected format" << endl;
                } catch (const char* err) {
                    cout << "Error in chatGPT communication. Please try again." << endl;
                }
            }
        } else if (strncmp(prompt.c_str(), "cd ", 3) == 0) {
            gpt_prompt = prompt.substr(3);
            chdir(gpt_prompt.c_str());
        } else {
            system(prompt.c_str());
        }
    }

    return 0;
}

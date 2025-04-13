#include "geminiAPI.h"
using json = nlohmann::json;


void callGeminiAPI(std::string prompt, std::string clipboardText, std::string GEMINI_KEY) {
    std::string finalPrompt = prompt + "\n\n" + clipboardText;

    // https://docs.libcpr.org/introduction.html#post-requests
    cpr::Response response = cpr::Post(
        cpr::Url{ "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + GEMINI_KEY },
        cpr::Body{ "{ \"contents\": [ { \"parts\": [ { \"text\": \"" + finalPrompt + "\" } ] } ] }" },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    std::cout << response.status_code << std::endl; // 200
    std::cout << response.header["content-type"] << std::endl; // application/json; charset=utf-8
    //std::cout << response.text << std::endl;

    // https://github.com/nlohmann/json?tab=readme-ov-file#serialization--deserialization
    json data = json::parse(response.text);
    std::cout << data.dump() << std::endl;
}
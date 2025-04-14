#pragma once
#include <string>
#include <vector>
#include <cpr/cpr.h>
#include "nlohmann/json.hpp"

class GeminiClient {
public:
    enum State {
        IDLE,
        RUNNING,
        FINISHED,
        FAILED 
    };

    enum PromptType {
        SYNONYMS,
        REPHRASE,
        FORMALIZE,
        ANTONYMS,
        UNGARBLE,
        SHORTEN,
        //
        HEADLINE,
        TAGLINE,
        ONEWORD,
        TWOWORD
    };

    std::string GEMINI_KEY;
    State state;
    cpr::Response httpResponse;
    std::string httpFeedback;
    std::vector<std::string> suggestions;

    GeminiClient(std::string apiKey);

    void reset();

    bool callAPI(std::string prompt, std::string clipboardText);

    void processResponse(cpr::Response response);

    std::string getPrompt(PromptType type);
};
#include "geminiAPI.hpp"
#include <iostream>

using json = nlohmann::json;

// initializer list
GeminiClient::GeminiClient(std::string apiKey) :
    GEMINI_KEY(apiKey),
    state(State::IDLE) {}

void GeminiClient::reset() {
    state = State::IDLE;
    suggestions = "";
}

bool GeminiClient::callAPI(std::string prompt, std::string clipboardText) {
    if (state != State::IDLE) {
        return false;
    }
    state = State::RUNNING;

    std::string finalPrompt = prompt + "\n\n" + clipboardText;

    std::cout << "!!! CALLING API" << std::endl;
    cpr::PostCallback(
        // c++ lambda
        [this](cpr::Response response) {
            std::cout << "!!! CPR CALLBACK FIRING" << std::endl;
            processResponse(response);
        },
        cpr::Url{ "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + GEMINI_KEY },
        cpr::Body{ "{ \"contents\": [ { \"parts\": [ { \"text\": \"" + finalPrompt + "\" } ] } ] }" },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    return true;
}

void GeminiClient::processResponse(cpr::Response response) {
    std::cout << "!!! PROCESSING" << std::endl;

    if (response.status_code != 200) {
        state = State::FAILED;
        std::cout << "!!! FAILED?" << response.status_code << std::endl;
        std::cout << response.text << std::endl;
        return;
    }

    json data = json::parse(response.text);
    suggestions = data["candidates"][0]["content"]["parts"][0]["text"];
    state = State::FINISHED;

    std::cout << "!!!!!!!!!!! GEMINI RESPONSE" << suggestions << std::endl;
}

std::string GeminiClient::getPrompt(PromptType type) {
    switch (type) {
    case PromptType::SYNONYMS:
        return "If the following text snippet is a single word or a two word phrase, give me 10 synonyms for it. Otherwise give me 10 ways to say something similar.";
        break;
    case PromptType::REPHRASE:
        return "Give me 10 other ways to rephrase the following text snippet:";
        break;
    case PromptType::FORMALIZE:
        return "Make the following text snippet more formal: ";
        break;
    case PromptType::ANTONYMS:
        return "If the following text snippet is a single word or a two word phrase, give me 10 antonyms for it. Otherwise give me 10 ways to say the opposite thing while still keeping the same intent.";
        break;
    case PromptType::UNGARBLE:
        return "This snippet of text doesn't sound quite right, please rewrite it while keeping the same tone, intent, and meaning.";
        break;
    case PromptType::SHORTEN:
        return "Make the following text snippet shorter:";
        break;
        // -----
    case PromptType::HEADLINE:
        return "Turn the following text snippet into a headline, like, for example, the title for a blog post or web page:";
        break;
    case PromptType::TAGLINE:
        return "Rewrite the following text snipport into a tag line:";
        break;
    case PromptType::ONEWORD:
        return "Rewrite the following text snippet into a one word phrase that encapsulates the same meaning:";
        break;
    case PromptType::TWOWORD:
        return "Rewrite the following text snippet into two word phrase that encapsulates the same meaning:";
        break;
    default:
        return "";
        break;
    }
}
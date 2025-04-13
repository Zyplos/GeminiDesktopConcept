#pragma once
#include <iostream>
#include <cpr/cpr.h>
#include "nlohmann/json.hpp"

void callGeminiAPICallTest();

void callGeminiAPI(std::string prompt, std::string clipboardText, std::string GEMINI_KEY);
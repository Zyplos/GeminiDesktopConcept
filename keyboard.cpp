#include "keyboard.h"
#include <iostream>

// Function to save the current clipboard content (simplified for text)
bool SaveClipboard(SavedClipboardData& savedData) {
    savedData.format = 0;
    savedData.data.clear();

    if (!OpenClipboard(nullptr)) {
        // std::cerr << "Error: Cannot open clipboard for saving." << std::endl;
        return false;
    }

    // Check for Unicode text first (preferred)
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText != nullptr) {
                size_t len = wcslen(pText);
                size_t bufferSize = (len + 1) * sizeof(wchar_t);
                savedData.data.resize(bufferSize);
                memcpy(savedData.data.data(), pText, bufferSize);
                savedData.format = CF_UNICODETEXT;
                GlobalUnlock(hData);
            }
        }
    }
    // Add checks for other formats if needed (e.g., CF_TEXT, CF_BITMAP...)
    // This simplified version only saves Unicode text.

    CloseClipboard();
    return savedData.format != 0 || IsClipboardFormatAvailable(0); // Return true if something was saved OR clipboard was empty
}

// Function to restore clipboard content from saved data
bool RestoreClipboard(const SavedClipboardData& savedData) {
    if (!OpenClipboard(nullptr)) {
        // std::cerr << "Error: Cannot open clipboard for restoring." << std::endl;
        return false;
    }

    EmptyClipboard(); // Clear current content

    if (savedData.format != 0 && !savedData.data.empty()) {
        // Allocate global memory and copy saved data into it
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, savedData.data.size());
        if (hGlobal != nullptr) {
            void* pGlobal = GlobalLock(hGlobal);
            if (pGlobal != nullptr) {
                memcpy(pGlobal, savedData.data.data(), savedData.data.size());
                GlobalUnlock(hGlobal);

                // Set the clipboard data (ownership transfers to clipboard)
                if (!SetClipboardData(savedData.format, hGlobal)) {
                    // std::cerr << "Error: SetClipboardData failed." << std::endl;
                    GlobalFree(hGlobal); // Free memory if SetClipboardData failed
                }
            }
            else {
                GlobalFree(hGlobal); // Free memory if GlobalLock failed
            }
        }
        else {
            // std::cerr << "Error: GlobalAlloc failed during restore." << std::endl;
        }
    }
    // If savedData was empty, the clipboard remains empty, which is correct.

    CloseClipboard();
    return true;
}

// Function to simulate Ctrl+C key presses
void SimulateCtrlC() {
    Sleep(100);
    std::cout << "SENDING CTRL + C" << std::endl;
    INPUT inputs[4] = {};

    // Press CTRL using scan code
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[0].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;  // using scan code

    // Press C using scan code
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';
    inputs[1].ki.wScan = MapVirtualKey('C', MAPVK_VK_TO_VSC);
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE;

    // Release C
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.wScan = MapVirtualKey('C', MAPVK_VK_TO_VSC);
    inputs[2].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    // Release CTRL
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputs[3].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    UINT eventsSent = SendInput(4, inputs, sizeof(INPUT));;
    if (eventsSent != ARRAYSIZE(inputs)) {
        // Log this error! Use OutputDebugString or your preferred logging
        std::cout << "Error: SendInput failed to inject all key events!\n";
    }
    else {
        std::cout << "SendInput succeeded: Injected 4 key events.\n";
    }
}

// Function to get text from clipboard
std::wstring GetClipboardText() {
    std::wstring text = L"";
    if (!OpenClipboard(nullptr)) {
        return text; // Error or clipboard busy
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText != nullptr) {
                try {
                    text = pText; // Assign to std::wstring
                }
                catch (const std::bad_alloc&) {
                    // Handle memory allocation failure if necessary
                    text = L""; // Or some error indicator
                }
                GlobalUnlock(hData);
            }
        }
    }
    // Could add fallback for CF_TEXT here if needed

    CloseClipboard();
    return text;
}

// runs all the stuff needed to swap clipboard contents and get highlighted text
std::string getFinalClipboardString(HWND overlayWindow) {
    HWND foregroundWindow = GetForegroundWindow();

    // Don't do anything if the overlay itself is focused
    if (foregroundWindow == overlayWindow || foregroundWindow == NULL) {
        return "";
    }

    // 1. Save current clipboard
    SavedClipboardData originalClipboard;
    if (!SaveClipboard(originalClipboard)) {
        // Handle error - maybe clipboard is in use by another app
        // std::cerr << "Failed to save clipboard state." << std::endl;
        return "";
    }






    // Inside OnAltQHotkey, before SimulateCtrlC
    HWND targetHwnd = GetForegroundWindow();
    if (targetHwnd) {
        wchar_t windowTitle[256] = { 0 };
        GetWindowTextW(targetHwnd, windowTitle, ARRAYSIZE(windowTitle));
        // Use OutputDebugStringW for easy viewing in DebugView or VS Output window
        std::cout << "Targeting window for Ctrl+C: ";
        std::wcout << windowTitle;
        std::cout << "\n";
    }
    else {
        std::cout << "GetForegroundWindow returned NULL!\n";
        // Maybe handle this error case - don't proceed
        return "";
    }
    // Ensure we don't target ourselves
    if (targetHwnd == overlayWindow) {
        std::cout << "Target window is the overlay itself. Skipping.\n";
        RestoreClipboard(originalClipboard); // Don't forget to restore if you bail early
        return "";
    }

    if (targetHwnd && targetHwnd != overlayWindow) {
        std::cout << "!!! DEBUG CALLING SimulateCtrlC" << std::endl;
        // Attempt to ensure the target window is foreground.
        // This might fail depending on permissions and foreground rights.
        SetForegroundWindow(targetHwnd);
        SetFocus(targetHwnd);
        // Add a very tiny sleep AFTER setting focus to allow the OS to process it
        Sleep(20); // e.g., 20ms - adjust if needed

        OutputDebugStringW(L"Sending Ctrl+C input...\n");
        SimulateCtrlC();
        //SendMessage(targetHwnd, WM_COPY, 0, 0);
    }
    else {
        // 2. Simulate Ctrl+C
        std::cout << "CALLING SimulateCtrlC" << std::endl;
        SimulateCtrlC();
        //SendMessage(targetHwnd, WM_COPY, 0, 0);
    }








    

    // 3. Wait briefly and Read New Clipboard
    // IMPORTANT: SendInput is asynchronous. Need a short delay.
    // This value might need tuning! Too short = no text. Too long = laggy.
    Sleep(50); // Wait 50 milliseconds (adjust as needed)

    std::wstring capturedWideText = GetClipboardText();

    if (capturedWideText.empty()) {
        return std::string();
    }

    std::cout << "WIDESTRING" << std::endl;
    std::wcout << capturedWideText << std::endl;

    // 4. Restore Original Clipboard
    RestoreClipboard(originalClipboard);

    // https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte?redirectedfrom=MSDN
    // Determine the required buffer size for the UTF-8 string
    // CP_UTF8: Use the UTF-8 code page
    // 0: Default flags
    // wstr.c_str(): Pointer to the wide-character string
    // -1: Indicates the string is null-terminated, let the function calculate the length
    // NULL: No buffer provided yet, we're asking for the size
    // 0: Buffer size is 0
    // NULL: Not using default char
    // NULL: Not tracking if default char was used
    int newStringSize = WideCharToMultiByte(CP_UTF8, 0, capturedWideText.c_str(), -1, NULL, 0, NULL, NULL);
    if (newStringSize == 0) return "";

    std::string capturedText;
    capturedText.resize(newStringSize);
    
    int result   = WideCharToMultiByte(CP_UTF8, 0, capturedWideText.c_str(), -1, &capturedText[0], newStringSize, NULL, NULL);
    // error converting wstring to string
    if (result == 0) return "";

    capturedText.pop_back();

    return capturedText;
}
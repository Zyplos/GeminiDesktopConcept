#ifndef CLIPBOARD_UTILS_H
#define CLIPBOARD_UTILS_H

#include <Windows.h>
#include <string>
#include <vector>

// Structure to hold saved clipboard data (simplified for text)
struct SavedClipboardData {
    UINT format = 0;
    std::vector<BYTE> data; // Use vector for dynamic size
};

// Saves the current clipboard content (Unicode text only in this version)
// Returns true if something was saved, or if clipboard was empty
bool SaveClipboard(SavedClipboardData& savedData);

// Restores the clipboard content from saved data
// Returns true if the operation completed (even if clipboard ends up empty)
bool RestoreClipboard(const SavedClipboardData& savedData);

// Simulates pressing Ctrl+C (used to copy selected text)
void SimulateCtrlC();

// Retrieves Unicode text from the clipboard
// Returns an empty wstring on failure or if clipboard doesn't contain text
std::wstring GetClipboardText();

// Handles the hotkey event for Alt+Q: captures selected text from foreground window
std::string getFinalClipboardString(HWND overlayWindow);

#endif // CLIPBOARD_UTILS_H

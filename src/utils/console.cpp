#include "console.hpp"
#include "imgui.h"
#include <vector>
#include <iostream>

using namespace std;

std::queue<Console::ConsoleMessage> Console::messages{};
bool Console::scrollToBottom = true;

Console::ConsoleMessage Console::constructMessage(const string& message, const string& source, ConsoleMessage::Type type)
{
    string result = "";
    if(source != "")
    {
        result += "[" + source + "] ";
    }
    result += message;
    ConsoleMessage newMessage{result, type};
    return newMessage;
}

void Console::pushMessage(ConsoleMessage& message)
{
    messages.push(message);
    if(messages.size() > maxMessages)
    {
        messages.pop();
    }
}

void Console::logRaw(const string& message, bool terminalOnly)
{
    ConsoleMessage newMessage{message, ConsoleMessage::NONE};
    cout << message << endl; // Print to standard output
    if(!terminalOnly)
        pushMessage(newMessage);
}

void Console::log(const string& message, const string& source, bool terminalOnly)
{
    ConsoleMessage newMessage = constructMessage(message, source, ConsoleMessage::INFO);
    cout << ANSIgray << "[INFO] " << ANSIreset << newMessage.message << consoleEndl; // Print to standard output
    if(!terminalOnly)
        pushMessage(newMessage);
}

void Console::debug(const string& message, const string& source, bool terminalOnly)
{
    ConsoleMessage newMessage = constructMessage(message, source, ConsoleMessage::DEBUG);
    cout << "[DEBUG] " << newMessage.message << consoleEndl; // Print to standard output
    if(!terminalOnly)
        pushMessage(newMessage);
}

void Console::warn(const string& message, const string& source, bool terminalOnly)
{
    ConsoleMessage newMessage = constructMessage(message, source, ConsoleMessage::WARNING);
    cout << ANSIyellow << "[WARNING] " << ANSIreset << newMessage.message << consoleEndl; // Print to standard output
    if(!terminalOnly)
        pushMessage(newMessage);
}

void Console::error(const string& message, const string& source, bool terminalOnly)
{
    ConsoleMessage newMessage = constructMessage(message, source, ConsoleMessage::ERROR);
    cout << ANSIred << "[ERROR] " << ANSIreset << newMessage.message << endl; // Print to standard output
    if(!terminalOnly)
        pushMessage(newMessage);
}

void Console::drawImGui()
{
    queue<ConsoleMessage> tempQueue = messages;
    vector<ConsoleMessage> messageVector{};

    while(!tempQueue.empty())
    {
        messageVector.push_back(tempQueue.front());
        tempQueue.pop();
    }

    // Get the main viewport size
    ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

    // Apply the position to the next window
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 60), ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_None);

    if(!ImGui::IsWindowCollapsed())
    {
        // Start a child window with scrolling
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        float spacing = 8;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, spacing));

        // Update scrollToBottom only if user is actively scrolling
        if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0)
        {
            scrollToBottom = (ImGui::GetScrollY() >= ImGui::GetScrollMaxY());
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float width = ImGui::GetContentRegionAvail().x;
        int counter = 0;
        for(const ConsoleMessage& message : messageVector)
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            // Draw filled rectangle behind the text
            if(counter % 2 == 1)
            {
                // Get size of the text
                ImVec2 textSize = ImGui::CalcTextSize(message.message.c_str());
                drawList->AddRectFilled(
                    ImVec2{pos.x, pos.y - spacing / 2},              // top-left
                    ImVec2{pos.x + width, pos.y + textSize.y + spacing / 2},   // bottom-right
                    IM_COL32(5, 5, 5, 63)  // color in RGBA 0-255
                );
            }
            counter++;

            switch(message.type)
            {
                case ConsoleMessage::INFO:
                    ImGui::TextColored(ImVec4(0.5,0.5,0.5,1), "[INFO]");
                    ImGui::SameLine();
                    break;

                case ConsoleMessage::DEBUG:
                    ImGui::TextColored(ImVec4(1,1,1,1), "[DEBUG]");
                    ImGui::SameLine();
                    break;
                
                case ConsoleMessage::WARNING:
                    ImGui::TextColored(ImVec4(1,1,0,1), "[WARNING]");
                    ImGui::SameLine();
                    break;
                
                case ConsoleMessage::ERROR:
                    ImGui::TextColored(ImVec4(1,0,0,1), "[ERROR]");
                    ImGui::SameLine();
                    break;

                default: // No prefix for type NONE
                    break;
            }
            
            ImGui::Text("%s", message.message.c_str());
        }
        
        if (scrollToBottom)
            ImGui::SetScrollHereY(1.0f);

        ImGui::PopStyleVar();

        ImGui::EndChild(); // End scroll region
    }

    ImGui::End();
}
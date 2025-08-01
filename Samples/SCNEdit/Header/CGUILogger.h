#include <imgui.h>
#include <TextSelect.h>


class CGUILogger
{   
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    TextSelect textEditor; // Use your simplified TextSelect editor
    int lineCount = 0;

    void Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
        textEditor.SetText(""); // Clear text in editor
        lineCount = 0;
    }


    private:
        void AddLogV(const char* fmt, va_list args)
        {
            int old_size = Buf.size();
            Buf.appendfv(fmt, args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
            {
                if (Buf[old_size] == '\n')
                {
                    LineOffsets.push_back(old_size + 1);
                }
            }
    
        }

    public:
        CGUILogger()
        {
            Clear();
            textEditor.SetReadOnlyEnabled(true);
            textEditor.SetShowWhitespacesEnabled(false);
        }


        void AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            va_list args;
            va_start(args, fmt);
            AddLogV(fmt, args);
            va_end(args);
        }

        void Draw(const char* title, bool* p_open = nullptr) {
            if (!ImGui::Begin(title, p_open)) {
                ImGui::End();
                return;
            }

            ImGui::TextUnformatted("Filter: ");
            ImGui::SameLine();
            Filter.Draw("##Filter", 0.0f);
            ImGui::Separator();

            // Apply filtering logic to build filtered text
            std::vector<std::string> filteredLines;
            for (int line_no = 0; line_no < LineOffsets.Size; ++line_no) {
                const char* line_start = Buf.begin() + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size)
                    ? (Buf.begin() + LineOffsets[line_no + 1] - 1)
                    : Buf.end();

                if (Filter.PassFilter(line_start, line_end)) {
                    filteredLines.emplace_back(line_start, line_end);
                }
            }

            // Set filtered text in the editor
            if (lineCount != filteredLines.size()) {
                textEditor.SetTextLines(filteredLines);
                lineCount = filteredLines.size();
                textEditor.SetViewAtLine(lineCount, TextSelect::SetViewAtLineMode::LastVisibleLine);
            }

            // Render your editor as read-only
            textEditor.Render("##FilteredLog", true, ImVec2(-FLT_MIN, -FLT_MIN), false);

            ImGui::End();
        }
};




#include "TextSelect.h"

#define IMGUI_SCROLLBAR_WIDTH 14.0f
#define POS_TO_COORDS_COLUMN_OFFSET 0.33f
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h" // for imGui::GetCurrentWindow()

TextSelect::TextSelect()
{
	mLines.push_back(Line());	
}

TextSelect::~TextSelect(){}

void TextSelect::SetTabSize(int aValue)
{
	mTabSize = std::max(1, std::min(8, aValue));
}

void TextSelect::SetLineSpacing(float aValue)
{
	mLineSpacing = std::max(1.0f, std::min(2.0f, aValue));
}

void TextSelect::SelectAll()
{
	ClearSelections();
	SetCursorPosition(Coordinates(0, 0), true);
	MoveBottom(true);
}
void TextSelect::SelectLine(int aLine)
{
	ClearSelections();
	SetSelection({ aLine, 0 }, { aLine, GetLineMaxColumn(aLine) });
}

void TextSelect::SelectRegion(int aStartLine, int aStartChar, int aEndLine, int aEndChar)
{
	ClearSelections();
	SetSelection(aStartLine, aStartChar, aEndLine, aEndChar);
}

void TextSelect::ClearSelections()
{
	mState.mCursor.mInteractiveEnd =
	mState.mCursor.mInteractiveStart =
	mState.mCursor.GetSelectionEnd();
}

void TextSelect::SetViewAtLine(int aLine, SetViewAtLineMode aMode)
{
	mSetViewAtLine = aLine;
	mSetViewAtLineMode = aMode;
}

void TextSelect::Copy()
{
	if (mState.mCursor.HasSelection())
	{
		std::string clipboardText = GetClipboardText();
		ImGui::SetClipboardText(clipboardText.c_str());
	}
	else
	{
		if (!mLines.empty())
		{
			std::string str;
			auto& line = mLines[GetSanitizedCursorCoordinates().mLine];
			for (auto& g : line)
				str.push_back(g.mChar);
			ImGui::SetClipboardText(str.c_str());
		}
	}
}

void TextSelect::Cut()
{
	if (mReadOnly)
	{
		Copy();
	}
	else
	{
		if (mState.mCursor.HasSelection())
		{
			UndoRecord u;
			u.mBefore = mState;

			Copy();
		
			u.mOperations.push_back({ GetText(mState.mCursor.GetSelectionStart(),
				mState.mCursor.GetSelectionEnd()),
				mState.mCursor.GetSelectionStart(), mState.mCursor.GetSelectionEnd(),
				UndoOperationType::Delete });
			DeleteSelection();
		

			u.mAfter = mState;
			AddUndo(u);
		}
	}
}


void TextSelect::Paste()
{
	if (mReadOnly)
		return;

	if (ImGui::GetClipboardText() == nullptr)
		return; // something other than text in the clipboard

	// check if we should do multicursor paste
	std::string clipText = ImGui::GetClipboardText();
	//bool canPasteToMultipleCursors = false;
	std::vector<std::pair<int, int>> clipTextLines;

	clipTextLines.push_back({ 0,0 });
	for (int i = 0; i < clipText.length(); i++)
	{
		if (clipText[i] == '\n')
		{
			clipTextLines.back().second = i;
			clipTextLines.push_back({ i + 1, 0 });
		}
	}
	clipTextLines.back().second = clipText.length();


	if (clipText.length() > 0)
	{
		UndoRecord u;
		u.mBefore = mState;

		if (mState.mCursor.HasSelection())
		{
		
			u.mOperations.push_back({ GetText(mState.mCursor.GetSelectionStart(), 
				mState.mCursor.GetSelectionEnd()),
				mState.mCursor.GetSelectionStart(), 
				mState.mCursor.GetSelectionEnd(), 
				UndoOperationType::Delete });
			DeleteSelection();
			
		}

		Coordinates start = GetSanitizedCursorCoordinates();
			
		InsertTextAtCursor(clipText.c_str());
		u.mOperations.push_back({ clipText, start, 
			GetSanitizedCursorCoordinates(), UndoOperationType::Add });


		u.mAfter = mState;
		AddUndo(u);
	}
}

void TextSelect::SetText(const std::string& aText)
{
	mLines.clear();
	mLines.emplace_back(Line());
	ImU32 currentColor = 0xFFFFFFFF;

	std::istringstream stream(aText);
	std::string line;
	std::regex hexRegex("^#([0-9a-fA-F]{6}|[0-9a-fA-F]{3})\\b");

	while (std::getline(stream, line)) {
		std::smatch match;
		size_t start = 0;
		if (std::regex_search(line, match, hexRegex)) {
			currentColor = ParseHexColor(match[1].str());
			start = match[0].length();
			// skip whitespace after hex
			if (start < line.size() && isspace(line[start])) ++start;
		}
		mLines.emplace_back(Line());
		for (size_t i = start; i < line.size(); ++i) {
			mLines.back().emplace_back(Glyph(line[i], currentColor));
		}
	}
	if (mLines.empty()) mLines.emplace_back(Line());


	mScrollToTop = true;

	mUndoBuffer.clear();
	mUndoIndex = 0;

}


std::string TextSelect::GetText() const
{
	auto lastLine = (int)mLines.size() - 1;
	auto lastLineLength = GetLineMaxColumn(lastLine);
	Coordinates startCoords = Coordinates();
	Coordinates endCoords = Coordinates(lastLine, lastLineLength);
	return startCoords < endCoords ? GetText(startCoords, endCoords) : "";
}

void TextSelect::SetTextLines(const std::vector<std::string>& aLines)
{
	mLines.clear();
	ImU32 currentColor = 0xFFFFFFFF;
	std::regex hexRegex("^#([0-9a-fA-F]{6}|[0-9a-fA-F]{3})\\b");

	for (const auto& line : aLines) {
		std::smatch match;
		size_t start = 0;
		if (std::regex_search(line, match, hexRegex)) {
			currentColor = ParseHexColor(match[1].str());
			start = match[0].length();
			if (start < line.size() && isspace(line[start])) ++start;
		}
		mLines.emplace_back(Line());
		for (size_t i = start; i < line.size(); ++i) {
			mLines.back().emplace_back(Glyph(line[i], currentColor));
		}
	}
	if (mLines.empty()) mLines.emplace_back(Line());

	mScrollToTop = true;
	mUndoBuffer.clear();
	mUndoIndex = 0;
}



std::vector<std::string> TextSelect::GetTextLines() const
{
	std::vector<std::string> result;

	result.reserve(mLines.size());

	for (auto& line : mLines)
	{
		std::string text;

		text.resize(line.size());

		for (size_t i = 0; i < line.size(); ++i)
			text[i] = line[i].mChar;

		result.emplace_back(std::move(text));
	}

	return result;
}

bool TextSelect::Render(const char* aTitle, bool aParentIsFocused, const ImVec2& aSize, bool aBorder)
{

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(0x000080ff));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	ImGui::BeginChild(aTitle, aSize, aBorder, ImGuiWindowFlags_HorizontalScrollbar | 
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavInputs);

	bool isFocused = ImGui::IsWindowFocused();
	HandleKeyboardInputs(aParentIsFocused);
	HandleMouseInputs();
	Render(aParentIsFocused);

	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	return isFocused;
}


TextSelect::UndoRecord::UndoRecord(const std::vector<UndoOperation>& aOperations,
	TextSelect::EditorState& aBefore, TextSelect::EditorState& aAfter)
{
	mOperations = aOperations;
	mBefore = aBefore;
	mAfter = aAfter;
	for (const UndoOperation& o : mOperations)
		assert(o.mStart <= o.mEnd);
}

void TextSelect::UndoRecord::Undo(TextSelect* aEditor)
{
	for (int i = mOperations.size() - 1; i > -1; i--)
	{
		const UndoOperation& operation = mOperations[i];
		if (!operation.mText.empty())
		{
			switch (operation.mType)
			{
			case UndoOperationType::Delete:
			{
				auto start = operation.mStart;
				aEditor->InsertTextAt(start, operation.mText.c_str());
				break;
			}
			case UndoOperationType::Add:
			{
				aEditor->DeleteRange(operation.mStart, operation.mEnd);
				break;
			}
			}
		}
	}

	aEditor->mState = mBefore;
	aEditor->EnsureCursorVisible();
}

void TextSelect::UndoRecord::Redo(TextSelect* aEditor)
{
	for (int i = 0; i < mOperations.size(); i++)
	{
		const UndoOperation& operation = mOperations[i];
		if (!operation.mText.empty())
		{
			switch (operation.mType)
			{
			case UndoOperationType::Delete:
			{
				aEditor->DeleteRange(operation.mStart, operation.mEnd);
				break;
			}
			case UndoOperationType::Add:
			{
				auto start = operation.mStart;
				aEditor->InsertTextAt(start, operation.mText.c_str());
				break;
			}
			}
		}
	}

	aEditor->mState = mAfter;
	aEditor->EnsureCursorVisible();
}

// ---------- Text editor internal functions --------- //

std::string TextSelect::GetText(const Coordinates& aStart, const Coordinates& aEnd) const
{
	assert(aStart < aEnd);

	std::string result;
	auto lstart = aStart.mLine;
	auto lend = aEnd.mLine;
	auto istart = GetCharacterIndexR(aStart);
	auto iend = GetCharacterIndexR(aEnd);
	size_t s = 0;

	for (size_t i = lstart; i < lend; i++)
		s += mLines[i].size();

	result.reserve(s + s / 8);

	while (istart < iend || lstart < lend)
	{
		if (lstart >= (int)mLines.size())
			break;

		auto& line = mLines[lstart];
		if (istart < (int)line.size())
		{
			result += line[istart].mChar;
			istart++;
		}
		else
		{
			istart = 0;
			++lstart;
			result += '\n';
		}
	}

	return result;
}

std::string TextSelect::GetClipboardText() const
{
	std::string result;

	if (mState.mCursor.GetSelectionStart() < mState.mCursor.GetSelectionEnd())
	{
		if (result.length() != 0)
			result += '\n';
		result += GetText(mState.mCursor.GetSelectionStart(), mState.mCursor.GetSelectionEnd());
	}
	
	return result;
}


void TextSelect::SetCursorPosition(const Coordinates& aPosition, bool aClearSelection)
{
	if (aClearSelection)
		mState.mCursor.mInteractiveStart = aPosition;
	if (mState.mCursor.mInteractiveEnd != aPosition)
	{
		mState.mCursor.mInteractiveEnd = aPosition;
		EnsureCursorVisible();
	}
}

int TextSelect::InsertTextAt(Coordinates& /* inout */ aWhere, const char* aValue)
{
	assert(!mReadOnly);

	int cindex = GetCharacterIndexR(aWhere);
	int totalLines = 0;
	while (*aValue != '\0')
	{
		assert(!mLines.empty());

		if (*aValue == '\r')
		{
			// skip
			++aValue;
		}
		else if (*aValue == '\n')
		{
			if (cindex < (int)mLines[aWhere.mLine].size())
			{
				auto& newLine = InsertLine(aWhere.mLine + 1);
				auto& line = mLines[aWhere.mLine];
				AddGlyphsToLine(aWhere.mLine + 1, 0, line.begin() + cindex, line.end());
				RemoveGlyphsFromLine(aWhere.mLine, cindex);
			}
			else
			{
				InsertLine(aWhere.mLine + 1);
			}
			++aWhere.mLine;
			aWhere.mColumn = 0;
			cindex = 0;
			++totalLines;
			++aValue;
		}
		else
		{
			auto& line = mLines[aWhere.mLine];
			auto d = UTF8CharLength(*aValue);
			while (d-- > 0 && *aValue != '\0')
				AddGlyphToLine(aWhere.mLine, cindex++, Glyph(*aValue++, 0xffffffff));
			aWhere.mColumn = GetCharacterColumn(aWhere.mLine, cindex);
		}
	}

	return totalLines;
}

void TextSelect::InsertTextAtCursor(const char* aValue)
{
	if (aValue == nullptr)
		return;

	auto pos = GetSanitizedCursorCoordinates();
	auto start = std::min(pos, mState.mCursor.GetSelectionStart());
	int totalLines = pos.mLine - start.mLine;

	totalLines += InsertTextAt(pos, aValue);

	SetCursorPosition(pos);
}

bool TextSelect::Move(int& aLine, int& aCharIndex, bool aLeft, bool aLockLine) const
{
	// assumes given char index is not in the middle of utf8 sequence
	// char index can be line.length()

	// invalid line
	if (aLine >= mLines.size())
		return false;

	if (aLeft)
	{
		if (aCharIndex == 0)
		{
			if (aLockLine || aLine == 0)
				return false;
			aLine--;
			aCharIndex = mLines[aLine].size();
		}
		else
		{
			aCharIndex--;
			while (aCharIndex > 0 && IsUTFSequence(mLines[aLine][aCharIndex].mChar))
				aCharIndex--;
		}
	}
	else // right
	{
		if (aCharIndex == mLines[aLine].size())
		{
			if (aLockLine || aLine == mLines.size() - 1)
				return false;
			aLine++;
			aCharIndex = 0;
		}
		else
		{
			int seqLength = UTF8CharLength(mLines[aLine][aCharIndex].mChar);
			aCharIndex = std::min(aCharIndex + seqLength, (int)mLines[aLine].size());
		}
	}
	return true;
}

void TextSelect::MoveCharIndexAndColumn(int aLine, int& aCharIndex, int& aColumn) const
{
	assert(aLine < mLines.size());
	assert(aCharIndex < mLines[aLine].size());
	char c = mLines[aLine][aCharIndex].mChar;
	aCharIndex += UTF8CharLength(c);
	if (c == '\t')
		aColumn = (aColumn / mTabSize) * mTabSize + mTabSize;
	else
		aColumn++;
}

void TextSelect::MoveCoords(Coordinates& aCoords, MoveDirection aDirection, 
	bool aWordMode, int aLineCount) const
{
	int charIndex = GetCharacterIndexR(aCoords);
	int lineIndex = aCoords.mLine;
	switch (aDirection)
	{
	case MoveDirection::Right:
		if (charIndex >= mLines[lineIndex].size())
		{
			if (lineIndex < mLines.size() - 1)
			{
				aCoords.mLine = std::max(0, std::min((int)mLines.size() - 1, lineIndex + 1));
				aCoords.mColumn = 0;
			}
		}
		else
		{
			Move(lineIndex, charIndex);
			int oneStepRightColumn = GetCharacterColumn(lineIndex, charIndex);
			if (aWordMode)
			{
				aCoords = FindWordEnd(aCoords);
				aCoords.mColumn = std::max(aCoords.mColumn, oneStepRightColumn);
			}
			else
				aCoords.mColumn = oneStepRightColumn;
		}
		break;
	case MoveDirection::Left:
		if (charIndex == 0)
		{
			if (lineIndex > 0)
			{
				aCoords.mLine = lineIndex - 1;
				aCoords.mColumn = GetLineMaxColumn(aCoords.mLine);
			}
		}
		else
		{
			Move(lineIndex, charIndex, true);
			aCoords.mColumn = GetCharacterColumn(lineIndex, charIndex);
			if (aWordMode)
				aCoords = FindWordStart(aCoords);
		}
		break;
	case MoveDirection::Up:
		aCoords.mLine = std::max(0, lineIndex - aLineCount);
		break;
	case MoveDirection::Down:
		aCoords.mLine = std::max(0, std::min((int)mLines.size() - 1, lineIndex + aLineCount));
		break;
	}
}


void TextSelect::MoveVertical(MoveDirection dir,int aAmount, bool aSelect)
{

	assert(mState.mCursor.mInteractiveEnd.mColumn >= 0);
	Coordinates newCoords = mState.mCursor.mInteractiveEnd;
	MoveCoords(newCoords, dir, false, aAmount);
	SetCursorPosition(newCoords, !aSelect);

	EnsureCursorVisible();
}

void TextSelect::MoveHorizontal(MoveDirection dir,bool aSelect, bool aWordMode)
{
	if (mLines.empty())
		return;

	if (mState.mCursor.HasSelection() && !aSelect && !aWordMode)
	{ 
		SetCursorPosition(dir == MoveDirection::Right ? mState.mCursor.GetSelectionEnd() :
			mState.mCursor.GetSelectionStart());
	}
	else
	{
		Coordinates newCoords = mState.mCursor.mInteractiveEnd;
		MoveCoords(newCoords, dir, aWordMode);
		SetCursorPosition(newCoords, !aSelect);
	}
	EnsureCursorVisible();
}



void TextSelect::TextSelect::MoveBottom(bool aSelect)
{
	int maxLine = (int)mLines.size() - 1;
	Coordinates newPos = Coordinates(maxLine, GetLineMaxColumn(maxLine));
	SetCursorPosition(newPos, !aSelect);
}


void TextSelect::MoveEnd(bool aSelect)
{
	int lindex = mState.mCursor.mInteractiveEnd.mLine;
	SetCursorPosition(Coordinates(lindex, GetLineMaxColumn(lindex)), !aSelect);
}

void TextSelect::EnterCharacter(ImWchar aChar, bool aShift)
{
	assert(!mReadOnly);

	
	bool isIndentOperation = mState.mCursor.HasSelection() && aChar == '\t';
	if (isIndentOperation)
	{
		ChangeCurrentLinesIndentation(!aShift);
		return;
	}

	UndoRecord u;
	u.mBefore = mState;

	if (mState.mCursor.HasSelection())
	{
		u.mOperations.push_back({ GetText(mState.mCursor.GetSelectionStart(), 
			mState.mCursor.GetSelectionEnd()), 
			mState.mCursor.GetSelectionStart(),
			mState.mCursor.GetSelectionEnd(), UndoOperationType::Delete });
		DeleteSelection();
	}

	auto coord = GetSanitizedCursorCoordinates();
	UndoOperation added;
	added.mType = UndoOperationType::Add;
	added.mStart = coord;

	assert(!mLines.empty());

	if (aChar == '\n')
	{
		InsertLine(coord.mLine + 1);
		auto& line = mLines[coord.mLine];
		auto& newLine = mLines[coord.mLine + 1];

		added.mText = "";
		added.mText += (char)aChar;
		if (mAutoIndent)
			for (int i = 0; i < line.size() && isascii(line[i].mChar) && isblank(line[i].mChar); ++i)
			{
				newLine.push_back(line[i]);
				added.mText += line[i].mChar;
			}

		const size_t whitespaceSize = newLine.size();
		auto cindex = GetCharacterIndexR(coord);
		AddGlyphsToLine(coord.mLine + 1, newLine.size(), line.begin() + cindex, line.end());
		RemoveGlyphsFromLine(coord.mLine, cindex);
		SetCursorPosition(Coordinates(coord.mLine + 1, GetCharacterColumn(coord.mLine + 1, 
			(int)whitespaceSize)));
	}
	else
	{
		char buf[7];
		int e = ImTextCharToUtf8(buf, 7, aChar);
		if (e > 0)
		{
			buf[e] = '\0';
			auto& line = mLines[coord.mLine];
			auto cindex = GetCharacterIndexR(coord);

			for (auto p = buf; *p != '\0'; p++, ++cindex)
				AddGlyphToLine(coord.mLine, cindex, Glyph(*p, 0xffffffff));
			added.mText = buf;

			SetCursorPosition(Coordinates(coord.mLine, GetCharacterColumn(coord.mLine, cindex)));
		}
	}

	added.mEnd = GetSanitizedCursorCoordinates();
	u.mOperations.push_back(added);

	u.mAfter = mState;
	AddUndo(u);

	EnsureCursorVisible();
}

void TextSelect::Backspace(bool aWordMode)
{
	assert(!mReadOnly);

	if (mLines.empty())
		return;

	if (mState.mCursor.HasSelection())
	{
		Delete(aWordMode);
		return;
	}

	const Coordinates& endRef = mState.mCursor.mInteractiveEnd;

	if (endRef.mLine == 0 && endRef.mColumn == 0)
		return; // can't backspace at beginning of document

	EditorState stateBeforeDeleting = mState;

	// Determine what to delete (simulate a backward selection)
	Coordinates end = endRef;
	MoveCoords(end, MoveDirection::Left, aWordMode);

	mState.mCursor.mInteractiveStart = end;
	Delete(aWordMode, &stateBeforeDeleting);
}

void TextSelect::Delete(bool aWordMode, const EditorState* aEditorState)
{
	assert(!mReadOnly);

	if (mLines.empty())
		return;

	if (mState.mCursor.HasSelection())
	{
		UndoRecord u;
		u.mBefore = aEditorState ? *aEditorState : mState;

		u.mOperations.push_back({
			GetText(mState.mCursor.GetSelectionStart(), mState.mCursor.GetSelectionEnd()),
			mState.mCursor.GetSelectionStart(),
			mState.mCursor.GetSelectionEnd(),
			UndoOperationType::Delete
			});
		DeleteSelection();

		u.mAfter = mState;
		AddUndo(u);
	}
	else
	{
		const Coordinates& endRef = mState.mCursor.mInteractiveEnd;

		if (endRef.mLine >= mLines.size())
			return;

		if (endRef.mLine == mLines.size() - 1 && endRef.mColumn >= GetLineMaxColumn(endRef.mLine))
			return; // can't delete at EOF

		EditorState stateBeforeDeleting = mState;

		// Simulate forward selection
		Coordinates end = endRef;
		MoveCoords(end, MoveDirection::Right, aWordMode);

		mState.mCursor.mInteractiveStart = endRef;
		mState.mCursor.mInteractiveEnd = end;

		Delete(aWordMode, &stateBeforeDeleting);
	}
}

void TextSelect::SetSelection(Coordinates aStart, Coordinates aEnd)
{

	Coordinates minCoords = Coordinates(0, 0);
	int maxLine = (int)mLines.size() - 1;
	Coordinates maxCoords = Coordinates(maxLine, GetLineMaxColumn(maxLine));
	if (aStart < minCoords)
		aStart = minCoords;
	else if (aStart > maxCoords)
		aStart = maxCoords;
	if (aEnd < minCoords)
		aEnd = minCoords;
	else if (aEnd > maxCoords)
		aEnd = maxCoords;

	mState.mCursor.mInteractiveStart = aStart;
	SetCursorPosition(aEnd, false);
}

void TextSelect::SetSelection(int aStartLine, int aStartChar, int aEndLine, int aEndChar)
{
	Coordinates startCoords = { aStartLine, GetCharacterColumn(aStartLine, aStartChar) };
	Coordinates endCoords = { aEndLine, GetCharacterColumn(aEndLine, aEndChar) };
	SetSelection(startCoords, endCoords);
}


void TextSelect::ChangeCurrentLinesIndentation(bool aIncrease)
{
	assert(!mReadOnly);

	UndoRecord u;
	u.mBefore = mState;

	for (int currentLine = mState.mCursor.GetSelectionEnd().mLine; 
		currentLine >= mState.mCursor.GetSelectionStart().mLine; currentLine--)
	{
		if (Coordinates{ currentLine, 0 } == mState.mCursor.GetSelectionEnd() && 
			mState.mCursor.GetSelectionEnd() != mState.mCursor.GetSelectionStart()) 
			// when selection ends at line start
			continue;

		if (aIncrease)
		{
			if (mLines[currentLine].size() > 0)
			{
				Coordinates lineStart = { currentLine, 0 };
				Coordinates insertionEnd = lineStart;
				InsertTextAt(insertionEnd, "\t"); // sets insertion end
				u.mOperations.push_back({ "\t", lineStart, insertionEnd, UndoOperationType::Add });
			}
		}
		else
		{
			Coordinates start = { currentLine, 0 };
			Coordinates end = { currentLine, mTabSize };
			int charIndex = GetCharacterIndexL(end) - 1;
			
			while (charIndex > -1 && (mLines[currentLine][charIndex].mChar == ' ' || 
				mLines[currentLine][charIndex].mChar == '\t')) charIndex--;
			
			bool onlySpaceCharactersFound = charIndex == -1;
			if (onlySpaceCharactersFound)
			{
				u.mOperations.push_back({ GetText(start, end), start, end, UndoOperationType::Delete });
				DeleteRange(start, end);

			}
		}
	}

	if (u.mOperations.size() > 0)
		AddUndo(u);
}

void TextSelect::MoveUpCurrentLines()
{
	assert(!mReadOnly);

	UndoRecord u;
	u.mBefore = mState;

	std::set<int> affectedLines;
	int minLine = -1;
	int maxLine = -1;

	for (int currentLine = mState.mCursor.GetSelectionEnd().mLine; 
		currentLine >= mState.mCursor.GetSelectionStart().mLine; currentLine--)
	{
		if (Coordinates{ currentLine, 0 } == mState.mCursor.GetSelectionEnd() 
			&& mState.mCursor.GetSelectionEnd() != mState.mCursor.GetSelectionStart()) 
			// when selection ends at line start
			continue;
		affectedLines.insert(currentLine);
		minLine = minLine == -1 ? currentLine : (currentLine < minLine ? currentLine : minLine);
		maxLine = maxLine == -1 ? currentLine : (currentLine > maxLine ? currentLine : maxLine);
	}
	
	if (minLine == 0) // can't move up anymore
		return;

	Coordinates start = { minLine - 1, 0 };
	Coordinates end = { maxLine, GetLineMaxColumn(maxLine) };
	u.mOperations.push_back({ GetText(start, end), start, end, UndoOperationType::Delete });

	for (int line : affectedLines) // lines should be sorted here
		std::swap(mLines[line - 1], mLines[line]);
	
	mState.mCursor.mInteractiveStart.mLine -= 1;
	mState.mCursor.mInteractiveEnd.mLine -= 1;
	// no need to set mCursorPositionChanged as cursors will remain sorted
	

	end = { maxLine, GetLineMaxColumn(maxLine) }; 
	// this line is swapped with line above, need to find new max column
	u.mOperations.push_back({ GetText(start, end), start, end, UndoOperationType::Add });
	u.mAfter = mState;
	AddUndo(u);
}

void TextSelect::MoveDownCurrentLines()
{
	assert(!mReadOnly);

	UndoRecord u;
	u.mBefore = mState;

	std::set<int> affectedLines;
	int minLine = -1;
	int maxLine = -1;
	for (int currentLine = mState.mCursor.GetSelectionEnd().mLine; 
		currentLine >= mState.mCursor.GetSelectionStart().mLine; currentLine--)
	{
		if (Coordinates{ currentLine, 0 } == mState.mCursor.GetSelectionEnd() && 
			mState.mCursor.GetSelectionEnd() != mState.mCursor.GetSelectionStart()) 
			// when selection ends at line start
			continue;
		affectedLines.insert(currentLine);
		minLine = minLine == -1 ? currentLine : (currentLine < minLine ? currentLine : minLine);
		maxLine = maxLine == -1 ? currentLine : (currentLine > maxLine ? currentLine : maxLine);
	}
	if (maxLine == mLines.size() - 1) // can't move down anymore
		return;

	Coordinates start = { minLine, 0 };
	Coordinates end = { maxLine + 1, GetLineMaxColumn(maxLine + 1) };
	u.mOperations.push_back({ GetText(start, end), start, end, UndoOperationType::Delete });

	std::set<int>::reverse_iterator rit;
	for (rit = affectedLines.rbegin(); rit != affectedLines.rend(); rit++) // lines should be sorted here
		std::swap(mLines[*rit + 1], mLines[*rit]);
	
	mState.mCursor.mInteractiveStart.mLine += 1;
	mState.mCursor.mInteractiveEnd.mLine += 1;
	// no need to set mCursorPositionChanged as cursors will remain sorted
	

	end = { maxLine + 1, GetLineMaxColumn(maxLine + 1) };
	// this line is swapped with line below, need to find new max column
	u.mOperations.push_back({ GetText(start, end), start, end, UndoOperationType::Add });
	u.mAfter = mState;
	AddUndo(u);
}

void TextSelect::RemoveCurrentLines()
{
	UndoRecord u;
	u.mBefore = mState;

	if (mState.mCursor.HasSelection())
	{

		u.mOperations.push_back({ GetText(mState.mCursor.GetSelectionStart(),
			mState.mCursor.GetSelectionEnd()),
			mState.mCursor.GetSelectionStart(), 
			mState.mCursor.GetSelectionEnd(), UndoOperationType::Delete });
		DeleteSelection();
		
	}
	SetCursorPosition(Coordinates(mState.mCursor.mInteractiveEnd.mLine, 0), true);

	int currentLine = mState.mCursor.mInteractiveEnd.mLine;
	int nextLine = currentLine + 1;
	int prevLine = currentLine - 1;

	Coordinates toDeleteStart, toDeleteEnd;
	if (mLines.size() > nextLine) // next line exists
	{
		toDeleteStart = Coordinates(currentLine, 0);
		toDeleteEnd = Coordinates(nextLine, 0);
		SetCursorPosition({ mState.mCursor.mInteractiveEnd.mLine, 0 });
	}
	else if (prevLine > -1) // previous line exists
	{
		toDeleteStart = Coordinates(prevLine, GetLineMaxColumn(prevLine));
		toDeleteEnd = Coordinates(currentLine, GetLineMaxColumn(currentLine));
		SetCursorPosition({ prevLine, 0 });
	}
	else
	{
		toDeleteStart = Coordinates(currentLine, 0);
		toDeleteEnd = Coordinates(currentLine, GetLineMaxColumn(currentLine));
		SetCursorPosition({ currentLine, 0 });
	}

	u.mOperations.push_back({ GetText(toDeleteStart, toDeleteEnd), 
		toDeleteStart, toDeleteEnd, UndoOperationType::Delete });

	if (toDeleteStart.mLine != toDeleteEnd.mLine)
		RemoveLine(currentLine);
	else
		DeleteRange(toDeleteStart, toDeleteEnd);
	
	u.mAfter = mState;
	AddUndo(u);
}

float TextSelect::TextDistanceToLineStart(const Coordinates& aFrom, bool aSanitizeCoords) const
{
	if (aSanitizeCoords)
		return SanitizeCoordinates(aFrom).mColumn * mCharAdvance.x;
	else
		return aFrom.mColumn * mCharAdvance.x;
}

void TextSelect::EnsureCursorVisible(bool aStartToo)
{
	mEnsureCursorVisible = 0;
	mEnsureCursorVisibleStartToo = aStartToo;
	return;
}

TextSelect::Coordinates TextSelect::SanitizeCoordinates(const Coordinates& aValue) const
{
	// Clamp in document and line limits
	auto line = std::max(aValue.mLine, 0);
	auto column = std::max(aValue.mColumn, 0);
	Coordinates out;
	if (line >= (int)mLines.size())
	{
		if (mLines.empty())
		{
			line = 0;
			column = 0;
		}
		else
		{
			line = (int)mLines.size() - 1;
			column = GetLineMaxColumn(line);
		}
		out = Coordinates(line, column);
	}
	else
	{
		column = mLines.empty() ? 0 : GetLineMaxColumn(line, column);
		out = Coordinates(line, column);
	}

	// Move if inside a tab character
	int charIndex = GetCharacterIndexL(out);
	if (charIndex > -1 && charIndex < mLines[out.mLine].size() && 
		mLines[out.mLine][charIndex].mChar == '\t')
	{
		int columnToLeft = GetCharacterColumn(out.mLine, charIndex);
		int columnToRight = GetCharacterColumn(out.mLine, GetCharacterIndexR(out));
		if (out.mColumn - columnToLeft <= columnToRight - out.mColumn)
			out.mColumn = columnToLeft;
		else
			out.mColumn = columnToRight;
	}
	return out;
}

TextSelect::Coordinates TextSelect::ScreenPosToCoordinates(const ImVec2& aPosition, bool* 
	isOverLineNumber) const
{
	ImVec2 origin = ImGui::GetCursorScreenPos();
	ImVec2 local(aPosition.x - origin.x + 3.0f, aPosition.y - origin.y);

	if (isOverLineNumber != nullptr)
		*isOverLineNumber = local.x < mTextStart;

	Coordinates out = {
		std::max(0, (int)floor(local.y / mCharAdvance.y)),
		std::max(0, (int)floor((local.x - mTextStart) / mCharAdvance.x))
	};
	out.mColumn = std::max(0, (int)floor((local.x - mTextStart + POS_TO_COORDS_COLUMN_OFFSET *
		mCharAdvance.x) / mCharAdvance.x));

	return SanitizeCoordinates(out);
}

TextSelect::Coordinates TextSelect::FindWordStart(const Coordinates& aFrom) const
{
	if (aFrom.mLine >= (int)mLines.size())
		return aFrom;

	int lineIndex = aFrom.mLine;
	auto& line = mLines[lineIndex];
	int charIndex = GetCharacterIndexL(aFrom);

	if (charIndex > (int)line.size() || line.size() == 0)
		return aFrom;
	if (charIndex == (int)line.size())
		charIndex--;

	bool initialIsWordChar = CharIsWordChar(line[charIndex].mChar);
	bool initialIsSpace = isspace(line[charIndex].mChar);
	char initialChar = line[charIndex].mChar;
	while (Move(lineIndex, charIndex, true, true))
	{
		bool isWordChar = CharIsWordChar(line[charIndex].mChar);
		bool isSpace = isspace(line[charIndex].mChar);
		if (initialIsSpace && !isSpace ||
			initialIsWordChar && !isWordChar ||
			!initialIsWordChar && !initialIsSpace && initialChar != line[charIndex].mChar)
		{
			Move(lineIndex, charIndex, false, true); // one step to the right
			break;
		}
	}
	return { aFrom.mLine, GetCharacterColumn(aFrom.mLine, charIndex) };
}

TextSelect::Coordinates TextSelect::FindWordEnd(const Coordinates& aFrom) const
{
	if (aFrom.mLine >= (int)mLines.size())
		return aFrom;

	int lineIndex = aFrom.mLine;
	auto& line = mLines[lineIndex];
	auto charIndex = GetCharacterIndexL(aFrom);

	if (charIndex >= (int)line.size())
		return aFrom;

	bool initialIsWordChar = CharIsWordChar(line[charIndex].mChar);
	bool initialIsSpace = isspace(line[charIndex].mChar);
	char initialChar = line[charIndex].mChar;
	while (Move(lineIndex, charIndex, false, true))
	{
		if (charIndex == line.size())
			break;
		bool isWordChar = CharIsWordChar(line[charIndex].mChar);
		bool isSpace = isspace(line[charIndex].mChar);
		if (initialIsSpace && !isSpace ||
			initialIsWordChar && !isWordChar ||
			!initialIsWordChar && !initialIsSpace && initialChar != line[charIndex].mChar)
			break;
	}
	return { lineIndex, GetCharacterColumn(aFrom.mLine, charIndex) };
}



int TextSelect::GetCharacterIndexL(const Coordinates& aCoords) const
{
	if (aCoords.mLine >= mLines.size())
		return -1;

	auto& line = mLines[aCoords.mLine];
	int c = 0;
	int i = 0;
	int tabCoordsLeft = 0;

	for (; i < line.size() && c < aCoords.mColumn;)
	{
		if (line[i].mChar == '\t')
		{
			if (tabCoordsLeft == 0)
				tabCoordsLeft = TabSizeAtColumn(c);
			if (tabCoordsLeft > 0)
				tabCoordsLeft--;
			c++;
		}
		else
			++c;
		if (tabCoordsLeft == 0)
			i += UTF8CharLength(line[i].mChar);
	}
	return i;
}

int TextSelect::GetCharacterIndexR(const Coordinates& aCoords) const
{
	if (aCoords.mLine >= mLines.size())
		return -1;
	int c = 0;
	int i = 0;
	for (; i < mLines[aCoords.mLine].size() && c < aCoords.mColumn;)
		MoveCharIndexAndColumn(aCoords.mLine, i, c);
	return i;
}

int TextSelect::GetCharacterColumn(int aLine, int aIndex) const
{
	if (aLine >= mLines.size())
		return 0;
	int c = 0;
	int i = 0;
	while (i < aIndex && i < mLines[aLine].size())
		MoveCharIndexAndColumn(aLine, i, c);
	return c;
}

int TextSelect::GetFirstVisibleCharacterIndex(int aLine) const
{
	if (aLine >= mLines.size())
		return 0;
	int c = 0;
	int i = 0;
	while (c < mFirstVisibleColumn && i < mLines[aLine].size())
		MoveCharIndexAndColumn(aLine, i, c);
	if (c > mFirstVisibleColumn)
		i--;
	return i;
}

int TextSelect::GetLineMaxColumn(int aLine, int aLimit) const
{
	if (aLine >= mLines.size())
		return 0;
	int c = 0;
	if (aLimit == -1)
	{
		for (int i = 0; i < mLines[aLine].size(); )
			MoveCharIndexAndColumn(aLine, i, c);
	}
	else
	{
		for (int i = 0; i < mLines[aLine].size(); )
		{
			MoveCharIndexAndColumn(aLine, i, c);
			if (c > aLimit)
				return aLimit;
		}
	}
	return c;
}

TextSelect::Line& TextSelect::InsertLine(int aIndex)
{
	assert(!mReadOnly);
	auto& result = *mLines.insert(mLines.begin() + aIndex, Line());

	if (mState.mCursor.mInteractiveEnd.mLine >= aIndex)
		SetCursorPosition({ mState.mCursor.mInteractiveEnd.mLine + 1, 
			mState.mCursor.mInteractiveEnd.mColumn });
	

	return result;
}

void TextSelect::RemoveLine(int aIndex)
{
	assert(!mReadOnly);
	assert(mLines.size() > 1);

	mLines.erase(mLines.begin() + aIndex);
	assert(!mLines.empty());

	// handle multiple cursors

	if (mState.mCursor.mInteractiveEnd.mLine >= aIndex)
	{
		SetCursorPosition({ mState.mCursor.mInteractiveEnd.mLine - 1, 
			mState.mCursor.mInteractiveEnd.mColumn });
	}

}

void TextSelect::RemoveLines(int aStart, int aEnd)
{
	assert(!mReadOnly);
	assert(aEnd >= aStart);
	assert(mLines.size() > (size_t)(aEnd - aStart));

	mLines.erase(mLines.begin() + aStart, mLines.begin() + aEnd);
	assert(!mLines.empty());


	if (mState.mCursor.mInteractiveEnd.mLine >= aStart)
	{
		int targetLine = mState.mCursor.mInteractiveEnd.mLine - (aEnd - aStart);
		targetLine = targetLine < 0 ? 0 : targetLine;
		mState.mCursor.mInteractiveEnd.mLine = targetLine;
	}
	if (mState.mCursor.mInteractiveStart.mLine >= aStart)
	{
		int targetLine = mState.mCursor.mInteractiveStart.mLine - (aEnd - aStart);
		targetLine = targetLine < 0 ? 0 : targetLine;
		mState.mCursor.mInteractiveStart.mLine = targetLine;
	}
	
}

void TextSelect::DeleteRange(const Coordinates& aStart, const Coordinates& aEnd)
{
	assert(aEnd >= aStart);
	assert(!mReadOnly);

	if (aEnd == aStart)
		return;

	auto start = GetCharacterIndexL(aStart);
	auto end = GetCharacterIndexR(aEnd);

	if (aStart.mLine == aEnd.mLine)
	{
		auto maxCol = GetLineMaxColumn(aStart.mLine);
		if (aEnd.mColumn >= maxCol)
			RemoveGlyphsFromLine(aStart.mLine, start); // to end of line
		else
			RemoveGlyphsFromLine(aStart.mLine, start, end);
	}
	else
	{
		// Delete glyphs from both lines
		RemoveGlyphsFromLine(aStart.mLine, start); // from start to end of line
		RemoveGlyphsFromLine(aEnd.mLine, 0, end);

		// Merge aEnd's remaining content into aStart
		auto& firstLine = mLines[aStart.mLine];
		auto& lastLine = mLines[aEnd.mLine];
		AddGlyphsToLine(aStart.mLine, firstLine.size(), lastLine.begin(), lastLine.end());

		// Adjust cursor if needed
		const auto& selStart = mState.mCursor.GetSelectionStart();
		const auto& selEnd = mState.mCursor.GetSelectionEnd();

		bool skipCursorAdjust = (selStart == aStart && selEnd == aEnd) ||
			mState.mCursor.mInteractiveEnd.mLine > aEnd.mLine ||
			mState.mCursor.mInteractiveEnd.mLine != aEnd.mLine;

		if (!skipCursorAdjust)
		{
			int newBaseCharIndex = GetCharacterIndexR(aStart);
			int curStartIdx = GetCharacterIndexR(mState.mCursor.mInteractiveStart);
			int curEndIdx = GetCharacterIndexR(mState.mCursor.mInteractiveEnd);

			auto newStart = Coordinates(aStart.mLine, GetCharacterColumn(aStart.mLine, newBaseCharIndex + curStartIdx));
			auto newEnd = Coordinates(aStart.mLine, GetCharacterColumn(aStart.mLine, newBaseCharIndex + curEndIdx));
			SetCursorPosition(newStart, true);
			SetCursorPosition(newEnd, false);
		}

		// Remove the now-merged lines
		RemoveLines(aStart.mLine + 1, aEnd.mLine + 1);
	}
}

void TextSelect::DeleteSelection()
{
	if (mState.mCursor.GetSelectionEnd() == mState.mCursor.GetSelectionStart())
		return;

	Coordinates newCursorPos = mState.mCursor.GetSelectionStart();
	DeleteRange(newCursorPos, mState.mCursor.GetSelectionEnd());
	SetCursorPosition(newCursorPos);

}

void TextSelect::RemoveGlyphsFromLine(int aLine, int aStartChar, int aEndChar)
{
	int column = GetCharacterColumn(aLine, aStartChar);
	auto& line = mLines[aLine];
	OnLineChanged(true, aLine, column, aEndChar - aStartChar, true);
	line.erase(line.begin() + aStartChar, aEndChar == -1 ? line.end() : line.begin() + aEndChar);
	OnLineChanged(false, aLine, column, aEndChar - aStartChar, true);
}

void TextSelect::AddGlyphsToLine(int aLine, int aTargetIndex, Line::iterator aSourceStart, 
	Line::iterator aSourceEnd)
{
	int targetColumn = GetCharacterColumn(aLine, aTargetIndex);
	int charsInserted = std::distance(aSourceStart, aSourceEnd);
	auto& line = mLines[aLine];
	OnLineChanged(true, aLine, targetColumn, charsInserted, false);
	line.insert(line.begin() + aTargetIndex, aSourceStart, aSourceEnd);
	OnLineChanged(false, aLine, targetColumn, charsInserted, false);
}

void TextSelect::AddGlyphToLine(int aLine, int aTargetIndex, Glyph aGlyph)
{
	int targetColumn = GetCharacterColumn(aLine, aTargetIndex);
	auto& line = mLines[aLine];
	OnLineChanged(true, aLine, targetColumn, 1, false);
	line.insert(line.begin() + aTargetIndex, aGlyph);
	OnLineChanged(false, aLine, targetColumn, 1, false);
}


void TextSelect::HandleKeyboardInputs(bool aParentIsFocused)
{
	if (ImGui::IsWindowFocused() || aParentIsFocused)
	{
		if (ImGui::IsWindowHovered())
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
		//ImGui::CaptureKeyboardFromApp(true);

		ImGuiIO& io = ImGui::GetIO();
		auto isOSX = io.ConfigMacOSXBehaviors;
		auto alt = io.KeyAlt;
		auto ctrl = io.KeyCtrl;
		auto shift = io.KeyShift;
		auto super = io.KeySuper;

		auto isShortcut = (isOSX ? (super && !ctrl) : (ctrl && !super)) && !alt && !shift;
		auto isShiftShortcut = (isOSX ? (super && !ctrl) : (ctrl && !super)) && shift && !alt;
		auto isWordmoveKey = isOSX ? alt : ctrl;
		auto isAltOnly = alt && !ctrl && !shift && !super;
		auto isCtrlOnly = ctrl && !alt && !shift && !super;
		auto isShiftOnly = shift && !alt && !ctrl && !super;

		io.WantCaptureKeyboard = true;
		io.WantTextInput = true;

		if (!mReadOnly && isShortcut && ImGui::IsKeyPressed(ImGuiKey_Z))
			Undo();
		else if (!mReadOnly && isAltOnly && ImGui::IsKeyPressed(ImGuiKey_Backspace))
			Undo();
		else if (!mReadOnly && isShortcut && ImGui::IsKeyPressed(ImGuiKey_Y))
			Redo();
		else if (!mReadOnly && isShiftShortcut && ImGui::IsKeyPressed(ImGuiKey_Z))
			Redo();
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			MoveVertical(MoveDirection::Up,1, shift);
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			MoveVertical(MoveDirection::Down,1, shift);
		else if ((isOSX ? !ctrl : !alt) && !super && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			MoveHorizontal(MoveDirection::Left,shift, isWordmoveKey);
		else if ((isOSX ? !ctrl : !alt) && !super && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			MoveHorizontal(MoveDirection::Right,shift, isWordmoveKey);
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_PageUp))
			MoveVertical(MoveDirection::Up, mVisibleLineCount - 2, shift);
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_PageDown))
			MoveVertical(MoveDirection::Down,mVisibleLineCount - 2, shift);
		else if (ctrl && !alt && !super && ImGui::IsKeyPressed(ImGuiKey_Home))
			SetCursorPosition(Coordinates(0, 0), !shift);
		else if (ctrl && !alt && !super && ImGui::IsKeyPressed(ImGuiKey_End))
			MoveBottom(shift);
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_Home))
			SetCursorPosition(Coordinates(mState.mCursor.mInteractiveEnd.mLine, 0), !shift);
		else if (!alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_End))
			MoveEnd(shift);
		else if (!mReadOnly && !alt && !shift && !super && ImGui::IsKeyPressed(ImGuiKey_Delete))
			Delete(ctrl);
		else if (!mReadOnly && !alt && !shift && !super && ImGui::IsKeyPressed(ImGuiKey_Backspace))
			Backspace(ctrl);
		else if (!mReadOnly && !alt && ctrl && shift && !super && ImGui::IsKeyPressed(ImGuiKey_K))
			RemoveCurrentLines();
		else if (!mReadOnly && !alt && ctrl && !shift && !super && 
			ImGui::IsKeyPressed(ImGuiKey_LeftBracket))
			ChangeCurrentLinesIndentation(false);
		else if (!mReadOnly && !alt && ctrl && !shift && !super && 
			ImGui::IsKeyPressed(ImGuiKey_RightBracket))
			ChangeCurrentLinesIndentation(true);
		else if (!alt && ctrl && shift && !super && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			MoveUpCurrentLines();
		else if (!alt && ctrl && shift && !super && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			MoveDownCurrentLines();
		else if (isCtrlOnly && ImGui::IsKeyPressed(ImGuiKey_Insert))
			Copy();
		else if (isShortcut && ImGui::IsKeyPressed(ImGuiKey_C))
			Copy();
		else if (!mReadOnly && isShiftOnly && ImGui::IsKeyPressed(ImGuiKey_Insert))
			Paste();
		else if (!mReadOnly && isShortcut && ImGui::IsKeyPressed(ImGuiKey_V))
			Paste();
		else if (isShortcut && ImGui::IsKeyPressed(ImGuiKey_X))
			Cut();
		else if (isShiftOnly && ImGui::IsKeyPressed(ImGuiKey_Delete))
			Cut();
		else if (isShortcut && ImGui::IsKeyPressed(ImGuiKey_A))
			SelectAll();
		else if (!mReadOnly && !alt && !ctrl && !shift && !super && 
			(ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)))
			EnterCharacter('\n', false);
		else if (!mReadOnly && !alt && !ctrl && !super && ImGui::IsKeyPressed(ImGuiKey_Tab))
			EnterCharacter('\t', shift);
		if (!mReadOnly && !io.InputQueueCharacters.empty() && ctrl == alt && !super)
		{
			for (int i = 0; i < io.InputQueueCharacters.Size; i++)
			{
				auto c = io.InputQueueCharacters[i];
				if (c != 0 && (c == '\n' || c >= 32))
					EnterCharacter(c, shift);
			}
			io.InputQueueCharacters.resize(0);
		}
	}
}

void TextSelect::HandleMouseInputs()
{
	ImGuiIO& io = ImGui::GetIO();
	auto shift = io.KeyShift;
	auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
	auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

	/*
	Pan with middle mouse button
	*/
	mPanning &= ImGui::IsMouseDown(2);
	if (mPanning && ImGui::IsMouseDragging(2))
	{
		ImVec2 scroll = { ImGui::GetScrollX(), ImGui::GetScrollY() };
		ImVec2 currentMousePos = ImGui::GetMouseDragDelta(2);
		ImVec2 mouseDelta = {
			currentMousePos.x - mLastMousePos.x,
			currentMousePos.y - mLastMousePos.y
		};
		ImGui::SetScrollY(scroll.y - mouseDelta.y);
		ImGui::SetScrollX(scroll.x - mouseDelta.x);
		mLastMousePos = currentMousePos;
	}

	// Mouse left button dragging (=> update selection)
	mDraggingSelection &= ImGui::IsMouseDown(0);
	if (mDraggingSelection && ImGui::IsMouseDragging(0))
	{
		io.WantCaptureMouse = true;
		Coordinates cursorCoords = ScreenPosToCoordinates(ImGui::GetMousePos());
		SetCursorPosition(cursorCoords, false);
	}

	if (ImGui::IsWindowHovered())
	{
		auto click = ImGui::IsMouseClicked(0);
		if (!shift && !alt)
		{
			auto doubleClick = ImGui::IsMouseDoubleClicked(0);
			auto t = ImGui::GetTime();
			auto tripleClick = click && !doubleClick &&
				(mLastClickTime != -1.0f && (t - mLastClickTime) < io.MouseDoubleClickTime &&
					Distance(io.MousePos, mLastClickPos) < 0.01f);

			if (click)
				mDraggingSelection = true;

			/*
			Pan with middle mouse button
			*/

			if (ImGui::IsMouseClicked(2))
			{
				mPanning = true;
				mLastMousePos = ImGui::GetMouseDragDelta(2);
			}

			/*
			Left mouse button triple click
			*/

			if (tripleClick)
			{

				Coordinates cursorCoords = ScreenPosToCoordinates(ImGui::GetMousePos());
				Coordinates targetCursorPos = cursorCoords.mLine < mLines.size() - 1 ?
					Coordinates{ cursorCoords.mLine + 1, 0 } :
					Coordinates{ cursorCoords.mLine, GetLineMaxColumn(cursorCoords.mLine) };
				SetSelection({ cursorCoords.mLine, 0 }, targetCursorPos);

				mLastClickTime = -1.0f;
			}

			/*
			Left mouse button double click
			*/

			else if (doubleClick)
			{

				Coordinates cursorCoords = ScreenPosToCoordinates(ImGui::GetMousePos());
				SetSelection(FindWordStart(cursorCoords), FindWordEnd(cursorCoords));

				mLastClickTime = (float)ImGui::GetTime();
				mLastClickPos = io.MousePos;
			}

			/*
			Left mouse button click
			*/
			else if (click)
			{
			
				bool isOverLineNumber;
				Coordinates cursorCoords = ScreenPosToCoordinates(ImGui::GetMousePos(), &isOverLineNumber);
				if (isOverLineNumber)
				{
					Coordinates targetCursorPos = cursorCoords.mLine < mLines.size() - 1 ?
						Coordinates{ cursorCoords.mLine + 1, 0 } :
						Coordinates{ cursorCoords.mLine, GetLineMaxColumn(cursorCoords.mLine) };
					SetSelection({ cursorCoords.mLine, 0 }, targetCursorPos);
				}
				else
					SetCursorPosition(cursorCoords);

				mLastClickTime = (float)ImGui::GetTime();
				mLastClickPos = io.MousePos;
			}

		}
		else if (shift)
		{
			if (click)
			{
				Coordinates newSelection = ScreenPosToCoordinates(ImGui::GetMousePos());
				SetCursorPosition(newSelection, false);
			}
		}
	}
}

void TextSelect::UpdateViewVariables(float aScrollX, float aScrollY)
{
	mContentHeight = ImGui::GetWindowHeight() - (IsHorizontalScrollbarVisible() ? 
		IMGUI_SCROLLBAR_WIDTH : 0.0f);
	mContentWidth = ImGui::GetWindowWidth() - (IsVerticalScrollbarVisible() ? 
		IMGUI_SCROLLBAR_WIDTH : 0.0f);

	mVisibleLineCount = std::max((int)ceil(mContentHeight / mCharAdvance.y), 0);
	mFirstVisibleLine = std::max((int)(aScrollY / mCharAdvance.y), 0);
	mLastVisibleLine = std::max((int)((mContentHeight + aScrollY) / mCharAdvance.y), 0);

	mVisibleColumnCount = std::max((int)ceil((mContentWidth - std::max(mTextStart - aScrollX, 0.0f)) / 
		mCharAdvance.x), 0);
	mFirstVisibleColumn = std::max((int)(std::max(aScrollX - mTextStart, 0.0f) / mCharAdvance.x), 0);
	mLastVisibleColumn = std::max((int)((mContentWidth + aScrollX - mTextStart) / mCharAdvance.x), 0);
}

void TextSelect::Render(bool aParentIsFocused)
{
	/* Compute mCharAdvance regarding to scaled font size (Ctrl + mouse wheel)*/
	const float fontWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(),
		FLT_MAX, -1.0f, "#", nullptr, nullptr).x;
	const float fontHeight = ImGui::GetTextLineHeightWithSpacing();
	mCharAdvance = ImVec2(fontWidth, fontHeight * mLineSpacing);

	// Deduce mTextStart by evaluating mLines size (global lineMax) plus two spaces as text width
	mTextStart = mLeftMargin;
	static char lineNumberBuffer[16];

	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	mScrollX = ImGui::GetScrollX();
	mScrollY = ImGui::GetScrollY();
	UpdateViewVariables(mScrollX, mScrollY);

	int maxColumnLimited = 0;
	if (!mLines.empty())
	{
		auto drawList = ImGui::GetWindowDrawList();
		float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), 
			FLT_MAX, -1.0f, " ", nullptr, nullptr).x;

		for (int lineNo = mFirstVisibleLine; lineNo <= mLastVisibleLine && 
			lineNo < mLines.size(); lineNo++)
		{
			ImVec2 lineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + 
				lineNo * mCharAdvance.y);
			ImVec2 textScreenPos = ImVec2(lineStartScreenPos.x + mTextStart, lineStartScreenPos.y);

			auto& line = mLines[lineNo];
			maxColumnLimited = std::max(GetLineMaxColumn(lineNo, mLastVisibleColumn), maxColumnLimited);

			Coordinates lineStartCoord(lineNo, 0);
			Coordinates lineEndCoord(lineNo, maxColumnLimited);

			// Draw selection for the current line
			
			float rectStart = -1.0f;
			float rectEnd = -1.0f;
			Coordinates cursorSelectionStart = mState.mCursor.GetSelectionStart();
			Coordinates cursorSelectionEnd = mState.mCursor.GetSelectionEnd();
			assert(cursorSelectionStart <= cursorSelectionEnd);

			if (cursorSelectionStart <= lineEndCoord)
				rectStart = cursorSelectionStart > lineStartCoord ? 
				TextDistanceToLineStart(cursorSelectionStart) : 0.0f;
			if (cursorSelectionEnd > lineStartCoord)
				rectEnd = TextDistanceToLineStart(cursorSelectionEnd < lineEndCoord ? 
					cursorSelectionEnd : lineEndCoord);
			if (cursorSelectionEnd.mLine > lineNo || cursorSelectionEnd.mLine == lineNo &&
				cursorSelectionEnd > lineEndCoord)
				rectEnd += mCharAdvance.x;

			if (rectStart != -1 && rectEnd != -1 && rectStart < rectEnd)
				drawList->AddRectFilled(
					ImVec2{ lineStartScreenPos.x + mTextStart + rectStart, lineStartScreenPos.y },
					ImVec2{ lineStartScreenPos.x + mTextStart + rectEnd, lineStartScreenPos.y 
					+ mCharAdvance.y },
					0x6e7a8580);
			

			std::vector<Coordinates> cursorCoordsInThisLine;
		
			if (mState.mCursor.mInteractiveEnd.mLine == lineNo)
				cursorCoordsInThisLine.push_back(mState.mCursor.mInteractiveEnd);
			
			if (cursorCoordsInThisLine.size() > 0)
			{
				bool focused = ImGui::IsWindowFocused() || aParentIsFocused;

				// Render the cursors
				if (focused)
				{
					const auto& cursorCoords = cursorCoordsInThisLine[0];
					float width = 1.0f;
					auto cindex = GetCharacterIndexR(cursorCoords);
					float cx = TextDistanceToLineStart(cursorCoords);

					ImVec2 cstart(textScreenPos.x + cx, lineStartScreenPos.y);
					ImVec2 cend(textScreenPos.x + cx + width, lineStartScreenPos.y + mCharAdvance.y);
					drawList->AddRectFilled(cstart, cend, 0xff8000ff);
					
				}
			}

			// Render colorized text
			static std::string glyphBuffer;
			int charIndex = GetFirstVisibleCharacterIndex(lineNo);
			int column = mFirstVisibleColumn; // can be in the middle of tab character
			while (charIndex < mLines[lineNo].size() && column <= mLastVisibleColumn)
			{
				auto& glyph = line[charIndex];
				ImVec2 targetGlyphPos = { lineStartScreenPos.x + mTextStart + 
					TextDistanceToLineStart({lineNo, column}, false), lineStartScreenPos.y };

				if (glyph.mChar == '\t')
				{
					if (mShowWhitespaces)
					{
						ImVec2 p1, p2, p3, p4;

						const auto s = ImGui::GetFontSize();
						const auto x1 = targetGlyphPos.x + mCharAdvance.x * 0.3f;
						const auto y = targetGlyphPos.y + fontHeight * 0.5f;

						if (mShortTabs)
						{
							const auto x2 = targetGlyphPos.x + mCharAdvance.x;
							p1 = ImVec2(x1, y);
							p2 = ImVec2(x2, y);
							p3 = ImVec2(x2 - s * 0.16f, y - s * 0.16f);
							p4 = ImVec2(x2 - s * 0.16f, y + s * 0.16f);
						}
						else
						{
							const auto x2 = targetGlyphPos.x + TabSizeAtColumn(column) * 
								mCharAdvance.x - mCharAdvance.x * 0.3f;
							p1 = ImVec2(x1, y);
							p2 = ImVec2(x2, y);
							p3 = ImVec2(x2 - s * 0.2f, y - s * 0.2f);
							p4 = ImVec2(x2 - s * 0.2f, y + s * 0.2f);
						}

						drawList->AddLine(p1, p2, 0xffffff15);
						drawList->AddLine(p2, p3, 0xffffff15);
						drawList->AddLine(p2, p4, 0xffffff15);
					}
				}
				else if (glyph.mChar == ' ')
				{
					if (mShowWhitespaces)
					{
						const auto s = ImGui::GetFontSize();
						const auto x = targetGlyphPos.x + spaceSize * 0.5f;
						const auto y = targetGlyphPos.y + s * 0.5f;
						drawList->AddCircleFilled(ImVec2(x, y), 1.5f, 0xffffff15, 4);
					}
				}
				else
				{
					int seqLength = UTF8CharLength(glyph.mChar);
				
					glyphBuffer.clear();
					for (int i = 0; i < seqLength; i++)
						glyphBuffer.push_back(line[charIndex + i].mChar);
					drawList->AddText(targetGlyphPos, glyph.mColor, glyphBuffer.c_str());
				}

				MoveCharIndexAndColumn(lineNo, charIndex, column);
			}
		}
	}
	mCurrentSpaceHeight = (mLines.size() + std::min(mVisibleLineCount - 1, (int)mLines.size())) * 
		mCharAdvance.y;
	mCurrentSpaceWidth = std::max((maxColumnLimited + std::min(mVisibleColumnCount - 1, 
		maxColumnLimited)) * mCharAdvance.x, mCurrentSpaceWidth);

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Dummy(ImVec2(mCurrentSpaceWidth, mCurrentSpaceHeight));

	if (mEnsureCursorVisible > -1)
	{
		for (int i = 0; i < (mEnsureCursorVisibleStartToo ? 2 : 1); i++) 
			// first pass for interactive end and second pass for interactive start
		{
			if (i) UpdateViewVariables(mScrollX, mScrollY);
			// second pass depends on changes made in first pass
			Coordinates targetCoords = GetSanitizedCursorCoordinates(mEnsureCursorVisible); 
			// cursor selection end or start
			if (targetCoords.mLine <= mFirstVisibleLine)
			{
				float targetScroll = std::max(0.0f, (targetCoords.mLine - 0.5f) * mCharAdvance.y);
				if (targetScroll < mScrollY)
					ImGui::SetScrollY(targetScroll);
			}
			if (targetCoords.mLine >= mLastVisibleLine)
			{
				float targetScroll = std::max(0.0f, (targetCoords.mLine + 1.5f) *
					mCharAdvance.y - mContentHeight);
				if (targetScroll > mScrollY)
					ImGui::SetScrollY(targetScroll);
			}
			if (targetCoords.mColumn <= mFirstVisibleColumn)
			{
				float targetScroll = std::max(0.0f, mTextStart + (targetCoords.mColumn - 0.5f) *
					mCharAdvance.x);
				if (targetScroll < mScrollX)
					ImGui::SetScrollX(mScrollX = targetScroll);
			}
			if (targetCoords.mColumn >= mLastVisibleColumn)
			{
				float targetScroll = std::max(0.0f, mTextStart + (targetCoords.mColumn + 0.5f) * 
					mCharAdvance.x - mContentWidth);
				if (targetScroll > mScrollX)
					ImGui::SetScrollX(mScrollX = targetScroll);
			}
		}
		mEnsureCursorVisible = -1;
	}
	if (mScrollToTop)
	{
		ImGui::SetScrollY(0.0f);
		mScrollToTop = false;
	}
	if (mSetViewAtLine > -1)
	{
		float targetScroll;
		switch (mSetViewAtLineMode)
		{
		default:
		case SetViewAtLineMode::FirstVisibleLine:
			targetScroll = std::max(0.0f, (float)mSetViewAtLine * mCharAdvance.y);
			break;
		case SetViewAtLineMode::LastVisibleLine:
			targetScroll = std::max(0.0f, (float)(mSetViewAtLine - (mLastVisibleLine - mFirstVisibleLine)) *
				mCharAdvance.y);
			break;
		case SetViewAtLineMode::Centered:
			targetScroll = std::max(0.0f, ((float)mSetViewAtLine - (float)(mLastVisibleLine -
				mFirstVisibleLine) * 0.5f) * mCharAdvance.y);
			break;
		}
		ImGui::SetScrollY(targetScroll);
		mSetViewAtLine = -1;
	}
}



void TextSelect::OnLineChanged(bool aBeforeChange, int aLine, int aColumn, int aCharCount, bool aDeleted) 
// adjusts cursor position when other cursor writes/deletes in the same line
{
	static std::unordered_map<int, int> cursorCharIndices;
	if (aBeforeChange)
	{
		cursorCharIndices.clear();
		
		if (mState.mCursor.mInteractiveEnd.mLine == aLine && // cursor is at the line
			mState.mCursor.mInteractiveEnd.mColumn > aColumn && // cursor is to the right of changing part
			mState.mCursor.GetSelectionEnd() == mState.mCursor.GetSelectionStart()) 
			// cursor does not have a selection
		{
			cursorCharIndices[0] = GetCharacterIndexR({ aLine, mState.mCursor.mInteractiveEnd.mColumn });
			cursorCharIndices[0] += aDeleted ? -aCharCount : aCharCount;
		}
		
	}
	else
	{
		SetCursorPosition({ aLine, GetCharacterColumn(aLine, cursorCharIndices[0]) });
	}
}

void TextSelect::AddUndo(UndoRecord& aValue)
{
	assert(!mReadOnly);
	mUndoBuffer.resize((size_t)(mUndoIndex + 1));
	mUndoBuffer.back() = aValue;
	++mUndoIndex;
}

ImU32 TextSelect::ParseHexColor(const std::string& hex) {
	std::string h = hex;
	if (h.length() == 3) {
		// Expand #rgb to #rrggbb
		h = { h[0], h[0], h[1], h[1], h[2], h[2] };
	}
	if (h.length() == 6) {
		unsigned int rgb = std::stoul(h, nullptr, 16);
		return 0xFF000000 | rgb; // ARGB, alpha=0xFF
	}
	if (h.length() == 8) {
		unsigned int argb = std::stoul(h, nullptr, 16);
		return argb;
	}
	return 0xFFFFFFFF; // fallback: white
}
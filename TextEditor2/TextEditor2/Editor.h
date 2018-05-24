#pragma once
#include"TextBlock.h"
#include<vector>
#include<fstream>

const uint8_t page_size_max = UINT8_MAX;

class Editor
{
public:
	enum Mode { VIM_COMMAND, VIM_INSERT,VIM_LAST_LINE,CANVAS};
private:
	HWND hwnd;
	Mode mode = VIM_INSERT;
	LOGFONT lfont;
	vector<TextBlock> textBlocks;
public:
	Editor(HWND& hwnd);
	void ChangeMode(Mode mode);
	void Insert(wchar_t ch);
	void Insert(const wstring& str);
	void Delete();
	void Delete(size_t count);
	void Backspace();
	void Redo();
	void Undo();
	bool CursorLeft();
	bool CursorRight();
	bool CursorUp();
	bool CursorDown();
	bool SetCursor(size_t lineIndex, line_size_type charIndex);
	bool SetCursor(POINTS point);
	//debug functionaa
	void Resize(size_t height,size_t width);
	void SelectBegin();
	void SelectEnd();
	bool UpSelect();
	bool DownSelect();
	bool LeftSelect();
	bool RightSelect();
	void Copy();
	void Paste();
	void SetFocus();
	void KillFocus();
	void Paint(HDC& hdc);
};
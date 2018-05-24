#include "Editor.h"

Editor::Editor(HWND& hwnd)
	:hwnd(hwnd)
{
	//set font
	lfont.lfHeight = 18;
	lfont.lfWidth = 9;
	lfont.lfPitchAndFamily &= ~(1 << 0);
	lfont.lfPitchAndFamily &= ~(1 << 1);
	lfont.lfPitchAndFamily |= FIXED_PITCH;
	//set caret
	CreateCaret(hwnd, (HBITMAP)NULL, lfont.lfWidth, lfont.lfHeight);

	//debug
	RECT rect; GetClientRect(hwnd, &rect);
	textBlocks.push_back(TextBlock(hwnd, rect, lfont));
}

void Editor::ChangeMode(Mode mode)
{
	this->mode=mode;
}

void Editor::Insert(wchar_t ch)
{
	textBlocks.front().Insert(ch);
}

void Editor::Insert(const wstring & str)
{
	textBlocks.front().Insert(str);
}

void Editor::Delete()
{
	textBlocks.front().Delete();
}

void Editor::Delete(size_t count)
{
	textBlocks.front().Delete(count);
}

void Editor::Backspace()
{
	textBlocks.front().Backspace();
}

void Editor::Redo()
{
	textBlocks.front().Redo();
}

void Editor::Undo()
{
	textBlocks.front().Undo();
}

bool Editor::CursorLeft()
{
	return textBlocks.front().CursorLeft();
}

bool Editor::CursorRight()
{
	return textBlocks.front().CursorRight();
}

bool Editor::CursorUp()
{
	return textBlocks.front().CursorUp();
}

bool Editor::CursorDown()
{
	return textBlocks.front().CursorDown();
}

bool Editor::SetCursor(size_t lineIndex, line_size_type charIndex)
{
	return textBlocks.front().SetCursor(lineIndex, charIndex);
}

bool Editor::SetCursor(POINTS point)
{
	return textBlocks.front().SetCursor(point);
}

void Editor::Resize(size_t height,size_t width)
{
	textBlocks.front().Resize(height, width);
}

void Editor::SelectBegin()
{
	textBlocks.front().SelectBegin();
}

void Editor::SelectEnd()
{
	textBlocks.front().SelectEnd();
}

bool Editor::UpSelect()
{
	if (!CursorUp())return false;
	CursorDown();
	SelectBegin();
	if (textBlocks.front().CursorUp() == false)//如果没有上一行
	{
		while (textBlocks.front().GetCursorPos().second != 1)//行不变 列移动到当前行的起始
			textBlocks.front().CursorLeft();
	}
	SelectEnd();
}
bool Editor::DownSelect()
{
	if (!CursorDown())return false;
	CursorUp();
	SelectBegin();
	if (textBlocks.front().CursorDown() == false)//如果没有下一行
	{
		//行不变 列移动到当前行的末尾
		while (textBlocks.front().CursorRight() != false) {}
	}
	SelectEnd();
}
bool Editor::LeftSelect()
{
	if (!CursorLeft())return false;
	CursorRight();
	SelectBegin();
	textBlocks.front().CursorLeft();//调用光标左移函数
	SelectEnd();
}
bool Editor::RightSelect()
{
	if (!CursorRight())return false;
	CursorLeft();
	SelectBegin();
	textBlocks.front().CursorRight();//调用光标右移函数
	SelectEnd();
}

void Editor::Copy()
{
	if (!textBlocks.front().HasSelect())return;
	if (OpenClipboard(hwnd))
	{
		EmptyClipboard();//清空剪切板内容
		wstring str = textBlocks.front().GetSelectString();
		HGLOBAL hClip= GlobalAlloc(GHND, str.length());//分配新全局内存空间
		wchar_t *pBuf = (wchar_t *)GlobalLock(hClip);//锁住全局内存空间
		memcpy(pBuf, str.data(), str.length());//将内容写入全局内存空间
		//将空间中的内容写入剪切板
		SetClipboardData(CF_UNICODETEXT, hClip);//设置数据
		GlobalUnlock(hClip);//解锁
		GlobalFree(hClip);//释放全局内存空间
		CloseClipboard();//关闭剪切板
	}
}

void Editor::Paste()
{
	if (OpenClipboard(hwnd)) { // 打开剪贴板
		HANDLE hData = GetClipboardData(CF_UNICODETEXT); // 获取剪贴板数据句柄
		CloseClipboard(); // 关闭剪贴板
	}
}


void Editor::SetFocus()
{
	CreateCaret(hwnd, (HBITMAP)NULL, lfont.lfWidth, lfont.lfHeight);
	ShowCaret(hwnd);
}

void Editor::KillFocus()
{
	DestroyCaret();
}

void Editor::Paint(HDC& hdc)
{
	textBlocks.front().Paint(hdc);
}

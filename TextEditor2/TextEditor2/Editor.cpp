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
	SelectBegin();
	if (textBlocks.front().CursorUp() == false)//���û����һ��
	{
		while (textBlocks.front().GetCursorPos().second != 1)//�в��� ���ƶ�����ǰ�е���ʼ
			textBlocks.front().CursorLeft();
	}
	SelectEnd();
}
bool Editor::DownSelect()
{
	if (!CursorDown())return false;
	SelectBegin();
	if (textBlocks.front().CursorDown() == false)//���û����һ��
	{
		//�в��� ���ƶ�����ǰ�е�ĩβ
		while (textBlocks.front().CursorRight() != false) {}
	}
	SelectEnd();
}
bool Editor::LeftSelect()
{
	if (!CursorLeft())return false;
	textBlocks.front().CursorLeft();//���ù�����ƺ���
}
bool Editor::RightSelect()
{
	if (!CursorRight())return false;
	textBlocks.front().CursorRight();//���ù�����ƺ���
}

void Editor::Copy()
{
	if (!textBlocks.front().HasSelect())return;
	if (OpenClipboard(hwnd))
	{
		EmptyClipboard();//��ռ��а�����
		wstring str = textBlocks.front().GetSelectString();
		HGLOBAL hClip= GlobalAlloc(GHND, str.length());//������ȫ���ڴ�ռ�
		wchar_t *pBuf = (wchar_t *)GlobalLock(hClip);//��סȫ���ڴ�ռ�
		memcpy(pBuf, str.data(), str.length());//������д��ȫ���ڴ�ռ�
		//���ռ��е�����д����а�
		SetClipboardData(CF_UNICODETEXT, hClip);//��������
		GlobalUnlock(hClip);//����
		GlobalFree(hClip);//�ͷ�ȫ���ڴ�ռ�
		CloseClipboard();//�رռ��а�
	}
}

void Editor::Paste()
{
	if (OpenClipboard(hwnd)) { // �򿪼�����
		HANDLE hData = GetClipboardData(CF_UNICODETEXT); // ��ȡ���������ݾ��
		CloseClipboard(); // �رռ�����
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
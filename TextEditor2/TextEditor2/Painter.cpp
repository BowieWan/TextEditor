#include "Painter.h"

Painter::Painter(HWND hwnd,const RECT& bound)
	:hwnd(hwnd),bound(bound)
{}

RECT Painter::GetBound() const
{
	return bound;
}

HWND Painter::GetHWND() const
{
	return hwnd;
}

void Painter::SetPaintBegin(list<Line>::const_iterator begin, size_t beginIndex)
{
	this->paintBegin = begin;
	this->beginIndex = beginIndex;
}

void Painter::SetPaintLineCount(page_size_type lineCount)
{
	this->paintLineCount = lineCount;
}

void Painter::SetEnd(list<Line>::const_iterator end)
{
	this->end = end;
}

void Painter::SetBound(const RECT & bound)
{
	this->bound = bound;
}

size_t Painter::GetBeginLine() const
{
	return beginIndex;
}

bool Painter::OnPage(size_t lineIndex)
{
	return lineIndex >= beginIndex && (lineIndex - beginIndex) < paintLineCount;
}

void Painter::SlideUp()
{
	--beginIndex;
	--paintBegin;
}

void Painter::SlideDown()
{
	++beginIndex;
	++paintBegin;
}

void Painter::TextReady()
{
	RedrawWindow(hwnd, &bound, NULL, RDW_ERASE | RDW_INVALIDATE);
}

void Painter::PaintCaret(size_t lineIndex, line_size_type charIndex,bool isSoft,const LOGFONT& lfont)
{
	if(OnPage(lineIndex))
		SetCaretPos((charIndex - 1)*lfont.lfWidth+bound.left, (lineIndex - beginIndex)*lfont.lfHeight + bound.top);
}

void Painter::PaintText(const HDC & hdc,const LOGFONT & lf)
{
	HFONT hfont = CreateFontIndirect(&lf);
	SelectObject(hdc, hfont);
	auto it = paintBegin;
	for (auto i = 0; i <= paintLineCount&&it!=end; ++it, ++i) {
		TextOut(hdc,bound.left, bound.top+ i*lf.lfHeight, it->Data().data(), it->Length());
	}
	DeleteObject(hfont);
}

void Painter::PaintText(const HDC & hdc, const LOGFONT & lf, pair<size_t, line_size_type> selectBegin, pair<size_t, line_size_type> selectEnd)
{
	HFONT hfont = CreateFontIndirect(&lf);
	SelectObject(hdc, hfont);
	auto it = paintBegin;
	if (selectEnd.first == selectBegin.first) {
		for (auto i = 0; i <= paintLineCount && it != end; ++it, ++i) {
			if (beginIndex + i == selectEnd.first) {
				wstring front = it->Substr(1, selectBegin.second - 1);
				wstring substr = it->Substr(selectBegin.second, selectEnd.second - selectBegin.second + 1);
				wstring rare = it->Substr(selectEnd.second + 1);
				TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, front.data(), front.length());
				SetBkColor(hdc, RGB(0, 0, 0));
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, bound.left+selectBegin.second*lf.lfWidth, bound.top + i * lf.lfHeight, substr.data(), substr.length());
				SetBkColor(hdc, RGB(255, 255, 255));
				SetTextColor(hdc, RGB(0, 0, 0));
				TextOut(hdc, bound.left + selectEnd.second*lf.lfWidth, bound.top + i * lf.lfHeight, rare.data(),rare.length());
			}
			else TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, it->Data().data(), it->Length());
		}
	}
	else {
		for (auto i = 0; i <= paintLineCount && it != end; ++it, ++i) {
			if ((beginIndex + i) > selectBegin.first && (beginIndex + i) < selectEnd.first) {
				SetBkColor(hdc, RGB(0, 0, 0));
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, it->Data().data(), it->Length());
				SetBkColor(hdc, RGB(255, 255, 255));
				SetTextColor(hdc, RGB(0, 0, 0));
			}
			else if (beginIndex + i == selectBegin.first) {
				wstring front = it->Substr(1, selectBegin.second - 1);
				wstring substr = it->Substr(selectBegin.second);
				TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, front.data(), front.length());
				SetBkColor(hdc, RGB(0, 0, 0));
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, bound.left + selectBegin.second*lf.lfWidth, bound.top + i * lf.lfHeight, substr.data(), substr.length());
				SetBkColor(hdc, RGB(255, 255, 255));
				SetTextColor(hdc, RGB(0, 0, 0));
			}
			else if (beginIndex + i == selectEnd.first){
				wstring rare = it->Substr(selectEnd.second + 1);
				wstring substr = it->Substr(1,selectBegin.second);
				SetBkColor(hdc, RGB(0, 0, 0));
				SetTextColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, substr.data(),substr.length());
				SetBkColor(hdc, RGB(255, 255, 255));
				SetTextColor(hdc, RGB(0, 0, 0));
				TextOut(hdc, bound.left + selectEnd.second*lf.lfWidth, bound.top + i * lf.lfHeight, rare.data(), rare.length());
			}
			else TextOut(hdc, bound.left, bound.top + i * lf.lfHeight, it->Data().data(), it->Length());
		}
	}
	DeleteObject(hfont);
}

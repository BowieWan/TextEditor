#pragma once
#include"Line.h"
#include<Windows.h>
using page_size_type = uint8_t;

class Painter
{
	const HWND hwnd;
	list<Line>::const_iterator paintBegin;
	list<Line>::const_iterator end;//∑¿÷π‘ΩΩÁ
	size_t beginIndex;
	page_size_type paintLineCount=0;
	RECT bound;


public:
	Painter(HWND hwnd,const RECT& bound);
	RECT GetBound()const;
	HWND GetHWND()const;
	void SetPaintBegin(list<Line>::const_iterator begin, size_t beginIndex);
	void SetPaintLineCount(page_size_type paintLineCount);
	void SetEnd(list<Line>::const_iterator end);
	void SetBound(const RECT& bound);
	size_t GetBeginLine()const;
	bool OnPage(size_t lineIndex);
	void SlideUp();
	void SlideDown();
	void TextReady();
	void PaintCaret(size_t lineIndex, line_size_type charIndex,bool isSoft,const LOGFONT& lfont);
	void PaintText(const HDC& hdc,const LOGFONT& lf);
	void PaintText(const HDC& hdc, const LOGFONT& lf, pair<size_t, line_size_type> selectBegin, pair<size_t, line_size_type> selectEnd);
};


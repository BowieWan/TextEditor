#include "TextBlock.h"

TextBlock::TextBlock(const HWND& hwnd, const RECT& bound, const LOGFONT& lf)
	:lfont(lf),cursor(text.begin(), 1),painter(hwnd,bound)
{
	text.push_back(Line(false, (bound.right - bound.left) / lfont.lfWidth));
	cursor.lineIt = text.begin();
	painter.SetPaintLineCount((bound.bottom - bound.top) / lfont.lfHeight);
	painter.SetPaintBegin(text.begin(), 1);
	painter.SetEnd(text.end());
}

void TextBlock::_Insert(const wstring & str)
{
	auto bound = painter.GetBound();
	if (str == L"\n") {
		auto next = cursor.lineIt; ++next;
		wstring right = cursor.lineIt->Delete(cursor.charIndex, cursor.lineIt->Length() - cursor.charIndex + 1).opString;
		if (cursor.lineIndex == text.size() || !next->IsSoft()) {
			Line line(false, (bound.right - bound.left) / lfont.lfWidth);
			line.Insert(1, right);
			++cursor.lineIt;
			++cursor.lineIndex;
			cursor.charIndex = 1;
			cursor.lineIt = text.insert(cursor.lineIt, move(line));
		}
		else {
			++cursor.lineIndex;
			++cursor.lineIt;
			cursor.charIndex = 1;
			cursor.lineIt->SetSoft(false);
			auto lineItBackup = cursor.lineIt;
			auto lineIndexBackup = cursor.lineIndex;
			_Insert(right);
			cursor.lineIt = lineItBackup;
			cursor.lineIndex = lineIndexBackup;
			cursor.charIndex = 1;
		}
	}
	else {
		LineOpInfo loi;
		//第一次插入
		if (cursor.lineIndex == text.size() + 1) {
			Line line(true, (bound.right - bound.left) / lfont.lfWidth);
			loi = line.Insert(cursor.charIndex, str);
			cursor.lineIt = text.insert(cursor.lineIt, move(line));
			++cursor.charIndex;
		}
		else loi = cursor.lineIt->Insert(cursor.charIndex, str);
		auto lineItBackup = cursor.lineIt;
		auto increase = loi.cursor.lineIncrease;
		cursor.charIndex = loi.cursor.charIndex;
		cursor.lineIndex += increase;

		//如果有溢出，继续调整插入
		while (loi.overRange > 0) {
			++cursor.lineIt;
			if (cursor.lineIt == text.end() || !cursor.lineIt->IsSoft()) {
				Line line(true, (bound.right - bound.left) / lfont.lfWidth);
				loi = line.Insert(1, loi.overString);
				cursor.lineIt = text.insert(cursor.lineIt, move(line));
			}
			else {
				loi = cursor.lineIt->Insert(1, loi.overString);
			}
		}
		//设置cursor
		cursor.lineIt = lineItBackup;
		for (; increase > 0; --increase)++cursor.lineIt;
	}
}

void TextBlock::_Insert(wchar_t ch)
{
	wstring str(1, ch);
	_Insert(str);
}

wchar_t TextBlock::_Delete()
{
	wstring str = _Delete(1);
	if (str == L"")return '\0';
	else return str[0];
}

wstring TextBlock::_Delete(size_t count)
{
	wstring deleted;
	LineOpInfo loi;
	line_size_type lineIndexBackup = cursor.lineIndex;
	auto lineItBackup = cursor.lineIt;
	for (; count > 0; --count, cursor.lineIndex = lineIndexBackup, cursor.lineIt = lineItBackup) {
		loi = cursor.lineIt->Delete(cursor.charIndex);
		if (loi.opString == L"") {
			if (cursor.lineIndex < text.size()) {
				auto next = cursor.lineIt; ++next;
				next->SetSoft(true);
				deleted.push_back('\n');
			}
			else return deleted;
		}
		else deleted.append(loi.opString);
		//调整
		auto next = cursor.lineIt; ++next;
		for (; loi.overRange < 0 && cursor.lineIndex < text.size() && next->IsSoft(); ) {
			bool enough = -loi.overRange < next->Length();
			if (enough) {
				loi = next->Delete(1, -loi.overRange);
				cursor.lineIt->Insert(cursor.lineIt->Length() + 1, loi.opString);
			}
			else {
				wstring data = next->Data();
				loi = cursor.lineIt->Insert(cursor.lineIt->Length() + 1, data);
				text.erase(next);
			}
			++cursor.lineIt;
			++cursor.lineIndex;
			if (cursor.lineIndex >= text.size())break;
			next = cursor.lineIt;
			++next;
		}
	}
	return deleted;
}

bool TextBlock::_Search(const wstring& pattern)
{
	auto it = text.begin();
	size_t textSize = text.size();
	wstring str;
	for (size_t line = 1; line <= textSize; ++line,++it){
		str = it->Data();
		size_t length = str.length();
		for (size_t begin = 0; begin < length;) {
			size_t pos = str.find(pattern, begin);
			if (pos == wstring::npos)break;
			searchResult.PushBack(pair <size_t, line_size_type>(line, pos+1));
			begin = pos + 1;
		}
	}
	return true;
}

void TextBlock::Insert(wchar_t ch)
{
	Insert(wstring(1, ch));
}

void TextBlock::Insert(const wstring & str)
{
	undoStack.push(TextOperation(TextOperation::INSERT_OP, str, cursor.lineIndex, cursor.charIndex));
	for (auto ch : str) {
		_Insert(ch);
	}
	painter.SetEnd(text.end());
	while (!painter.OnPage(cursor.lineIndex))painter.SlideDown();
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(), lfont);
	painter.TextReady();
}

void TextBlock::Delete()
{
	Delete(1);
}

void TextBlock::Delete(size_t count)
{
	wstring str = _Delete(count);
	if (str != L"") {
		undoStack.push(TextOperation(TextOperation::DELETE_OP, str, cursor.lineIndex, cursor.charIndex));
		painter.SetEnd(text.end());
		if (!painter.OnPage(cursor.lineIndex))painter.SetPaintBegin(cursor.lineIt, cursor.lineIndex);
		painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex > text.size() || cursor.lineIt->IsSoft(), lfont);
		painter.TextReady();
	}
}

void TextBlock::Backspace()
{
	if (!CursorLeft())return;
	Delete();
}

void TextBlock::Search(const wstring& pattern)
{
	if (!_Search(pattern)) {
		//求nextval
		size_t i = 1, j = 0;
		size_t* nextval = new size_t[pattern.length()];
		size_t length = pattern.size();
		while (i < length)
		{
			if (j == 0 || pattern[i] == pattern[j])
			{
				i++;
				j++;
				if (pattern[i] != pattern[j])
					nextval[i] = j;
				else nextval[i] = nextval[j];
			}
			else j = nextval[j];
		}
		//每一行
		auto it = text.begin();
		for (size_t line = 1; line <= text.size(); ++line) {
			size_t charIndex = 0;
			size_t i = 0, j = 0;
			wstring s = it->Data();
			size_t s_search_length = s.length();

			size_t s_length = s.size();
			while (i < s_length&&j < s_search_length)
			{
				if (j == -1 || s[i] == pattern[j]) { i++; j++; }
				else
					j = nextval[j];
			}
			if (j > s_search_length)
				charIndex = i - j;
			searchResult.PushBack(pair<size_t, line_size_type>(line, charIndex));
		}
	}
}

void TextBlock::Redo()
{
	if (redoStack.empty())return;
	TextOperation to(redoStack.top());
	if (to.type == TextOperation::INSERT_OP) {
		if (!SetCursor(to.lineIndex, to.charIndex))return;
		_Insert(to.opString);
	}
	else if (to.type == TextOperation::DELETE_OP) {
		if (!SetCursor(to.lineIndex, to.charIndex))return;
		_Delete(to.opString.length());
	}
	redoStack.pop();
	undoStack.push(move(to));
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	painter.TextReady();
}

void TextBlock::Undo()
{
	if (undoStack.empty())return;
	TextOperation to(undoStack.top());
	if (to.type == TextOperation::INSERT_OP) {
		if (!SetCursor(to.lineIndex, to.charIndex)) {
			abort();
		}
		_Delete(to.opString.length());
	}
	else if (to.type == TextOperation::DELETE_OP) {
		if (!SetCursor(to.lineIndex, to.charIndex)) {
			abort();
		}
		_Insert(to.opString);
	}
	undoStack.pop();
	redoStack.push(move(to));
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	painter.TextReady();
}

pair<size_t, line_size_type> TextBlock::GetCursorPos() const
{
	return pair<size_t, line_size_type>(cursor.lineIndex,cursor.charIndex);
}

bool TextBlock::CursorLeft()
{
	if (cursor.charIndex == 1) {
		if (cursor.lineIndex == 1) return false;
		else {
			--cursor.lineIndex;
			--cursor.lineIt;
			cursor.charIndex = cursor.lineIt->Length() + 1 > text.front().GetLimits() ?
				text.front().GetLimits() : cursor.lineIt->Length() + 1;
		}
	}
	else {
		--cursor.charIndex;
	}
	if(!painter.OnPage(cursor.lineIndex)) {
		painter.SlideUp();
		painter.TextReady();
	}
	if (selection.HasSelect())painter.TextReady();
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	return true;
}

bool TextBlock::CursorRight()
{
	if (cursor.lineIndex == text.size() + 1)return false;//最后的虚行
	else if(cursor.lineIndex==text.size()){//最后一行
		if (cursor.charIndex == cursor.lineIt->GetLimits()) {
			++cursor.lineIt;
			++cursor.lineIndex;
			cursor.charIndex = 1;
		}
		else if(cursor.charIndex<=cursor.lineIt->Length())++cursor.charIndex;
		else return false;
	}
	else {
		line_size_type length = cursor.lineIt->Length();
		if (cursor.charIndex < length)++cursor.charIndex;
		else if (cursor.charIndex == length) {
			if (length < cursor.lineIt->GetLimits())++cursor.charIndex;
			else {
				++cursor.lineIt;
				++cursor.lineIndex;
				cursor.charIndex = 1;
			}
		}
		else {
			++cursor.lineIt;
			++cursor.lineIndex;
			cursor.charIndex = 1;
		}
	}
	if(!painter.OnPage(cursor.lineIndex)) {
		painter.SlideDown();
		painter.TextReady();
	}
	if (selection.HasSelect())painter.TextReady();
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	return true;
}

bool TextBlock::CursorUp()
{
	if (cursor.lineIndex == 1)return false;
	--cursor.lineIndex;
	--cursor.lineIt;
	if (cursor.charIndex > cursor.lineIt->Length()) {
		cursor.charIndex = cursor.lineIt->Length() + 1;
	}
	if(!painter.OnPage(cursor.lineIndex)) {
		painter.SetPaintBegin(cursor.lineIt, cursor.lineIndex);
		painter.TextReady();
	}
	if (selection.HasSelect())painter.TextReady();
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	return true;
}

bool TextBlock::CursorDown()
{
	if (cursor.lineIndex >= text.size())return false;
	++cursor.lineIndex;
	++cursor.lineIt;
	if (cursor.lineIndex == text.size() + 1)cursor.charIndex = 1;
	else if (cursor.charIndex > cursor.lineIt->Length()) {
		cursor.charIndex = cursor.lineIt->Length() + 1;
	}
	if (!painter.OnPage(cursor.lineIndex)) {
		painter.SlideDown();
		painter.TextReady();
	}
	if (selection.HasSelect())painter.TextReady();
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	return true;
}

bool TextBlock::HasSelect() const
{
	return selection.HasSelect();
}

void TextBlock::SelectBegin()
{
	if (!selection.HasSelect()) {
		selection.SetBegin(cursor.lineIndex, cursor.charIndex);
	}
}
//在每次调用该函数结束的时候判断begin与end的位置状况
void TextBlock::SelectEnd()
{
	if (cursor.lineIndex == text.size() + 1) {//不允许选中最后的空行
		CursorLeft();
	}
	selection.SetEnd(cursor.lineIndex, cursor.charIndex);
}


void TextBlock::SelectClear()
{
	selection.Clear();
}

void TextBlock::SetFont(const LOGFONT & lf)
{
	lfont = lf;
}

LOGFONT TextBlock::GetFont() const
{
	return lfont;
}

pair<size_t, line_size_type> TextBlock::GetSelectBegin() const
{
	return selection.GetRange().first;
}

pair<size_t, line_size_type> TextBlock::GetSelectEnd() const
{
	return selection.GetRange().second;
}

wstring TextBlock::GetSelectString() const
{
	//定位
	auto it = text.begin();
	auto range = selection.GetRange();
	auto begin = range.first;
	auto end = range.second;
	for (size_t i = 1; i < begin.first; ++it, ++i);
	if (begin.first == end.first) {//同行
		return it->Substr(begin.second, end.second - begin.second+1);
	}
	else if (end.first - begin.first == 1) {//上下两行
		wstring str;
		str = it->Substr(begin.second);
		str.push_back('\n');
		++it;
		str.append(it->Substr(1, end.second));
		return str;
	}
	else {
		wstring str;
		str = it->Substr(begin.second);
		str.push_back('\n');
		++it;
		for (size_t count = end.first - begin.first; count-2 > 0; ++it, --count) {
			str.append(it->Data());
		}
		str.append(it->Substr(1, end.second));
		return str;
	}
}

bool TextBlock::SetCursor(size_t lineIndex, line_size_type charIndex)
{
	
	if (lineIndex < cursor.lineIndex) {//上移
		if (lineIndex == 0)//移动到begin
			for (; cursor.lineIt != text.begin(); --cursor.lineIt, --cursor.lineIndex);
		else for (size_t off = cursor.lineIndex - lineIndex; off > 0; --off, --cursor.lineIt, --cursor.lineIndex);
	}
	else {//下移
		if (lineIndex >= text.size() + 1) {//底部的特殊行
			auto last = --text.end();
			for (; cursor.lineIt != last; ++cursor.lineIt, ++cursor.lineIndex);
			if (last->Length() == last->GetLimits()) {
				++cursor.lineIt; ++cursor.lineIndex;
				cursor.charIndex = 1;
				if (charIndex != 1) return false;
				else return true;
			}
			else {
				cursor.charIndex = cursor.lineIt->Length() + 1;
				return false;
			}
		}
		else for (size_t off = lineIndex-cursor.lineIndex; off > 0; --off, ++cursor.lineIt, ++cursor.lineIndex);
	}
	//左右移
	bool result = true;
	line_size_type length = cursor.lineIt->Length();
	if (length == cursor.lineIt->GetLimits()) {
		if (charIndex > length) {
			result = false;
			cursor.charIndex = length;
		}
		else cursor.charIndex = charIndex;
	}
	else {
		if (charIndex > length + 1) {
			result = false;
			cursor.charIndex = length + 1;
		}
		else cursor.charIndex = charIndex;
	}
	painter.PaintCaret(cursor.lineIndex, cursor.charIndex, cursor.lineIndex >text.size() || cursor.lineIt->IsSoft(),lfont);
	return result;
}

bool TextBlock::SetCursor(POINTS point)
{
	auto bound = painter.GetBound();
	if (point.x<bound.left || point.x>bound.right || point.y<bound.top || point.y>bound.bottom)return false;
	page_size_type lineOff=(point.y-bound.top)/lfont.lfHeight;
	line_size_type charOff=(point.x-bound.left)/lfont.lfWidth;
	return SetCursor(painter.GetBeginLine() + lineOff,charOff+1);
}

void TextBlock::Resize(size_t height, size_t width)
{
	if (width == 1)return;
	InvalidateRect(painter.GetHWND(),&painter.GetBound(),true);
	RECT rect = painter.GetBound(); rect.right =rect.left+ width; rect.bottom =rect.top+ height;
	painter.SetBound(rect);
	line_size_type newCharLimits = width / lfont.lfWidth;
	if (newCharLimits == 0)return;
	line_size_type curCharLimits = text.front().GetLimits();
	page_size_type newLineLimits = height / lfont.lfHeight;
	if (newCharLimits != curCharLimits) {
		//调整
		if (newCharLimits < curCharLimits) {//缩窄
			auto it = text.begin();
			wstring overString = it->ChangeLimits(newCharLimits).overString;
			++it;
			for (; it != text.end(); ++it) {
				if (it->IsSoft()) {
					it->ChangeLimits(line_size_max);
					overString = it->Insert(1, overString).overString;
					overString.insert(0, it->ChangeLimits(newCharLimits).overString);
				}
				else {
					if (overString != L"") {
						Line line(true, newCharLimits);
						overString = line.Insert(1, overString).overString;
						it = text.insert(it, move(line));

					}
					it->ChangeLimits(line_size_max);
					overString = it->Insert(1, overString).overString;
					overString.insert(0, it->ChangeLimits(newCharLimits).overString);
				}
			}
			while (overString != L"") {
				Line line(true,newCharLimits);
				overString=line.Insert(1, overString).overString;
				text.push_back(move(line));
			}
		}
		else {//扩大
			auto it = text.begin(), next = it; ++next;
			for (line_size_type move = 0;next != text.end(); ++it,++next) {//假设text最少也有一行
				move = 0 - it->ChangeLimits(newCharLimits).overRange;
				if (next->IsSoft()) {
					if (move >= next->Length()) {
						move -= next->Length();
						it->Insert(it->Length() + 1, next->Data());
						next = text.erase(next);
						if (next == text.end())break;
					}
					else {
						LineOpInfo loi = next->Delete(1, move);
						it->Insert(it->Length() + 1,loi.opString);
						move = 0 - loi.overRange;
					}
				}
			}
		}
		SetCursor(1, 1);
		selection.Clear();
		while (!undoStack.empty())undoStack.pop();
		while (!redoStack.empty())redoStack.pop();
	}
	painter.SetEnd(text.end());
	painter.SetPaintLineCount(newLineLimits);
	painter.TextReady();
}

wstring TextBlock::ToString()
{
	wstring str;
	for (auto it = text.begin(); it != text.end(); ++it) {
		str.append(it->Data());
	}
	return str;
}

size_t TextBlock::LineCount() const
{
	return text.size();
}

line_size_type TextBlock::CurLineLength() const
{
	return cursor.lineIt->Length();
}

void TextBlock::Paint(HDC& hdc)
{
	auto range = selection.GetRange();
	if (selection.HasSelect())painter.PaintText(hdc, lfont, range.first, range.second);
	else painter.PaintText(hdc, lfont);
}

TextOperation::TextOperation(Type type, wstring opString, size_t lineIndex, line_size_type charIndex)
	:type(type), opString(opString), lineIndex(lineIndex), charIndex(charIndex)
{}

bool Selection::HasSelect() const
{
	return select;
}

void Selection::SetBegin(size_t lineIndex, line_size_type charIndex)
{
	select = true;
	begin.first = lineIndex;
	begin.second = charIndex;
}

void Selection::SetEnd(size_t lineIndex, line_size_type charIndex)
{
	select = true;
	end.first = lineIndex;
	end.second = charIndex;
}

void Selection::Clear()
{
	select = false;
}

pair<size_t, line_size_type> Selection::GetBegin() const
{
	return begin;
}

pair<size_t, line_size_type> Selection::GetEnd() const
{
	return end;
}

pair<pair<size_t, line_size_type>, pair<size_t, line_size_type>> Selection::GetRange() const
{
	pair<size_t, line_size_type> begin, end;
	if (begin.first < end.first || (begin.first == end.first && begin.second < end.second))
	{
		int tmp1, tmp2;
		tmp1 = begin.first; tmp2 = begin.second;
		begin.first = end.first; begin.second = end.second;
		end.first = tmp1;
	}
	return pair<pair<size_t, line_size_type>, pair<size_t, line_size_type>>(begin, end);
}
void SearchResult::Clear()
{
	avail = false;
	results.clear();
}

void SearchResult::PushBack(pair<size_t, line_size_type>&& result)
{
	avail = true;
	results.push_back(result);
}

bool SearchResult::IsAvail() const
{
	return avail;
}

vector<pair<size_t, line_size_type>>& SearchResult::Data()
{
	return results;
}

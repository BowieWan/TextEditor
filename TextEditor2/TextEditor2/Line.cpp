#include "Line.h"


Line::Line(bool isSoft, line_size_type limits)
{
	this->isSoft = isSoft;
	this->limits = limits;
}

LineOpInfo Line::ChangeLimits(line_size_type limits)
{
	LineOpInfo loi;
	this->limits = limits;
	loi.overRange = data.length() - limits;
	if (loi.overRange>0) {
		loi.overString = data.substr(limits);
		data.erase(limits);
	}
	return loi;
}

line_size_type Line::GetLimits()
{
	return limits;
}

wstring Line::Data()const
{
	return data;
}

size_t Line::Length()const
{
	return data.length();
}

wstring Line::Substr(line_size_type start, line_size_type count)const
{
	return data.substr(start - 1, count);
}

bool Line::IsSoft()
{
	return isSoft;
}

void Line::SetSoft(bool isSoft)
{
	this->isSoft = isSoft;
}

LineOpInfo Line::Insert(line_size_type index, wchar_t ch)
{
	wstring str(1, ch);
	return Insert(index, str);
}

LineOpInfo Line::Insert(line_size_type index, const wstring& str)
{
	LineOpInfo loi;
	data.insert(index - 1, str);
	loi.overRange = data.length() - limits;
	loi.SetCursor((index + str.length() - 1) / limits, (index + str.length() - 1) % limits+1);
	if (loi.overRange > 0) {
		loi.overString = data.substr(limits);
		data.erase(limits);
	}
	return loi;
}

LineOpInfo Line::Delete(line_size_type index)
{
	return Delete(index, 1);
}

LineOpInfo Line::Delete(line_size_type index, line_size_type count)
{
	LineOpInfo loi;
	loi.opString = data.substr(index - 1, count);
	data.erase(index - 1, count);
	loi.overRange = data.length() - limits;
	return loi;
}

void LineOpInfo::SetCursor(size_t lineIncrease,line_size_type charIndex)
{
	cursor.lineIncrease = lineIncrease;
	cursor.charIndex = charIndex;
	cursor.avail = true;
}

Cursor::Cursor(list<Line>::iterator begin,size_t lineIndex)
{
	lineIt = begin;
	this->lineIndex = lineIndex;
}

void Cursor::Set(size_t lineIndex, list<Line>::iterator lineIt, line_size_type charIndex)
{
	this->lineIndex = lineIndex;
	this->lineIt = lineIt;
	this->charIndex = charIndex;
}
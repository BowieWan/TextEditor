#pragma once
#include<string>
#include<list>
using namespace std;

using line_size_type = uint16_t;
using line_size_type_signed = int32_t;
const uint16_t line_size_max = UINT16_MAX;

struct LineOpInfo;
struct Cursor;

class Line
{
	bool isSoft = false;
	wstring data;
	line_size_type limits;

public:
	Line(bool isSoft, line_size_type limits);
	LineOpInfo ChangeLimits(line_size_type limits);
	line_size_type GetLimits();
	wstring Data()const;
	size_t Length()const;
	wstring Substr(line_size_type start, line_size_type count = line_size_max)const;
	bool IsSoft();
	void SetSoft(bool isSoft);

	//insert 不处理换行符
	//delete 不能跨行操作
	LineOpInfo Insert(line_size_type index, wchar_t ch);
	LineOpInfo Insert(line_size_type index, const wstring& str);
	LineOpInfo Delete(line_size_type index);
	LineOpInfo Delete(line_size_type index, line_size_type count);
};

struct LineOpInfo
{
	line_size_type_signed overRange = 0;//超了多少字符，负数表示离limits还有多少字符，overRange>0时，overString有效；overRange>=0,cursor表示在溢出串的位置
	wstring opString;	//delete 操作的作用字符串，没有换行符
	wstring overString;	//如果overRange>0，操作后溢出产生的字符串，没有换行符
	struct CursorInfo {
		bool avail = false;
		line_size_type charIndex;
		size_t lineIncrease;
	}cursor;
	void SetCursor(size_t lineIncrease,line_size_type charIndex);
};

struct Cursor
{
	size_t lineIndex = 1;
	line_size_type charIndex = 1;
	list<Line>::iterator lineIt;
	Cursor(list<Line>::iterator begin,size_t lineIndex);
	void Set(size_t lineIndex, list<Line>::iterator lineIt, line_size_type charIndex);
};
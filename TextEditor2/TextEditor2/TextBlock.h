#pragma once
#include"Line.h"
#include"Painter.h"
#include<vector>
#include<stack>

struct TextOperation
{
	enum Type { INSERT_OP, DELETE_OP }type;
	wstring opString;
	size_t lineIndex;
	line_size_type charIndex;
	TextOperation(Type type, wstring opString, size_t lineIndex, line_size_type charIndex);
};

class Selection
{
	bool select=false;
	pair<size_t, line_size_type> begin;
	pair<size_t, line_size_type> end;
public:
	bool HasSelect()const;
	void SetBegin(size_t lineIndex,line_size_type charIndex);
	void SetEnd(size_t lineIndex,line_size_type charIndex);
	void Clear();
	pair<size_t, line_size_type> GetBegin()const;
	pair<size_t, line_size_type> GetEnd()const;
	pair<pair<size_t, line_size_type>, pair<size_t, line_size_type>> GetRange()const;
};

class SearchResult
{
	bool avail = false;
	vector<pair<size_t, line_size_type>> results;
public:
	void Clear();
	void PushBack(pair<size_t, line_size_type>&& result);
	bool IsAvail()const;
	vector<pair<size_t, line_size_type>>& Data();
};

class TextBlock
{
private:
	list<Line> text;
	Cursor cursor;
	LOGFONT lfont;
	Painter painter;
	stack<TextOperation> undoStack, redoStack;//记录包括换行在内的字符操作
	Selection selection;
	SearchResult searchResult;
private:
	//自动调整行宽的插入，处理换行符，调整cursor，
	void _Insert(const wstring& str);
	void _Insert(wchar_t ch);
	//自动调整行宽的删除，删除空的软行，处理换行符，不调整cursor，删除失败返回空
	wchar_t _Delete();
	wstring _Delete(size_t count);//包括换行符
	bool _Search(const wstring& pattern);
public:
	//参数：
	//	hwnd:本窗口
	//	bound:初始位置
	//	lf:初始的字体
	TextBlock(const HWND& hwnd,const RECT& bound, const LOGFONT& lf);

	//返回值：TextBlock全部文字的字符串，包括换行符'\n'
	wstring ToString();

	//返回值：text的总行数
	size_t LineCount()const;

	//返回值：cursor所在的行的长度
	line_size_type CurLineLength()const;

	//功能：在当前游标位置插入
	void Insert(wchar_t ch);
	void Insert(const wstring& str);
	
	//功能：在当前游标位置删除
	void Delete();
	//参数：
	//	count:删除个数
	void Delete(size_t count);
	
	//功能：回退删除键
	void Backspace();

	//功能：全行查找
	void Search(const wstring& pattern);

	//功能：重做
	void Redo();

	//功能：撤销
	void Undo();

	pair<size_t, line_size_type> GetCursorPos()const;

	//功能：游标左移
	//返回值：能否左移
	bool CursorLeft();

	//功能：游标右移
	//返回值：能否右移
	bool CursorRight();

	//功能：游标上移
	//返回值：能否上移
	bool CursorUp();

	//功能：游标下移
	//返回值：能否下移
	bool CursorDown();

	bool HasSelect()const;

	//功能：开始选中，记录游标位置并作为选中字符
	void SelectBegin();

	//功能：选中结束
	void SelectEnd();

	//返回值：当前选中开始的游标位置
	pair<size_t, line_size_type> GetSelectBegin()const;

	//返回值：当前选中结束的游标位置
	pair<size_t, line_size_type> GetSelectEnd()const;

	//返回值：从选中开始到结束的字符串，包括换行符'\n'
	wstring GetSelectString()const;

	//功能：关闭选中模式
	void SelectClear();

	//功能：设置字体
	//参数：
	//	lf：字体
	void SetFont(const LOGFONT& lf);

	//返回值：当前使用的字体
	LOGFONT GetFont()const;

	//功能：直接将游标移动到指定位置
	//参数：
	//	lineIndex:移动到的行号（1为第一行）
	//	charIndex:移动到此行的字符下标（1为第一个）
	//返回值：是否能移动到指定位置
	//注意：如果无法移动到指定位置，则自动移动到最近的位置
	bool SetCursor(size_t lineIndex, line_size_type charIndex);

	//功能：将游标移动到鼠标位置
	//参数：
	//	point:坐标结构体
	bool SetCursor(POINTS point);

	//功能：当字体/显示范围大小变化时调用
	//参数：
	//	height:新高度
	//	width:新宽度
	void Resize(size_t height, size_t width);

	void ShowWarning(const wstring& msg);

	//功能
	void Paint(HDC& hdc);
};
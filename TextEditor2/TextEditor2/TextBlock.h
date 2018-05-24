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
	stack<TextOperation> undoStack, redoStack;//��¼�����������ڵ��ַ�����
	Selection selection;
	SearchResult searchResult;
private:
	//�Զ������п�Ĳ��룬�����з�������cursor��
	void _Insert(const wstring& str);
	void _Insert(wchar_t ch);
	//�Զ������п��ɾ����ɾ���յ����У������з���������cursor��ɾ��ʧ�ܷ��ؿ�
	wchar_t _Delete();
	wstring _Delete(size_t count);//�������з�
	bool _Search(const wstring& pattern);
public:
	//������
	//	hwnd:������
	//	bound:��ʼλ��
	//	lf:��ʼ������
	TextBlock(const HWND& hwnd,const RECT& bound, const LOGFONT& lf);

	//����ֵ��TextBlockȫ�����ֵ��ַ������������з�'\n'
	wstring ToString();

	//����ֵ��text��������
	size_t LineCount()const;

	//����ֵ��cursor���ڵ��еĳ���
	line_size_type CurLineLength()const;

	//���ܣ��ڵ�ǰ�α�λ�ò���
	void Insert(wchar_t ch);
	void Insert(const wstring& str);
	
	//���ܣ��ڵ�ǰ�α�λ��ɾ��
	void Delete();
	//������
	//	count:ɾ������
	void Delete(size_t count);
	
	//���ܣ�����ɾ����
	void Backspace();

	//���ܣ�ȫ�в���
	void Search(const wstring& pattern);

	//���ܣ�����
	void Redo();

	//���ܣ�����
	void Undo();

	pair<size_t, line_size_type> GetCursorPos()const;

	//���ܣ��α�����
	//����ֵ���ܷ�����
	bool CursorLeft();

	//���ܣ��α�����
	//����ֵ���ܷ�����
	bool CursorRight();

	//���ܣ��α�����
	//����ֵ���ܷ�����
	bool CursorUp();

	//���ܣ��α�����
	//����ֵ���ܷ�����
	bool CursorDown();

	bool HasSelect()const;

	//���ܣ���ʼѡ�У���¼�α�λ�ò���Ϊѡ���ַ�
	void SelectBegin();

	//���ܣ�ѡ�н���
	void SelectEnd();

	//����ֵ����ǰѡ�п�ʼ���α�λ��
	pair<size_t, line_size_type> GetSelectBegin()const;

	//����ֵ����ǰѡ�н������α�λ��
	pair<size_t, line_size_type> GetSelectEnd()const;

	//����ֵ����ѡ�п�ʼ���������ַ������������з�'\n'
	wstring GetSelectString()const;

	//���ܣ��ر�ѡ��ģʽ
	void SelectClear();

	//���ܣ���������
	//������
	//	lf������
	void SetFont(const LOGFONT& lf);

	//����ֵ����ǰʹ�õ�����
	LOGFONT GetFont()const;

	//���ܣ�ֱ�ӽ��α��ƶ���ָ��λ��
	//������
	//	lineIndex:�ƶ������кţ�1Ϊ��һ�У�
	//	charIndex:�ƶ������е��ַ��±꣨1Ϊ��һ����
	//����ֵ���Ƿ����ƶ���ָ��λ��
	//ע�⣺����޷��ƶ���ָ��λ�ã����Զ��ƶ��������λ��
	bool SetCursor(size_t lineIndex, line_size_type charIndex);

	//���ܣ����α��ƶ������λ��
	//������
	//	point:����ṹ��
	bool SetCursor(POINTS point);

	//���ܣ�������/��ʾ��Χ��С�仯ʱ����
	//������
	//	height:�¸߶�
	//	width:�¿��
	void Resize(size_t height, size_t width);

	void ShowWarning(const wstring& msg);

	//����
	void Paint(HDC& hdc);
};
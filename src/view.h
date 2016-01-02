/*
	View 视图,即用于保存创建的View的数据结构.
	数据结构为:
	对于一个View, 它应该有一个名字, 并且有若干列(Column)
	对于一个Column, 它应该有一个名字, 并且有若干个Span
	对于一个Span, 它应该存有字符串的值, 并且存有起始终止位置
*/

#ifndef VIEW_H
#define VIEW_H

#include <vector>
#include <string>
#include <set>

using std::vector;
using std::string;
using std::set;

class Span {
public:
	Span(string text, int startPos, int endPos, int startWordPos, int endWordPos);
	string toString();    // 将Span格式化为形如: abc:(0,3) 的格式
	string getText() const;  // 仅获得存储的文本
	int getStartPos() const;
	int getEndPos() const;
	int getStartWordPosition() const;
	int getEndWordPosition() const;
	void addRecordID(int id);
	set<int> getRecordID() const;
	void clearRecordID();

private:
	string text;
	int startPosition;   // 起始字符位置
	int endPosition;     // 终止字符位置
	int startWordPosition;  // 起始单词位置
	int endWordPosition;   // 结束单词位置
	set<int> recordID;    // 分组的时候需要用到,用于保存它与匹配的最终结果的哪条记录有关联,非常重要!
};

class Column {
public:
	Column(string name);
	void setName(string name);
	string getName() const;
	void addSpan(Span newSpan);   // 将一个Span插入Column中
	vector<Span> getSpans() const; // 获得Column所储存的所有Span
	int getWidth() const;   // 获得当前列中Span的最大宽度,用于对齐输出
private:
	int maxSpanWidth;
	string name;
	vector<Span> spans;
};

class View {
public:
	View();
	View(string name);
	void addColumn(Column column);
	void setName(string newName);
	string getName() const;
	void output() const;   // 按照指定的规则输出表格
	void output(string viewName) const;  // 重载函数, 可以使用view的别名金星输出
	vector<Column> getColumns() const;  // 获得View所储存的所有Column
private:
	string name;
	vector<Column> columns;
};

#endif

#include "view.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

using std::cout;
using std::endl;
using std::setw;
using std::setfill;
using std::left;

Span::Span(string text, int startPos, int endPos, int startWordPos, int endWordPos) {
	this->text = text;
	this->startPosition = startPos;
	this->endPosition = endPos;
	this->startWordPosition = startWordPos;
	this->endWordPosition = endWordPos;
}

void Span::addRecordID(int id) {
	this->recordID.insert(id);
}

set<int> Span::getRecordID() const {
	return this->recordID;
}

void Span::clearRecordID() {
	this->recordID.clear();
}

string Span::toString() {
	char str[100000];
	sprintf(str, "%s:(%d,%d)", this->text.c_str(), this->startPosition, this->endPosition);
	return str;
}

string Span::getText() const {
	return this->text;
}

int Span::getStartPos() const {
	return this->startPosition;
}

int Span::getEndPos() const {
	return this->endPosition;
}

int Span::getStartWordPosition() const {
	return this->startWordPosition;
}

int Span::getEndWordPosition() const {
	return this->endWordPosition;
}

Column::Column(string name) {
	this->name = name;
	this->maxSpanWidth = name.length();
	this->spans.clear();
}

void Column::setName(string name) {
	this->name = name;
}

string Column::getName() const {
	return this->name;
}

void Column::addSpan(Span newSpan) {
	string spanStr = newSpan.toString();
	if (this->maxSpanWidth < int(spanStr.length())) {
		this->maxSpanWidth = int(spanStr.length());
	}
	this->spans.push_back(newSpan);
}

vector<Span> Column::getSpans() const {
	return this->spans;
}

int Column::getWidth() const {
	return this->maxSpanWidth;
}

View::View() {}

View::View(string name) {
	this->name = name;
	this->columns.clear();
}

void View::setName(string newName) {
	this->name = newName;
}

string View::getName() const {
	return this->name;
}

void View::addColumn(Column column) {
	this->columns.push_back(column);
}

vector<Column> View::getColumns() const {
	return this->columns;
}

void View::output() const {
	this->output(this->name);
}

void View::output(string viewName) const {
	cout << "View: " << viewName << endl;
	int columnSize = columns.size();
	for (int i = 0; i < columnSize ; i++) {
		cout << "+" << setfill('-') << setw(columns[i].getWidth() + 2) << "";
	}
	cout << "+" << endl;
	for (int i = 0; i < columnSize; i++) {
		cout << "| ";
		cout << setfill(' ') << setw(columns[i].getWidth() + 1) << left << columns[i].getName();
	}
	cout << "|" << endl;
	for (int i = 0; i < columnSize; i++) {
		cout << "+" << setfill('-') << setw(columns[i].getWidth() + 2) << "";
	}
	cout << "+" << endl;
	if (columnSize == 0 || columns[0].getSpans().size() == 0) {
		cout << "(Empty set)" << endl << endl;
	}
	else {
		int rowsCount = columns[0].getSpans().size();
		vector< vector<Span> > columnsSpans;
		for (int i = 0; i < columnSize; i++) {
			columnsSpans.push_back(columns[i].getSpans());
		}
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnSize; j++) {
				cout << "| ";
				cout << setfill(' ') << setw(columns[j].getWidth() + 1) << left << columnsSpans[j][i].toString();
			}
			cout << "|" << endl;
		}

		for (int i = 0; i < columnSize; i++) {
			cout << "+" << setfill('-') << setw(columns[i].getWidth() + 2) << "";
		}
		cout << "+" << endl;
		cout << rowsCount << " rows in set" << endl << endl;
	}
}

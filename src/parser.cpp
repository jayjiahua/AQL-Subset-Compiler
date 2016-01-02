#include "parser.h"
#include <iostream>
#include <cstdlib>
#include <set>
#include "regex.h"
#include "exception.h"

using std::cout;
using std::endl;

/* 全局辅助变量, 由于pattern_spec的特殊性, 不断相互调用,
   无法通过返回值维护, 因此需要全局变量才能维护以下的信息 */

// atom列表
vector<Atom> AtomList;

// 当前未闭合的分组集合
set<int> UnclosedGroup;

// 当前组号
int GroupNum = 1;

// 组号与特定的列对应的哈希表
map< int, vector<Span> > GroupMap;

Parser::Parser(Lexer &lexer, Tokenizer &tokenizer) : lexer(lexer), tokenizer(tokenizer) {
	this->move();

	/* 添加默认view: Document */
	this->document = tokenizer.getText();
	Span span(document, 0, document.length(), 0, 0);
	Column column("text");
	column.addSpan(span);
	views["Document"] = View("Document");
	views["Document"].addColumn(column);
}

void Parser::move() {
	this->look = this->lexer.scan();
}

string Parser::peek() {
	return this->look.getValue();
}

void Parser::syntaxError(string errorMsg) {
	lexer.printCurrentPosition();
	cout << "SyntaxError: " << errorMsg << endl;
	throw SyntaxErrorException();
}

void Parser::semanticError(string errorMsg) {
	lexer.printCurrentPosition();
	cout << "SemanticError: " << errorMsg << endl;
	throw SemanticErrorException();
}

void Parser::term(AQLToken::Tag tag) {
	if (this->look.getType() == tag) {
		this->move();
	}
	else {
		this->syntaxError("Unexpected \"" + this->look.getValue() + "\"");
	}
}

bool Parser::match(AQLToken::Tag tag) {
	return (this->look.getType() == tag);
}

void Parser::program() {
	while (!this->match(AQLToken::END)) {
		this->aqlStmt();
	}
}

void Parser::aqlStmt() {
	//cout << "In aqlStmt" << endl;
	if (this->match(AQLToken::CREATE)) {
		this->createStmt();
	}
	else if (this->match(AQLToken::OUTPUT)) {
		this->outputStmt();
	}
	else {
		this->syntaxError("Expected \"create\" or \"output\"");
	}
	this->term(AQLToken::SEMICOLON);
}

void Parser::createStmt() {
	//cout << "In createStmt" << endl;
	this->term(AQLToken::CREATE);
	this->term(AQLToken::VIEW);
	string id = this->peek();

	/* 后端代码 */
	// 若view ID 已经存在, 则抛出错误
	if (views.find(id) != views.end()) {
		this->semanticError("\"" + id + "\": The View ID has been exist!");
	}
	this->term(AQLToken::ID);
	this->term(AQLToken::AS);

	View view = this->viewStmt();
	view.setName(id);
	views[id] = view; // 保存View

	// 重置全局辅助变量
	AtomList.clear();
	UnclosedGroup.clear();
	GroupMap.clear();
	GroupNum = 1;
}

void Parser::outputStmt() {
	//cout << "In outputStmt" << endl;
	this->term(AQLToken::OUTPUT);
	this->term(AQLToken::VIEW);
	string firstId = this->peek();
	this->term(AQLToken::ID);
	string asId = this->alias();

	/* 后端代码 */
	if (views.find(firstId) != views.end()) {
		if (asId == "") {
			views[firstId].output();
		}
		else {
			views[firstId].output(asId);
		}
	}
	else {
		this->semanticError("\"" + firstId + "\": Undefined View ID!");
	}
}

View Parser::viewStmt() {
	//cout << "In viewStmt" << endl;
	if (this->match(AQLToken::SELECT)) {
		return this->selectStmt();
	}
	else if (this->match(AQLToken::EXTRACT)) {
		return this->extractStmt();
	}
	else {
		this->syntaxError("Expected \"select\" or \"extract\"");
	}
}

string Parser::alias() {
	//cout << "In alias" << endl;
	string id = "";
	if (this->match(AQLToken::AS)) {
		move();
		id = this->peek();
		this->term(AQLToken::ID);
	}
	return id;
}

View Parser::selectStmt() {
	//cout << "In selectStmt" << endl;
	this->term(AQLToken::SELECT);
	vector<SelectItem> selectList = this->selectList();
	this->term(AQLToken::FROM);
	map<string, string> fromList = this->fromList();

	/* 后端代码 */
	string viewName;
	if (views.find((*fromList.begin()).second) == views.end()) {
		this->semanticError("\"" + (*fromList.begin()).second + "\": Undefined View ID!");
	}
	else {
		viewName = (*fromList.begin()).first;
	}
	View view("");

	// 从指定视图找出对应的列
	for (size_t i = 0; i < selectList.size(); i++) {
		if (selectList[i].viewName != viewName) {
			this->semanticError("\"" + selectList[i].viewName + "\": Undefined View ID!");
		}
		else {
			vector<Column> columns = views[fromList[viewName]].getColumns();
			bool findColumn = false;
			for (size_t j = 0; j < columns.size(); j++) {
				if (columns[j].getName() == selectList[i].field) {
					Column newColumn = columns[j];
					if (selectList[i].alias != "") {
						newColumn.setName(selectList[i].alias);
					}
					view.addColumn(newColumn);
					findColumn = true;
					break;
				}
			}
			if (!findColumn) {
				this->semanticError("\"" + selectList[i].field + "\": Undefined Column ID!");
			}
		}
	}
	return view;
}

vector<SelectItem> Parser::selectList() {
	//cout << "In selectList" << endl;
	vector<SelectItem> slist;
	slist.push_back(this->selectItem());
	while (this->match(AQLToken::COMMA)) {
		move();
		slist.push_back(this->selectItem());
	}
	return slist;
}

SelectItem Parser::selectItem() {
	//cout << "In selectItem" << endl;
	string firstId = this->peek();
	this->term(AQLToken::ID);
	this->term(AQLToken::DOT);
	string secondId = this->peek();
	this->term(AQLToken::ID);
	string alias = this->alias();
	return SelectItem(firstId, secondId, alias);
}


map<string, string> Parser::fromList() {
	//cout << "In fromList" << endl;

	map<string, string> fromList;
	FromItem fromItem = this->fromItem();
	fromList[fromItem.id] = fromItem.viewName;
	while (this->match(AQLToken::COMMA)) {
		this->move();
		FromItem fromItem = this->fromItem();
		fromList[fromItem.id] = fromItem.viewName;
	}
	return fromList;
}

/* 构建结构化对象 */
FromItem Parser::fromItem() {
	//cout << "In fromItem" << endl;
	string firstId = this->peek();
	this->term(AQLToken::ID);
	string secondId = this->peek();
	this->term(AQLToken::ID);
	return FromItem(firstId, secondId);
}

/* 核心代码 处理extra_stmt */
View Parser::extractStmt() {
	//cout << "In extractStmt" << endl;
	this->term(AQLToken::EXTRACT);
	ExtractSpec _extractSpec = this->extractSpec();
	this->term(AQLToken::FROM);
	map<string, string> fromList = this->fromList();

	/* 后端代码 */
	vector<Token>& tokenList = _extractSpec.tokenList;
	vector<Atom>& atomList = _extractSpec.atomList;
	vector<Group>& groupList = _extractSpec.groupList;
	View view("");

	// 当语句为 extract REG 时,相对比较简单, 调用正则引擎的findall即可
	if (tokenList[0].getType() == AQLToken::REGEX) {
		string viewName = tokenList[2].getValue();
		string field = tokenList[3].getValue();
		if (fromList.find(viewName) != fromList.end()) {
			viewName = fromList[viewName];
		}
		else {
			this->semanticError("\"" + viewName + "\": Undefined View ID!");
		}
		string text;
		bool isFound = false;
		if (this->views.find(viewName) != this->views.end()) { // 判断 view ID 是否存在
			vector<Column> columns = this->views[viewName].getColumns();

			// 查找是否存在该列
			for (size_t i = 0; i < columns.size(); i++) {
				if (columns[i].getName() == field) {
					text = columns[i].getSpans()[0].getText();
					isFound = true;
				}
			}
			if (isFound) {
				vector< vector<int> > documentTokenList = findall(tokenList[1].getValue().c_str(), text.c_str());
				for (size_t k = 0; k < groupList.size(); k++) {
					Column column(groupList[k].id);
					for (size_t i = 0; i < documentTokenList.size(); i++) {
						// 根据正则引擎的返回值进行分组
						if (documentTokenList[i].size() >= groupList[k].groupNum * 2 + 2) {
							int start = documentTokenList[i][groupList[k].groupNum * 2];
							int end = documentTokenList[i][groupList[k].groupNum * 2 + 1];

							// 创建一个新的span并且将其插入列中
							Span span(text.substr(start, end - start), start, end,
								tokenizer.getWordPosition(start),
								tokenizer.getWordPosition(end - 1));
							column.addSpan(span);
						}
						else {
							this->semanticError("Group number is out of range!");
						}
					}
					view.addColumn(column);  // 将列插入view中
				}
			}
			else {
				this->semanticError("\"" + field + "\": Undefined Column ID!");
			}
		}
		else {
			this->semanticError("\"" + viewName + "\": Undefined View ID!");
		}
	} /* 当语句为extract pattern时, 处理相对复杂 */
	else if (tokenList[0].getType() == AQLToken::PATTERN) {
		/*
			处理pattern的基本思路:
			1. 关键在于column的合并(merge), 最后返回的结果就是一个新的column
			2. 对于<column>的情况,直接提取对应view中的列出来
			3. 对于REG, 调用正则引擎进行查找并构建一个新的列
			4. 对于<Token>, 不生成新的列, 下面再解释
			5. 故, 所有的atom都能看作是一个个的列
			6. 若存在2个以上的atom, 从左到右, 则从左向右依次合并, 每次合并产生新的中间Column,
				然后再用这个中间Column与第3个Column进行合并又得到一个新的中间Column
				例如: pattern <column1> <column2> <column3> <column4>
				最终的返回值: newColumn = merge(merge(merge(column1, column2), column3), column4)
			7. 若发现形如 <column>{1, 3} 的pattern, 处理方式是(假设下面的'+'为取并集的意思):
			   newColumn = column + merge(column, column) + merge(merge(column, colunm), merge)
			8. 对于形如 <column1> <Token>{1, 20} <column2>的情况, 则直接判断column1和column2的单词距离进行合并
				 故<Token>是不会产生中间列的, 也不能对其单独进行捕捉,
				 但对于(<column1> <Token>{1, 20} <column2>) 还是能够进行捕捉的
			9. 对于第一个patten, 需要进行特殊处理, 因为他是merge的起始点(第二列跟第一列合并, 那么第一列跟谁合并呢? 没有)

			关于分组:
			1. 维护一个map, GroupMap
				对应到 group_num -> column
			2. 新生成一个列后, 若发现它是有分组的, 就将他放到GroupMap中, 这时候会有两种情况:
			  (1) GroupMap对应的键值为空, 则直接放进去
				(2) GroupMap对应的键值已经存在一个列, 那么就使用merge函数将该两列合并
			3. 如何取出分组
					在group语句中得到要返回的分组号, 然后从GroupMap中提取列. 但是该列并不是最终要返回的结果,
				需要进行修剪, 因为随着合并, 结果的条目会越来越少, 而GroupMap却没有更新.
					这时候每个span保存的groupID集合就起作用了
					遍历group0的每个span(因为group0的条目总是正确的, 一直在被合并并且不断更新), 对于
				每个span, 查看它的recordID(正常来说, group0所有的span的recordID集合的大小都为1)
				然后在group_n中查找具有该recordID的span的条目,将其取出, 插入到新的列中,
				最后, 这个新的列就是正确的分组group_n, 与group0具有相同的条目数, 并且是匹配的.
		*/


		vector<Span> spanList = this->dealWithAtom(atomList[0], fromList);
		for (size_t i = 0; i < spanList.size(); i++) {
			spanList[i].addRecordID(i);
		}
		if (!atomList[0].groups.empty()) {
			set<int>::iterator it;
			for (it = atomList[0].groups.begin(); it != atomList[0].groups.end(); it++) {
				GroupMap[*it] = spanList;
			}
		}
		for (size_t i = 1; i < atomList.size(); i++) {
			int minCount = -1;
			int maxCount = 0;
			if (atomList[i].tokens[0].getType() == AQLToken::TOKEN) {
				minCount = atomList[i].minTime;
				maxCount = atomList[i].maxTime;
				i++;
			}
			// 多次迭代得到最后的列
			vector<Span> secondList = this->dealWithAtom(atomList[i], fromList);
			spanList = this->mergeColumn(spanList, secondList, minCount, maxCount);

			// 放入表中
			if (!atomList[i].groups.empty()) {
				set<int>::iterator it;
				for (it = atomList[i].groups.begin(); it != atomList[i].groups.end(); it++) {
					// 如果已经存在同一组的Column，则合并
					if (GroupMap.find(*it) == GroupMap.end()) {
						GroupMap[*it] = secondList;
					}
					else {
						vector<Span> copySecondList = secondList;
						GroupMap[*it] = this->mergeColumn(GroupMap[*it], copySecondList, minCount, maxCount);
					}
				}
			}

		}
		GroupMap[0] = spanList;
		for (size_t i = 0; i < groupList.size(); i++) {
			Column column(groupList[i].id);
			if (groupList[i].groupNum == 0) {
				for (size_t j = 0; j < spanList.size(); j++) {
					column.addSpan(spanList[j]);
				}
			}
			else {
				for (size_t j = 0; j < spanList.size(); j++) {
					vector<Span>& group = GroupMap[groupList[i].groupNum];
					for (size_t k = group.size() - 1; k >= 0; k--) {
						int recordId = *(spanList[j].getRecordID().begin());
						set<int>::iterator it;
						set<int> recordSet = group[k].getRecordID();
						if (recordSet.find(recordId) != recordSet.end()) {
							column.addSpan(group[k]);
							break;
						}
					}

				}
			}
			view.addColumn(column);
		}
	}
	return view;
}

/*
	核心算法: 合并两列

	 firstColumn为当前的列
	 secondColumn为将要被合并的列
	 minCount为最小的单词距离
	 maxCount为最大的单词距离
	 返回合并后的新的列

	合并算法的思路:
		对firstColumn和secondColumn进行遍历(二重循环),
		若发现某两个span的单词距离和字符距离均符合条件,
		则将第一列的span的起始字符位置和第二列的span的结束位置作为一个新的span的起始位置和结束位置,
		将第一列对应span的recordid插入到第二列对应span的recordID集合中(用于分组)
		然后将这个新的span插入到新的Column中
		将第一列对应Span的recordID插入到新的列对应的Span的recordID集合中
*/
vector<Span> Parser::mergeColumn(vector<Span>& firstColumn, vector<Span>& secondColumn, int minCount, int maxCount) {
	vector<Span> spanList;
	for (size_t i = 0; i < secondColumn.size(); i++) {
		secondColumn[i].clearRecordID();
	}
	for (size_t i = 0; i < firstColumn.size(); i++) {
		for (size_t j = 0; j < secondColumn.size(); j++) {
			if (firstColumn[i].getEndPos() <= secondColumn[j].getStartPos() &&
				secondColumn[j].getStartWordPosition() - firstColumn[i].getEndWordPosition() > minCount &&
				secondColumn[j].getStartWordPosition() - firstColumn[i].getEndWordPosition() <= maxCount + 1) {
				int start = firstColumn[i].getStartPos();
				int end = secondColumn[j].getEndPos();
				int startWord = tokenizer.getWordPosition(start);
				int endWord = tokenizer.getWordPosition(end - 1);
				Span span(this->document.substr(start, end - start).c_str(), start, end, startWord, endWord);
				if (!firstColumn[i].getRecordID().empty()) {
					span.addRecordID(*(firstColumn[i].getRecordID().begin()));
					secondColumn[j].addRecordID(*(firstColumn[i].getRecordID().begin()));
				}
				spanList.push_back(span);
				break;
			}
		}
	}
	return spanList;
}

/*
	核心算法: 处理atom
  这里的atom包括了 atom 和 atom{m,n}

	判断token, (ID.ID 或是 REG), 然后构建一个新的列
	若发现后面还有 {m, n}, 则调用merge算法对列进行合并
	例如: 对于 <column>{1, 3} 的pattern, 处理方式是(假设下面的'+'为取并集的意思):
		newColumn = column + merge(column, column) + merge(merge(column, colunm), merge)

	最后返回一个处理后的column
*/
vector<Span> Parser::dealWithAtom(Atom atom, map<string, string> fromList) {
	vector<Token>& tokens = atom.tokens;
	vector<Span> spanList;
	if (tokens[0].getType() == AQLToken::ID) {
		string viewName = tokens[0].getValue();
		if (fromList.find(viewName) != fromList.end()) {
			viewName = fromList[viewName];
		}
		else {
			this->semanticError("\"" + viewName + "\": Undefined View ID!");
		}
		View& view = views[viewName];
		vector<Column> columns = view.getColumns();
		bool isColumnExist = false;
		for (size_t j = 0; j < columns.size(); j++) {
			if (columns[j].getName() == tokens[1].getValue()) {
				spanList = columns[j].getSpans();
				isColumnExist = true;
			}
		}
		if (!isColumnExist) {
			this->semanticError("\"" + tokens[1].getValue() + "\": Undefined Column ID!");
		}
	}
	else if (tokens[0].getType() == AQLToken::REG) {
		vector< vector<int> > result = findall(tokens[0].getValue().c_str(), this->document.c_str());
		vector<Span> spans;
		for (size_t j = 0; j < result.size(); j++) {
			int start = result[j][0];
			int end = result[j][1];
			int startWord = tokenizer.getWordPosition(start);
			int endWord = tokenizer.getWordPosition(end - 1);
			Span span(this->document.substr(start, end - start), start, end, startWord, endWord);
			spans.push_back(span);
		}
		spanList = spans;
	}
	else {  // Token
		spanList = tokenizer.getTokens();
	}

	// 对{ }的操作, 处理 m ~ n次
	vector<Span> resultList;
	vector<Span> repeatSpanList = spanList;
	for (int i = 1; i < atom.minTime; i++) {
		repeatSpanList = this->mergeColumn(repeatSpanList, spanList);
	}
	resultList.insert(resultList.end(), repeatSpanList.begin(), repeatSpanList.end());
	for (int i = atom.minTime + 1; i <= atom.maxTime; i++) {
		repeatSpanList = this->mergeColumn(repeatSpanList, spanList);
		// 将结果加入到原来的列中
		resultList.insert(resultList.end(), repeatSpanList.begin(), repeatSpanList.end());
	}

	return resultList;
}



ExtractSpec Parser::extractSpec() {
	//cout << "In extractSpec" << endl;
	if (this->match(AQLToken::REGEX)) {
		return this->regexSpec();
	} else if (this->match(AQLToken::PATTERN)) {
		return this->patternSpec();
	}
	else {
		this->syntaxError("Expected \"regex\" or \"pattern\"");
	}
}

ExtractSpec Parser::regexSpec() {
	//cout << "In regexSpec" << endl;
	vector<Token> regexList;
	regexList.push_back(look);
	this->term(AQLToken::REGEX);
	regexList.push_back(look);
	this->term(AQLToken::REG);
	this->term(AQLToken::ON);
	vector<Token> colList = this->column();
	regexList.insert(regexList.end(), colList.begin(), colList.end());
	vector<Group> groupList = this->nameSpec();
	return ExtractSpec(regexList, vector<Atom>(), groupList);
}

vector<Token> Parser::column() {
	vector<Token> tokenList;
	//cout << "In column" << endl;
	tokenList.push_back(look);
	this->term(AQLToken::ID);
	this->term(AQLToken::DOT);
	tokenList.push_back(look);
	this->term(AQLToken::ID);
	return tokenList;
}

vector<Group> Parser::nameSpec() {
	//cout << "In nameSpec" << endl;
	if (this->match(AQLToken::AS)) {
		move();
		string id = this->peek();
		this->term(AQLToken::ID);
		Group group("0", id);
		vector<Group> groupList;
		groupList.push_back(group);
		return groupList;
	}
	else if (this->match(AQLToken::RETURN)) {
		move();
		return this->groupSpec();
	}
	else {
		this->syntaxError("Expected \"as\" or \"return\"");
	}
}

vector<Group> Parser::groupSpec() {
	//cout << "In groupSpec" << endl;
	vector<Group> groupList;
	groupList.push_back(this->singleGroup());
	while (this->match(AQLToken::AND)) {
		this->move();
		groupList.push_back(this->singleGroup());
	}
	return groupList;
}

Group Parser::singleGroup() {
	//cout << "In singleGroup" << endl;
	this->term(AQLToken::GROUP);
	string numStr = this->peek();
	this->term(AQLToken::NUM);
	int num = atoi(numStr.c_str());
	this->term(AQLToken::AS);
	string id = this->peek();
	this->term(AQLToken::ID);
	return Group(numStr, id);
}

ExtractSpec Parser::patternSpec() {
	//cout << "In patternSpec" << endl;
	vector<Token> tokenList;
	tokenList.push_back(look);
	this->term(AQLToken::PATTERN);
	this->patternExpr();
	vector<Group> groupList = this->nameSpec();
	return ExtractSpec(tokenList, AtomList, groupList);
}

void Parser::patternExpr() {
	//cout << "In patternExpr" << endl;
	while (!this->match(AQLToken::RIGHT_PARENTHESIS) && !this->match(AQLToken::AS) && !this->match(AQLToken::RETURN)) {
		this->patternPkg();
	}
}

void Parser::patternPkg() {
	//cout << "In patternPkg" << endl;
	if (this->match(AQLToken::LEFT_PARENTHESIS)) {
		this->patternGroup();
	}
	else if (this->match(AQLToken::LEFT_ANGLE_BACKET) || this->match(AQLToken::REG)) {
		this->atom();
		Atom& backAtom = AtomList.back();
		if (this->match(AQLToken::LEFT_BRACE)) {
			this->move();
			string leftNum = this->peek();
			this->term(AQLToken::NUM);
			this->term(AQLToken::COMMA);
			string rightNum = this->peek();
			this->term(AQLToken::NUM);
			this->term(AQLToken::RIGHT_BRACE);
			backAtom.minTime = atoi(leftNum.c_str());
			backAtom.maxTime = atoi(rightNum.c_str());
		}
	}
	else if (!this->match(AQLToken::RIGHT_PARENTHESIS)) {
		this->syntaxError("Expected \"(\" or \"<\"");
	}
}

void Parser::atom() {
	//cout << "In atom" << endl;
	vector<Token> tokenList;
	if (this->match(AQLToken::LEFT_ANGLE_BACKET)) {
		move();
		if (this->match(AQLToken::TOKEN)) {
			tokenList.push_back(this->look);
			this->move();
		}
		else if (this->match(AQLToken::ID)) {
			tokenList = this->column();
		}
		else {
			this->syntaxError("Expected \"Token\" or ID");
		}
		this->term(AQLToken::RIGHT_ANGLE_BACKET);
	}
	else if (this->match(AQLToken::REG)) {
		tokenList.push_back(this->look);
		this->move();
	}
	else {
		this->syntaxError("Expected \"<\" or regex");
	}

	AtomList.push_back(Atom(tokenList));

	// 将未闭合分组号插入到新的atom的分组集合中, 因为这说明它是属于这些分组的!
	AtomList.back().groups.insert(UnclosedGroup.begin(), UnclosedGroup.end());
}

void Parser::patternGroup() {
	//cout << "In patternGroup" << endl;
	//Atom _atom;
	//_atom.groups.insert(GroupNum);
	UnclosedGroup.insert(GroupNum); // 将分组号插入未闭合分组集合中
	int oldGroupNum = GroupNum;
	GroupNum++;
	//AtomList.push_back(_atom);

	this->term(AQLToken::LEFT_PARENTHESIS);
	this->patternExpr();
	this->term(AQLToken::RIGHT_PARENTHESIS);
	UnclosedGroup.erase(oldGroupNum);  // 分组闭合, 将分组号从未闭合分组集合中去除
}

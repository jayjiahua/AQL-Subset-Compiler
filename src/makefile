AQL_Subset: parser.o tokenizer.o view.o regex.o lexer.o token.o
	g++ -o ../bin/AQL_Subset main.cpp ../bin/parser.o ../bin/tokenizer.o \
	../bin/view.o ../bin/regex.o ../bin/lexer.o ../bin/token.o

parser.o: parser.h parser.cpp exception.h
	g++ -c parser.cpp -o ../bin/parser.o

tokenizer.o: tokenizer.cpp tokenizer.h exception.h
	g++ -c tokenizer.cpp -o ../bin/tokenizer.o

view.o: view.h view.cpp
	g++ -c view.cpp -o ../bin/view.o

regex.o: regex.cpp regex.h
	g++ -c regex.cpp -o ../bin/regex.o

lexer.o: lexer.cpp lexer.h exception.h
	g++ -c lexer.cpp -o ../bin/lexer.o

token.o: token.cpp token.h
	g++ -c token.cpp -o ../bin/token.o

clean:
	rm ../bin/*

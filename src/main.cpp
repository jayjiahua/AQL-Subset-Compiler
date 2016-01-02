#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

#include "lexer.h"
#include "parser.h"
#include "tokenizer.h"
#include "exception.h"


using std::cout;
using std::endl;
using std::string;
using std::vector;

/* 获得指定路径的文件列表 */
vector<string> getFileList(string dir) {
    vector<string> fileList;
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error: Can not open " << dir << endl;
        return fileList;
    }
    while ((dirp = readdir(dp)) != NULL) {
        fileList.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return fileList;
}

int main(int argc, char const *argv[]) {
    if (argc == 0) {
      /* 参数个数为0时扫描整个目录 */
      string datasetDir = "../dataset";

      vector<string> fileList = getFileList(datasetDir);

      for (size_t i = 0 ; i < fileList.size() ; i++) {
        // 获取dataset目录下的aql文件

          string fileName = fileList[i];
          string datasetName;
          size_t pos = fileName.find(".aql");
          if (pos == string::npos) {
              continue;
          }
          datasetName = fileName.substr(0, pos);

          vector<string> datasetFileList = getFileList(datasetDir + "/" + datasetName);

          // 进入相应的目录, 将里面所有的".txt"文本应用aql进行处理
          for (size_t j = 0 ; j < datasetFileList.size() ; j++) {
              string datasetFileName = datasetFileList[j];
              size_t pos1 = datasetFileName.find(".txt");
              //cout << datasetFileName << endl;
              if (pos1 == string::npos) {
                  continue;
              }
              datasetFileName = datasetFileName.substr(0, pos1);

              std::streambuf* coutBuf = cout.rdbuf();
              std::ofstream of(string(datasetDir + "/" + datasetName + "/" + datasetFileName + ".output").c_str());
              std::streambuf* fileBuf = of.rdbuf();

              try {
                  cout << "Processing \"" << datasetFileName << ".txt\" ...";
                  /* 代码中的输出全为cout, 输出到指定文件是通过将cout输出重定向到文件实现的. */
                  cout.rdbuf(fileBuf); // 重定向

                  // 构造词法分析器
                  Lexer lexer(string(datasetDir + "/" + fileName).c_str());

                  // 构造文本分词器
                  Tokenizer tokenizer(string(datasetDir + "/" + datasetName + "/" + datasetFileName + ".txt").c_str());

                  // 传入词法分析器和文本分词器来构造语法分析器
                  Parser parser(lexer, tokenizer);

                  cout << "Processing " << datasetFileName << ".txt" << endl;

                  // 开始语法分析
                  parser.program();

                  cout.rdbuf(coutBuf);
                  cout << " Success!" << endl;
                  of.flush();
                  of.close();  // 关闭文件
              } catch (LexicalErrorException) {
                  cout.rdbuf(coutBuf);
                  cout << " Lexical Error!" << endl;
                  of.flush();
                  of.close();
              } catch (SyntaxErrorException) {
                  cout.rdbuf(coutBuf);
                  cout << " Syntax Error!" << endl;
                  of.flush();
                  of.close();
              } catch (SemanticErrorException) {
                  cout.rdbuf(coutBuf);
                  cout << " Semantic Error!" << endl;
                  of.flush();
                  of.close();
              } catch (FileOpenException) {
                  cout.rdbuf(coutBuf);
                  cout << " File Open Error!" << endl;
                  of.flush();
                  of.close();
              }
          }
      }
      cout << "All work is done!" << endl;
    } else if (argc == 3) {
      /* 参数模式 */
      struct stat st;
      stat(argv[2], &st);
      // 判断是否目录
      if (S_ISDIR(st.st_mode)) {
          vector<string> datasetFileList = getFileList(argv[2]);
          // 进入相应的目录, 将里面所有的".txt"文本应用aql进行处理
          for (size_t j = 0 ; j < datasetFileList.size() ; j++) {
              string datasetFileName = datasetFileList[j];
              size_t pos1 = datasetFileName.find(".txt");
              //cout << datasetFileName << endl;
              if (pos1 == string::npos) {
                  continue;
              }
              try {
                  // 构造词法分析器
                  Lexer lexer(argv[1]);

                  // 构造文本分词器
                  Tokenizer tokenizer((string(argv[2]) + "/" + datasetFileName).c_str());

                  // 传入词法分析器和文本分词器来构造语法分析器
                  Parser parser(lexer, tokenizer);

                  cout << "Processing " << datasetFileName << endl;

                  // 开始语法分析
                  parser.program();

              } catch (LexicalErrorException) {

              } catch (SyntaxErrorException) {

              } catch (SemanticErrorException) {

              } catch (FileOpenException) {

              }
          }
      } else {
        try {
            // 构造词法分析器
            Lexer lexer(argv[1]);
            // 构造文本分词器
            Tokenizer tokenizer(argv[2]);
            // 传入词法分析器和文本分词器来构造语法分析器
            Parser parser(lexer, tokenizer);
            cout << "Processing " << argv[2] << endl;
            // 开始语法分析
            parser.program();
        } catch (LexicalErrorException) {
            //cout << " Lexical Error!" << endl;
        } catch (SyntaxErrorException) {
            //cout << " Syntax Error!" << endl;
        } catch (SemanticErrorException) {
            //cout << " Semantic Error!" << endl;
        } catch (FileOpenException) {
            //cout << " File Open Error!" << endl;
        }
      }
    } else {
      cout << "The number of arguments must be 0 or 2!" << endl;
    }
    return 0;
}

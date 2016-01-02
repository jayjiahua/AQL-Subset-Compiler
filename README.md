# AQL Subset Compiler

## 介绍
1. 使用了递归下降的语法分析
2. 不支持括号紧挨<Token>的分组的捕获
3. 能够识别出aql语句的词法错误, 语法错误和语义错误, 并给予指出.
4. 算法的具体实现还请查看源代码, 在关键的地方均有详细注释

## 实验环境
  Deepin 2014 (Linux)

## 代码结构
```
 src
  ├── exception.h    // 异常处理
  ├── lexer.cpp
  ├── lexer.h        // 词法分析器
  ├── main.cpp       // 程序入口
  ├── makefile
  ├── parser.cpp
  ├── parser.h       // 语法分析器
  ├── regex.cpp
  ├── regex.h        // 正则引擎
  ├── token.cpp
  ├── token.h        // AQL的Token
  ├── tokenizer.cpp
  ├── tokenizer.h    // 文本分词器
  ├── view.cpp
  └── view.h         // aql的View
```

## 运行方法
1. 打开终端, 进入src文件夹, 然后
    make
  对源代码进行编译, 编译后的可执行文件在bin目录中
2. 进入bin目录, AQL_Subset为可执行文件

### 三种运行方式
    1. 默认模式(无参数):
        `./AQL_Subset`

    2.单文件模式(两个参数, 第一个为aql, 第二个为文本文件):
        `./AQL_Subset ../dataset/Image.aql ../dataset/Image/image1.txt`

    3. 文件夹模式(两个参数, 第一个为aql, 第二个为文本文件所在的目录):
        `./AQL_Subset ../dataset/Image.aql ../dataset/Image`

## 执行过程
### 默认模式
1. 程序会扫描dataset文件夹中的.aql文件
2. 对于每个aql文件, 取其后缀名之前的文件名, 尝试打开同名的文件夹, 例如NBA.aql文件对应的文本应该在名为NBA的文件夹内
3. 打开同名的文件夹后, 程序将扫描该文件夹所有的.txt文件, 并尝试使用这个.aql文件去分析该文件夹内所有的.txt文件
4. 每处理完一个.txt文件, 程序将会输出结果到与.txt同级目录下的同名.output文件中, 例如分析了NBA.txt后, 生成的结果文件应为NBA.output

### 单文件模式
  处理单个文件,输出到终端

### 文件夹模式
  扫描最后一个参数的目录下的所有.txt文件, 然后将结果输出到终端

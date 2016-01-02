/*
  regex.h
  由于没有.h文件regex.cpp会报错,因此加了一个头文件,但是没有修改regex.cpp的内容
*/

#ifndef REGEX_H
#define REGEX_H
#include <iostream>
#include <vector>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

vector< vector<int> > findall(const char *regex, const char *content);

#endif

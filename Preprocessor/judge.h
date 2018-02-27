#pragma once

#include <string>

bool isSpace(char c);
bool isNumber(char c);
bool isLetter(char c);
bool isIdStart(char c);
bool isReal(char c);
bool isId(char c);
bool isPrimary(char c);
int isQuotes(char c);
bool isDiv(char c);
bool isKeyword(std::string &w);
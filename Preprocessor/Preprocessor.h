#pragma once

#include <string>
#include <vector>

class Macro;

class Preprocessor
{
public:
	Preprocessor();
	~Preprocessor();
	std::string getIfDef(std::string & code, int &index,bool def);
	std::string getIf(std::string & code, int &index);
	std::string parse(std::string &fileName,int& index,bool use=true);
	std::string doFile(const std::string &fileName);
	std::string doIncludeFile(std::string &fileName);
	std::string getId(std::string & code, int& index);
	std::string getFile(std::string &fileName);
	std::string getLibFile(std::string &fileName);
	std::string getAnyFile(std::string &fileName);
	std::string getCurrentDir();
	std::string getLibDir(int index);
	std::string readLine(std::string &code, int &index);
	std::string getIntMacro(std::string &macro);
	std::string getNumber(std::string &code, int &index);
	std::string getOperator(std::string &code, int &index);
	std::string parseMarcroFun(const std::string &name,std::vector<std::string> &args,int level);
	std::vector<std::string> getArg(std::string & code, int & index);
	void parseElse(std::string & code, int& index, std::string &result);
	void getMacro(std::string &code, int &index);
	void clear(std::string & code, int& index);
	void requireChar(std::string &code,int &index,char c);
	void addError(std::string &info);
	void endCmd(std::string &code, int &index);
	bool parseExpr(std::string &code);
	bool isCmd(std::string &code, int index);
	bool defined(const std::string &macro);
	bool parseCondition(std::string &code, int &index);
protected:
	struct Data;
	Data *data;
};


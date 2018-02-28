#include <fstream>
#include <vector>
#include <map>

#include "judge.h"
#include "Preprocessor.h"

#define DEBUG

struct Macro
{
	bool fun;
	std::vector<std::string> args;
	std::string data;
	std::string name;
};

struct Preprocessor::Data
{
	Data():isIf(0),currentIndex(nullptr),currentCode(nullptr){}
	int *currentIndex;
	std::string *currentCode;
	int isIf;
	std::map<std::string, Macro> macros;
	std::vector<std::string> fileStack;
};

std::string Preprocessor::getCurrentDir()
{

#ifdef DEBUG
	return "E:\Visual Studio 2015\MyProjects\Preprocessor\Preprocessor";
#endif 

}

std::string Preprocessor::getLibDir(int index)
{

#ifdef DEBUG
	if (index == 0)
	{
		return "include/";
	}
	return "";
#endif 

}

bool Preprocessor::parseExpr(std::string & code)
{

#ifdef DEBUG
	if (code == "0")
	{
		return false;
	}
	return true;
#endif 

}

std::string Preprocessor::getIntMacro(std::string & macro)
{
	auto it = data->macros.find(macro);
	if (it == data->macros.end()||it->second.fun)
	{
		return "";
	}

	std::string &s = it->second.data;

	for (int i = 0; i < s.size(); ++i)
	{
		if (!isNumber(s[i]))
		{
			return "";
		}
	}

	return s;
}

void Preprocessor::addError(std::string & info)
{
	int index = *data->currentIndex;
	std::string &code = *data->currentCode;
	int line = 1;

	for (int i = 0; i <index; ++i)
	{
		if (code[i] == '\n')
		{
			++line;
		}
	}

	throw data->fileStack[data->fileStack.size()-1]+"第"+std::to_string(line)+"行:"+info;
}

void clearSpace(std::string & code, int & index)
{
	while (index < code.size() && isSpace(code[index]) && code[index] != '\n')
	{
		++index;
	}
}

std::vector<std::string> Preprocessor::getArg(std::string &code, int & index)
{
	int q1 = 0;
	int q2 = 0;
	int q3 = 0;
	std::vector<std::string> result;
	std::string temp;

	while (index < code.size())
	{

		if (code[index] == '(')
		{
			++q1;
		}
		else if (code[index] == '[')
		{
			++q2;
		}
		else if (code[index] == '{')
		{
			++q3;
		}
		else if (code[index] == ')')
		{	
			if (q1 == 0)
			{
				result.push_back(temp);
				return result;
			}

			if (q1)
			{
				--q1;
			}
			else
			{
				addError(std::string("不匹配的括号数"));
			}
		}
		else if (code[index] == ']')
		{
			if (q2)
			{
				--q2;
			}
			else
			{
				addError(std::string("不匹配的括号数"));
			}
		}
		else if (code[index] == '}')
		{
			if (q3)
			{
				--q3;
			}
			else
			{
				addError(std::string("不匹配的括号数"));
			}
		}
		else if (code[index] == '\"')
		{
			temp.push_back('\"');
			++index;

			while (index < code.size())
			{
				if (code[index] == '\"'&&code[index - 1] != '\\')
				{
					temp.push_back('\"');
					++index;
					break;
				}
				temp.push_back(code[index]);
				++index;
			}

			if (temp[temp.size() - 1] != '\"')
			{
				addError(std::string("应输入\""));
			}

			continue;
		}
		else if (code[index] == '\'')
		{
			temp.push_back('\'');
			++index;

			while (index < code.size())
			{
				if (code[index] == '\''&&code[index - 1] != '\\')
				{
					temp.push_back('\'');
					++index;
					break;
				}
				temp.push_back(code[index]);
				++index;
			}

			if (temp[temp.size() - 1] != '\'')
			{
				addError(std::string("应输入\'"));
			}

		}
		else if (code[index] == ',')
		{
			if (!q1 && !q2 && !q3)
			{
				++index;
				result.push_back(temp);
				temp.clear();
				continue;
			}
		}

		temp.push_back(code[index]);

		++index;
	}

	addError(std::string("应输入参数"));
}

void Preprocessor::parseElse(std::string & code, int & index,std::string &result)
{
	parse(code, index);

	bool useelif = false;

	if (index < code.size())
	{
		std::string cmd;

		while (index < code.size())
		{
			cmd = getId(code, index);
			if (cmd == "else")
			{
				endCmd(code, index);
				result += parse(code, index);
				cmd = getId(code, index);
				break;
			}
			else if (cmd == "elif")
			{
				if (parseCondition(code, index) && !useelif)
				{
					result += parse(code, index);
					useelif = true;
				}
				else
				{
					parse(code, index,false);
				}
			}
			else if (cmd == "endif")
			{
				break;
			}
		}

		if (cmd != "endif")
		{
			addError(std::string("缺少#endif"));
		}

		endCmd(code, index);
	}
	else
	{
		addError(std::string("缺少#endif"));
	}
}

Preprocessor::Preprocessor()
{
	data = new Data;
}

Preprocessor::~Preprocessor()
{
	delete data;
}

bool Preprocessor::defined(const std::string & macro)
{
	return data->macros.find(macro) != data->macros.end();
}

std::string Preprocessor::parse(std::string & code,int& index,bool use)
{
	std::string result;
	bool idok = true;
	int *tempIndex = data->currentIndex;
	std::string *tempCode = data->currentCode;

	data->currentIndex = &index;
	data->currentCode = &code;

	while (index<code.size())
	{
		if (isNumber(code[index]))
		{
			idok = false;
		}
		else if (isDiv(code[index]))
		{
			idok = true;
		}

		/*是否是预编译指令*/
		if (code[index] == '#')
		{
			if (!isCmd(code, index))
			{
				addError(std::string("预编译指令必须单独占据一行"));
			}

			++index;
			std::string cmd = getId(code, index);

			if (cmd == "include")
			{
				clearSpace(code, index);

				if (code[index] != '<'&&code[index] != '\"')
				{
					addError(std::string("应输入文件名"));
				}

				std::string file;

				while (!isSpace(code[index]))
				{
					file.push_back(code[index]);
					++index;
				}
				
				endCmd(code,index);
				if (use)
				{
					result += doIncludeFile(file);
				}
			}
			else if (cmd == "define")
			{
				getMacro(code, index);
			}
			else if (cmd == "undef")
			{
				clearSpace(code, index);
				std::string id = getId(code,index);
				endCmd(code, index);
				auto it = data->macros.find(id);
				if (it != data->macros.end())
				{
					data->macros.erase(it);
				}
			}
			else if (cmd == "ifdef")
			{
				++data->isIf;
				result += getIfDef(code,index,true);
				--data->isIf;
			}
			else if (cmd == "ifndef")
			{
				++data->isIf;
				result += getIfDef(code, index, false);
				--data->isIf;
			}
			else if (cmd == "if")
			{
				++data->isIf;
				result += getIf(code, index);
				--data->isIf;
			}
			else if (cmd == "else"|| cmd == "elif" || cmd == "endif")
			{
				if (data->isIf)
				{
					index -= cmd.size();
					return result;
				}
				else
				{
					addError(std::string("缺少该指令的if"));
				}
				
			}
			else
			{
				addError(std::string("无法识别的预编译指令"));
			}

			if (index<code.size() && code[index] == '\n')
			{
				++index;
			}

			continue;
		}

		/*如果是一个标识符，则检查是不是宏，是则替换*/
		if (isIdStart(code[index])&&idok)
		{
			std::string id = getId(code, index);
			auto it = data->macros.find(id);
			if (it == data->macros.end())
			{
				result += id;
				continue;
			}

			if (!it->second.fun)
			{
				result += it->second.data;
				continue;
			}

			clearSpace(code, index);
			requireChar(code, index, '(');

			std::vector<std::string> macroArg = std::move(getArg(code, index));

			if (macroArg[0].empty())
			{
				addError(std::string("应输入参数"));
			}

			requireChar(code, index, ')');

			result+=parseMarcroFun(id, macroArg, 0);
		}

		/*字符串*/
		if (code[index] == '\"')
		{
			result.push_back(code[index]);
			++index;
			
			while (code[index] != '\"'&&index < code.size())
			{
				result.push_back(code[index]);
				++index;
			}

			if (index < code.size())
			{
				result.push_back(code[index]);
				++index;
			}
			else
			{
				addError(std::string("缺少右引号"));
			}

			continue;
		}

		/*单引号*/
		if (code[index] == '\'')
		{
			result.push_back(code[index]);
			++index;

			while (code[index] != '\''&&index < code.size())
			{
				result.push_back(code[index]);
				++index;
			}

			if (index < code.size())
			{
				result.push_back(code[index]);
				++index;
			}
			else
			{
				addError(std::string("缺少右引号"));
			}

			continue;
		}

		/*别的字符，直接装入result*/
		result.push_back(code[index]);
		++index;
	}

	data->currentIndex = tempIndex;
	data->currentCode = tempCode;

	return result;
}

std::string Preprocessor::getId(std::string &code, int& index)
{
	if (!isIdStart(code[index]))
	{
		addError(std::string("应输入标识符"));
	}

	std::string result;

	while (isId(code[index])&&index<code.size())
	{
		result.push_back(code[index]);
		++index;
	}

	return result;
}

std::string Preprocessor::getFile(std::string & fileName)
{
	char t1 = fileName[0];
	char t2 = fileName[fileName.size() - 1];

	if ((t1=='\"'&&t2!='\"')||(t1=='<'&&t2!='>'))
	{
		addError(std::string("应输入文件名"));
	}

	std::string file = fileName.substr(1, fileName.size() - 2);

	if (t1 == '<')
	{
		return getLibFile(file);
	}

	return getAnyFile(file);
}

std::string Preprocessor::getLibFile(std::string & fileName)
{
	for (int i = 0;; ++i)
	{
		std::string dir=getLibDir(i);
		if (dir.empty())
		{
			break;
		}
		dir += fileName;

		for (int i = 0; i < data->fileStack.size(); ++i)
		{
			if (dir == data->fileStack[i])
			{
				addError("不能重复包含文件" + fileName);
			}
		}

		std::ifstream file(dir);
		if (!file)
		{
			continue;
		}

		data->fileStack.push_back(dir);

		return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}

	addError("无法打开文件" + fileName);

	return std::string();
}

std::string Preprocessor::getAnyFile(std::string & fileName)
{
	std::string dir = getCurrentDir();
	dir += fileName;

	for (int i = 0; i < data->fileStack.size(); ++i)
	{
		if (dir == data->fileStack[i])
		{
			addError("不能重复包含文件" + fileName);
		}
	}

	std::ifstream file(dir);
	if (!file)
	{
		return getLibFile(fileName);
	}

	data->fileStack.push_back(dir);

	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string Preprocessor::doFile(const std::string & fileName)
{
	data->macros.clear();
	data->isIf = 0;
	data->fileStack.clear();
	std::ifstream file(fileName);

	if (!file)
	{
		addError(std::string("无法打开文件") + fileName);
	}

	std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	int index = 0;

	data->fileStack.push_back(fileName);
	return parse(code,index);
}

std::string Preprocessor::doIncludeFile(std::string & fileName)
{
	std::string code = getFile(fileName);
	int index = 0;
	std::string result=parse(code, index);
	data->fileStack.pop_back();
	return result;
}

std::string Preprocessor::getIfDef(std::string & code, int& index,bool def)
{
	clearSpace(code, index);

	std::string macro = getId(code,index);
	std::string result;

	endCmd(code, index);

	if (defined(macro)==def)
	{
		result = parse(code, index);

		if (index < code.size())
		{
			std::string cmd;

			while (index < code.size())
			{
				cmd = getId(code, index);
				if (cmd == "else")
				{
					endCmd(code, index);
					parse(code,index,false);
					cmd = getId(code, index);
					break;
				}
				else if (cmd == "elif")
				{
					parseCondition(code, index);
					parse(code,index,false);
				}
				else if (cmd == "endif")
				{
					break;
				}
			}

			if (cmd != "endif")
			{
				addError(std::string("缺少#endif"));
			}

			endCmd(code, index);
		}
		else
		{
			addError(std::string("缺少#endif"));
		}

	}
	else
	{
		parseElse(code, index, result);
	}

	return result;
}

void Preprocessor::requireChar(std::string & code, int & index, char c)
{
	if (code[index] != c)
	{
		addError(std::string("缺少") + c);
	}

	++index;
}

bool Preprocessor::isCmd(std::string & code, int index)
{
	--index;

	while (index >= 0)
	{
		if (!isSpace(code[index]))
		{
			return false;
		}
		if (code[index] == '\n')
		{
			return true;
		}
		--index;
	}

	return true;
}

void Preprocessor::endCmd(std::string & code, int & index)
{
	clearSpace(code, index);
	if (index < code.size())
	{
		requireChar(code, index, '\n');
	}
}

std::string Preprocessor::readLine(std::string & code, int & index)
{
	std::string result;

	while (index < code.size() && code[index] != '\n')
	{
		result.push_back(code[index]);
		++index;
	}

	if (code[index] == '\n')
	{
		result.push_back(code[index]);
		++index;
	}

	return result;
}

std::string Preprocessor::getNumber(std::string & code, int & index)
{
	std::string result;

	while (index < code.size())
	{
		if (isNumber(code[index]))
		{
			result += code[index];
			++index;
		}
		else if (code[index]=='.')
		{
			addError(std::string("只能使用整数类型"));
		}
		else
		{
			break;
		}
	}

	return result;
}

std::string Preprocessor::getOperator(std::string & code, int & index)
{
	std::string result;

	while (index < code.size())
	{
		if (isId(code[index])||isSpace(code[index]))
		{
			break;
		}

		result += code[index];
		++index;
	}

	return result;
}

std::string Preprocessor::parseMarcroFun(const std::string & name, std::vector<std::string>& args, int level)
{
	if (level >= 1024)
	{
		addError(std::string("宏的嵌套/递归层数太多"));
	}

	auto it = data->macros.find(name);
	Macro &m = it->second;
	std::string code = m.data,result;

	if (args.size() != m.args.size())
	{
		addError(std::string("宏参数数量不正确"));
	}

	bool flag = false;
	for (int i = 0; i < code.size();)//第一步，将所有的#替换成字符串
	{
		if (flag)
		{
			clearSpace(code, i);
			std::string arg=getId(code, i);

			bool find=false;
			int j;
			for (j = 0; j < m.args.size(); ++j)
			{
				if (m.args[j] == arg)
				{
					find = true;
					break;
				}
			}

			if (!find)
			{
				addError(std::string("应输入宏参数名"));
			}

			arg = args[j];

			result.push_back('\"');

			if (arg[0] == '"')
			{
				result.push_back('\\');
			}

			result.push_back(arg[0]);

			for (j = 1; j < arg.size(); ++j)
			{
				if (result[j] == '"'&&result[j - 1] != '\\')
				{
					result.push_back('\\');
					result.push_back('\"');
					continue;
				}
				result.push_back(result[j]);
			}

			result.push_back('\"');

			flag = false;
			continue;
		}

		if (code[i] == '#')
		{
			if (i + 1 < code.size() && code[i + 1] == '#')
			{
				i+=2;
				result.push_back('#');
				result.push_back('#');
				continue;
			}
			++i;
			flag = true;
			continue;
		}

		result.push_back(code[i]);
		++i;
	}

	if (flag)
	{
		addError(std::string("应输入宏参数"));
	}

	code = result;
	result.clear();

	bool idok = true;
	for (int i = 0; i < code.size();)//第二步，替换所有参数
	{
		if (isIdStart(code[i]))
		{
			if (idok)
			{
				std::string id = getId(code, i);

				bool find = false;
				int j;
				for (j = 0; j < m.args.size(); ++j)
				{
					if (m.args[j] == id)
					{
						find = true;
						break;
					}
				}

				if (find == true)
				{
					result += args[j];
				}
				else
				{
					result += id;
				}

			}
			else
			{
				result.push_back(code[i]);
				++i;
			}
			continue;
		}

		if (isNumber(code[i]))
		{
			idok = false;
		}
		else if (isDiv(code[i]))
		{
			idok = true;
		}

		result.push_back(code[i]);
		++i;
	}

	code = result;
	result.clear();

	for (int i = 0; i < code.size();++i)	//第三步，替换所有##
	{
		if (code[i] == '#')
		{
			continue;
		}
		result.push_back(code[i]);
	}

	code = result;
	result.clear();
	
	idok = true;
	for (int i = 0; i < code.size();)//第四步，递归处理宏定义
	{
		if (isIdStart(code[i]))
		{
			if (idok)
			{
				std::string id = getId(code, i);
				auto it = data->macros.find(id);

				if (it == data->macros.end())
				{
					result += id;
					continue;
				}

				if (!it->second.fun)
				{
					result += it->second.data;
					continue;
				}

				clearSpace(code, i);
				requireChar(code, i, '(');
				
				std::vector<std::string> macroArg=std::move(getArg(code,i));
				
				if (macroArg[0].empty())
				{
					addError(std::string("应输入参数"));
				}

				requireChar(code, i, ')');

				result+=parseMarcroFun(id, macroArg, level + 1);
			}
			else
			{
				result.push_back(code[i]);
				++i;
			}
			continue;
		}

		if (isNumber(code[i]))
		{
			idok = false;
		}
		else if (isDiv(code[i]))
		{
			idok = true;
		}

		result.push_back(code[i]);
		++i;
	}

	return result;
}

void Preprocessor::getMacro(std::string & code, int & index)
{
	clearSpace(code, index);
	std::string macro=getId(code, index);
	
	if (data->macros.find(macro) != data->macros.end())
	{
		addError(std::string("宏重定义"));
	} 

	if (code[index] != '(')//简单宏替换
	{
		Macro m;
		m.fun = false;
		clearSpace(code, index);
		while (index < code.size())
		{
			if (code[index] == '\n')
			{
				break;
			}

			if (code[index] == '\\')
			{
				++index;
				if (code[index] != '\n')
				{
					addError(std::string("无法识别的标记"));
				}
			}

			m.data.push_back(code[index]);
			++index;
		}

		if (!m.data.empty())
		{
			while (isSpace(m.data[m.data.size() - 1]))
			{
				m.data.pop_back();
			}
		}

		auto it = data->macros.find(m.data);//处理嵌套宏定义
		if (it != data->macros.end())
		{
			m.data = it->second.data;
		}

		m.name = macro;
		data->macros.insert(std::pair<std::string, Macro>(std::move(macro), m));

		return;
	}

	requireChar(code, index, '(');//带参宏替换
	std::vector<std::string> args;

	while (index < code.size())
	{
		if (!isIdStart(code[index]))
		{
			break;
		}

		std::string id = getId(code, index);
		
		if (data->macros.find(id) != data->macros.end() || id == macro)
		{
			addError(std::string("标识符重定义"));
		}

		for (int i = 0; i < args.size(); ++i)
		{
			if (id == args[i])
			{
				addError(std::string("宏参数重定义"));
			}
		}

		args.push_back(std::move(id));

		if (code[index] == ')')
		{
			break;
		}

		requireChar(code, index, ',');
	}

	if (args.empty())
	{
		addError(std::string("应输入参数"));
	}

	requireChar(code, index, ')');//宏参数解析完成

	clearSpace(code, index);//解析替换文本
	Macro m;
	m.args = std::move(args);

	std::string text;

	while (index<code.size()&&code[index] != '\n')
	{
		if (code[index] == '\\'&&index + 1 < code.size() && code[index + 1] == '\n')
		{
			text.push_back('\n');
			index += 2;
			continue;
		}

		text.push_back(code[index]);
		++index;
	}

	m.data = std::move(text);
	data->macros.insert(std::pair<std::string, Macro>(std::move(macro), m));
}

bool Preprocessor::parseCondition(std::string & code, int & index)
{
	clearSpace(code, index);

	if (isIdStart(code[index]))
	{
		std::string id = getId(code, index);

		if (id == "defined")
		{
			requireChar(code, index, '(');

			if (!isIdStart(code[index]))
			{
				addError(std::string("应输入标识符"));
			}

			std::string macro = getId(code, index);
			bool ok = false;

			if (defined(macro))
			{
				ok = true;
			}

			requireChar(code, index, ')');
			endCmd(code, index);
			return ok;
		}

		index -= id.size();
	}

	std::string expr;

	while (index < code.size())
	{
		clearSpace(code,index);

		if (isIdStart(code[index]))
		{
			std::string macro=getId(code,index);
			if (!defined(macro))
			{
				addError(std::string("使用了未定义的宏"));
			}
			
			std::string result = getIntMacro(macro);
			if (result.empty())
			{
				addError(std::string("必须使用整数类型"));
			}

			expr += result;
		}
		else if (isNumber(code[index]))
		{
			expr += getNumber(code, index);
		}
		else if(code[index]=='\n')
		{
			break;
		}
		else
		{
			expr += getOperator(code, index);
		}
	}

	endCmd(code, index);
	
	bool isBlank=true;

	for (int i = 0; i < expr.size(); ++i)
	{
		if (!isSpace(expr[i]))
		{
			isBlank = false;
			break;
		}
	}

	if (isBlank)
	{
		addError(std::string("应输入条件"));
	}

	return	parseExpr(expr);
}

std::string Preprocessor::getIf(std::string & code, int & index)
{
	std::string result;

	if (parseCondition(code,index))
	{
		result += parse(code, index);

		if (index < code.size())
		{
			std::string cmd;

			while (index < code.size())
			{
				cmd = getId(code, index);
				if (cmd == "else")
				{
					endCmd(code, index);
					parse(code, index,false);
					cmd = getId(code, index);
					break;
				}
				else if (cmd == "elif")
				{
					parseCondition(code, index);
					parse(code, index,false);
				}
				else if (cmd == "endif")
				{
					break;
				}
			}

			if (cmd != "endif")
			{
				addError(std::string("缺少#endif"));
			}

			endCmd(code, index);
		}
		else
		{
			addError(std::string("缺少#endif"));
		}

	}
	else
	{
		parseElse(code, index, result);
	}

	return result;
}

void Preprocessor::clear(std::string & code, int & index)
{
	while (index < code.size())
	{
		int i = index;
		while (isSpace(code[i]))
		{
			++i;
		}

		if (code[i] == '#')
		{
			index = i + 1;
			break;
		}

		readLine(code, index);
	}
}

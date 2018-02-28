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

	throw data->fileStack[data->fileStack.size()-1]+"��"+std::to_string(line)+"��:"+info;
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
				addError(std::string("��ƥ���������"));
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
				addError(std::string("��ƥ���������"));
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
				addError(std::string("��ƥ���������"));
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
				addError(std::string("Ӧ����\""));
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
				addError(std::string("Ӧ����\'"));
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

	addError(std::string("Ӧ�������"));
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
			addError(std::string("ȱ��#endif"));
		}

		endCmd(code, index);
	}
	else
	{
		addError(std::string("ȱ��#endif"));
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

		/*�Ƿ���Ԥ����ָ��*/
		if (code[index] == '#')
		{
			if (!isCmd(code, index))
			{
				addError(std::string("Ԥ����ָ����뵥��ռ��һ��"));
			}

			++index;
			std::string cmd = getId(code, index);

			if (cmd == "include")
			{
				clearSpace(code, index);

				if (code[index] != '<'&&code[index] != '\"')
				{
					addError(std::string("Ӧ�����ļ���"));
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
					addError(std::string("ȱ�ٸ�ָ���if"));
				}
				
			}
			else
			{
				addError(std::string("�޷�ʶ���Ԥ����ָ��"));
			}

			if (index<code.size() && code[index] == '\n')
			{
				++index;
			}

			continue;
		}

		/*�����һ����ʶ���������ǲ��Ǻ꣬�����滻*/
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
				addError(std::string("Ӧ�������"));
			}

			requireChar(code, index, ')');

			result+=parseMarcroFun(id, macroArg, 0);
		}

		/*�ַ���*/
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
				addError(std::string("ȱ��������"));
			}

			continue;
		}

		/*������*/
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
				addError(std::string("ȱ��������"));
			}

			continue;
		}

		/*����ַ���ֱ��װ��result*/
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
		addError(std::string("Ӧ�����ʶ��"));
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
		addError(std::string("Ӧ�����ļ���"));
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
				addError("�����ظ������ļ�" + fileName);
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

	addError("�޷����ļ�" + fileName);

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
			addError("�����ظ������ļ�" + fileName);
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
		addError(std::string("�޷����ļ�") + fileName);
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
				addError(std::string("ȱ��#endif"));
			}

			endCmd(code, index);
		}
		else
		{
			addError(std::string("ȱ��#endif"));
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
		addError(std::string("ȱ��") + c);
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
			addError(std::string("ֻ��ʹ����������"));
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
		addError(std::string("���Ƕ��/�ݹ����̫��"));
	}

	auto it = data->macros.find(name);
	Macro &m = it->second;
	std::string code = m.data,result;

	if (args.size() != m.args.size())
	{
		addError(std::string("�������������ȷ"));
	}

	bool flag = false;
	for (int i = 0; i < code.size();)//��һ���������е�#�滻���ַ���
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
				addError(std::string("Ӧ����������"));
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
		addError(std::string("Ӧ��������"));
	}

	code = result;
	result.clear();

	bool idok = true;
	for (int i = 0; i < code.size();)//�ڶ������滻���в���
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

	for (int i = 0; i < code.size();++i)	//���������滻����##
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
	for (int i = 0; i < code.size();)//���Ĳ����ݹ鴦��궨��
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
					addError(std::string("Ӧ�������"));
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
		addError(std::string("���ض���"));
	} 

	if (code[index] != '(')//�򵥺��滻
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
					addError(std::string("�޷�ʶ��ı��"));
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

		auto it = data->macros.find(m.data);//����Ƕ�׺궨��
		if (it != data->macros.end())
		{
			m.data = it->second.data;
		}

		m.name = macro;
		data->macros.insert(std::pair<std::string, Macro>(std::move(macro), m));

		return;
	}

	requireChar(code, index, '(');//���κ��滻
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
			addError(std::string("��ʶ���ض���"));
		}

		for (int i = 0; i < args.size(); ++i)
		{
			if (id == args[i])
			{
				addError(std::string("������ض���"));
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
		addError(std::string("Ӧ�������"));
	}

	requireChar(code, index, ')');//������������

	clearSpace(code, index);//�����滻�ı�
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
				addError(std::string("Ӧ�����ʶ��"));
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
				addError(std::string("ʹ����δ����ĺ�"));
			}
			
			std::string result = getIntMacro(macro);
			if (result.empty())
			{
				addError(std::string("����ʹ����������"));
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
		addError(std::string("Ӧ��������"));
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
				addError(std::string("ȱ��#endif"));
			}

			endCmd(code, index);
		}
		else
		{
			addError(std::string("ȱ��#endif"));
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

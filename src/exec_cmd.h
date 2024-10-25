#ifndef _EXEC_CMD_H_
#define _EXEC_CMD_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <sstream>

#include <map>

namespace CMDEXEC {

#define LEFTSTRIP 0    
#define RIGHTSTRIP 1    
#define BOTHSTRIP 2
using namespace std;




struct CmdRes
{
    std::string StdoutString;
    std::string StderrString;
    int ExitCode;
};

enum NodeType
{
	CONST_VALUE,
	FIELD,
	FUNCTION_TYPE,
};


struct Node
{
	CMDEXEC::NodeType NodeType;
	std::string NodeName; 
	std::vector<Node*> Children; 
	std::string GetValue();
};

typedef std::string FUNCTION(std::vector<Node*>);


bool Execute(const std::string &cmd, CmdRes &res);


void Stringsplit(std::string str, char split, std::vector<std::string> &res);
bool StartsWith(std::string src, std::string sub);
bool EndsWith(std::string src, std::string sub);
bool StartsWith(std::string src, char c);
bool EndsWith(std::string src, char c);
std::string Lstrip(const std::string &s);
std::string Lstrip(const std::string &s,char c);
std::string Rstrip(const std::string &s);
std::string Rstrip(const std::string &s,char c);
std::string Strip(const std::string &s);
std::string StripQuote(const std::string &src);
std::string StripBrackets(const std::string &src,char front,char back);
std::vector<std::string> Split(const std::string &s, const std::string &separator);
std::vector<std::string> Split(const std::string &s, char separator);
string replaceAll(string &str, string oldStr, string newStr);
std::string GetParamString(const std::string &src, size_t &endIndex);
std::string GetNextNodeString(const std::string &src, size_t &endIndex, char op);
std::string GetTopLevelNextNodeString(const std::string &src, size_t &endIndex);
std::string GetParamNextNodeString(const std::string &src, size_t &endIndex);

void ParseParam(std::string src, Node* parent);
void ParseNode(std::string src, Node* node);
std::string ParseSystemInfo(std::string src);


}
#endif
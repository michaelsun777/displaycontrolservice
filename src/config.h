#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <cstring>

 
using namespace std;
 
class CConfig
{
public:
	CConfig();
	~CConfig();
	void	SetFilePath(const string &filePath);
	string	GetFilePath();
	bool	GetValue(const string &section, const string &key, string &value, string &error);
	bool	ModifyValue(const string &section, const string &key, const string &value, string &error);
 
private:
	bool	OpenFile();
	bool	FindSection(const string &sectionName);
	bool	FindKey(const string &key);
	bool	OpenFileRead();
	bool	OpenFileWrite();
	bool	SetValue(const string &key, const string &value);
	void	WriteFile(vector<string> &vContent);
 
	string	m_filePath;
	fstream m_fout;
	fstream m_fin;
	string	m_content;
	string  m_value;
	string	m_error;
};
#endif
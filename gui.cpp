#pragma once
#pragma execution_character_set("utf-8")

#include <windows.h>
#include <string>
#include <stdio.h>
#include "gui.h"

using namespace std;
 int optind = 0;		// global argv index
 int opterr;
 string optarg;	// global argument pointer

int getopt(int num_str, string str_input[], char *optstring)
{
	static int flag;
	static char *next = NULL;
	if (next == NULL)
			next = new char[150];

	if (optind == 0)
		flag = 1;
	optarg.clear();
	if (flag == 1 || *next == '\0') {
		if (optind >= num_str || str_input[optind][0] != '-' || str_input[optind][1] == '\0')
		//超过命令行输入个数||索引为 - 开头||索引只有一个字符输入（\0在第二位）
		{
        	optarg.clear();//索引返回字符串初始化
			if (optind < num_str)
				optarg = str_input[optind];//在命令行输入个数内，返回index个字符串
			return EOF;//退出
			}
		string a = "--";
		if (str_input[optind].compare(a) == 0)
			//若argv[optind]为“--”跳过此字符串至下一个
		{
			optind++;
			optarg.clear();
			if (optind < num_str)
				optarg = str_input[optind];//直接返回原optind+1字符串
			return EOF;
		}
		//若条件均不满足，到下一个字符
		strcpy(next, str_input[optind].c_str());
		next++;
		optind++;
	}

	char c = *next++;//next[1]
	char *cp = strchr(optstring, c);//统计optstring中第一次出现c的位置

	if (cp == NULL || c == ':')
		return '?';//未出现，error
	cp++;
	if (*cp == ':')
	{
		if (*next != '\0')
		{
			optarg = next;
			next = NULL;
		}
		else if (optind < num_str)
		{
			optarg = str_input[optind];
			optind++;
		}
		else
			return '?';
	}
	return c;
}
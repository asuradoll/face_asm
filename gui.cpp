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
		//�����������������||����Ϊ - ��ͷ||����ֻ��һ���ַ����루\0�ڵڶ�λ��
		{
        	optarg.clear();//���������ַ�����ʼ��
			if (optind < num_str)
				optarg = str_input[optind];//����������������ڣ�����index���ַ���
			return EOF;//�˳�
			}
		string a = "--";
		if (str_input[optind].compare(a) == 0)
			//��argv[optind]Ϊ��--���������ַ�������һ��
		{
			optind++;
			optarg.clear();
			if (optind < num_str)
				optarg = str_input[optind];//ֱ�ӷ���ԭoptind+1�ַ���
			return EOF;
		}
		//�������������㣬����һ���ַ�
		strcpy(next, str_input[optind].c_str());
		next++;
		optind++;
	}

	char c = *next++;//next[1]
	char *cp = strchr(optstring, c);//ͳ��optstring�е�һ�γ���c��λ��

	if (cp == NULL || c == ':')
		return '?';//δ���֣�error
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
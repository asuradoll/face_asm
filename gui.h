#pragma once
#pragma execution_character_set("utf-8")
#ifndef  GUI_H
#define  GUI_H
#include <string>
using namespace std;
extern int optind ;		// global argv index
extern int opterr;
extern string optarg;	// global argument pointer

int getopt(int num_str, string str_input[], char *optstring);
#endif // ! GUI_H
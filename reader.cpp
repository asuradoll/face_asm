#pragma once
#pragma execution_character_set("utf-8")

#include"reader.h"

namespace modelshare {
	
	reader::reader(const char *filename) :CR(0x0a), LF(0x0d), Comment_char('#'){
		fl = fopen(filename, "rb");
	}

	reader::~reader() {
		puts("closing file");
		if (fl) fclose(fl);
	}

	bool reader::MoreNonWhiteSpaceOnLine() {

		char buf[256];
		int ch, n_gets = 0;
		bool non_white = false;

		do {
			ch = fgetc(fl);
			buf[n_gets++] = ch;
			if (ch != '\t' && ch != ' ' && ch != CR && ch != LF && ch != EOF) {
				non_white = true; break;
			}
		} while (ch != EOF && ch != CR && ch != LF);

		for (int i = 0;i<n_gets;i++)
			ungetc(buf[--n_gets], fl);

		return non_white;
	}
}

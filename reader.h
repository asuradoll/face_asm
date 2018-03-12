#pragma once
#pragma execution_character_set("utf-8")
#ifndef READER_H
#define READER_H

#include <cstdio>

namespace modelshare {

	class reader
	{
	public:
		reader(const char *filename);
		~reader();

		/// Skips whitespace and any commments preceeding the current file position.
		void Sync() { Skip_useless(); Skip_comments(); }

		bool FlValid() {
			bool valid = (fl == NULL);
			return valid;
		}

		FILE *FL() { return fl; }

		bool MoreNonWhiteSpaceOnLine();
	private:
		FILE *fl;
		const char CR; //0x0a
		const char LF; //0x0d ����س��ĸ����ַ�
		const char Comment_char;//#
		//˽�����ݳ�Ա���ɸ���ֵ
		void Skip_useless() {
			int ch;
			do {
				ch = fgetc(fl);
			} while (ch == ' ' || ch == '\t' || ch == CR || ch == LF);
			ungetc(ch, fl);//���һ��Ϊ����Ҫ���ַ������˻ص��ļ���
		}

		void Skip_restline() {
			int ch;
			do {
				ch = fgetc(fl);
			} while (ch != EOF && ch != CR && ch != LF);
			ungetc(ch, fl);//���һ��Ϊ����Ҫ���ַ������˻ص��ļ���

			Skip_useless();
		}

		void Skip_comments() {
			int ch;
			ch = getc(fl);
			if (ch == Comment_char) {
				Skip_restline();
				Skip_comments();
			}else{
				ungetc(ch, fl);
			}
		}
	};
}
#endif // !READER_H
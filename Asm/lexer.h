#ifndef __LEXER_H_
#define __LEXER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "code.h"

using namespace std;

// 词法单元
struct Token{
	int	kind;
	Token(int tag) :kind(tag){  }
	virtual string place(){
		ostringstream s;
		s << kind;
		return s.str();
	}
	virtual string code(){
		ostringstream s;
		s << kind;
		return s.str();
	}
};

struct Word :Token{
	string word;
	Word(int tag, string word) :Token(tag), word(word) {  }
	virtual string place(){
		ostringstream s;
		s << word;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

struct Type :Word{
	int width;
	static Type *Int, *Reg;
	Type(int kind, string word, int width) :Word(kind, word), width(width){  }
	virtual string place(){
		ostringstream s;
		s << word << ":" << width;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

Type* Type::Int = new Type(NUM, "int", 2);
Type* Type::Reg = new Type(REG, "int", 1);

struct Integer :Token{
	int value;
	Integer(int tag, int value) :Token(tag), value(value) {  }
	virtual string place(){
		ostringstream s;
		s << value;
		return s.str();
	}
	virtual string code(){
		return "";
	}
};

// 词法分析器
class Lexer{
	ifstream inf;
	map<string, Token*> words;
	int hexToDec(char word[20])
	{
		char *str;
		int l = strtol(word, &str, 16);
		return l;
	}
	int octToDec(char word[20])
	{
		char *str;
		int l = strtol(word, &str, 8);
		return l;
	}
public:
	int line = 1;
	Lexer(string fp){
		words["data"] = new Word(DATA, "data");
		words["stack"] = new Word(STACK, "stack");
		words["code"] = new Word(CODE, "code");
		// 数据操作
		words["load"] = new Word(LOAD, "load");
		words["store"] = new Word(STORE, "store");
		// 停机指令
		words["halt"] = new Word(HALT, "halt");
		// 算术逻辑运算
		words["sub"] = new Word(SUB, "add");
		words["add"] = new Word(ADD, "sub");
		// 栈操作指令
		words["push"] = new Word(PUSH, "push");
		words["pop"] = new Word(POP, "pop");
		// 跳转指令
		words["jmp"] = new Word(JMP, "jmp");
		words["jb"] = new Word(JB, "jb");
		words["jbe"] = new Word(JBE, "jbe");
		words["je"] = new Word(JE, "je");
		words["jge"] = new Word(JGE, "jge");
		words["jg"] = new Word(JG, "jg");
		words["jne"] = new Word(JNE, "jne");
		// 函数调用
		words["proc"] = new Word(PROC, "proc");
		words["endp"] = new Word(ENDP, "endp");
		words["call"] = new Word(CALL, "call");
		// 段寄存器
		words["ds"] = new Integer(REG, Register::DS);
		words["cs"] = new Integer(REG, Register::CS);
		words["ss"] = new Integer(REG, Register::SS);
		words["es"] = new Integer(REG, Register::ES);
		// 寄存器
		words["bp"] = new Integer(REG, Register::BP);
		words["sp"] = new Integer(REG, Register::SP);
		words["si"] = new Integer(REG, Register::SI);
		words["di"] = new Integer(REG, Register::DI);
		// MIPS指令集
		// R-type
		words["add"] = new Integer(RTYPE, 0x00000020);
		words["addu"] = new Integer(RTYPE, 0x00000021);
		words["sub"] = new Integer(RTYPE, 0x00000022);
		words["subu"] = new Integer(RTYPE, 0x00000023);
		words["and"] = new Integer(RTYPE, 0x00000024);
		words["or"] = new Integer(RTYPE, 0x00000025);
		words["xor"] = new Integer(RTYPE, 0x00000026);
		words["nor"] = new Integer(RTYPE, 0x00000027);
		words["slt"] = new Integer(RTYPE, 0x0000002A);
		words["sltu"] = new Integer(RTYPE, 0x0000002B);
		words["sll"] = new Integer(RTYPE, 0x00000000);
		words["srl"] = new Integer(RTYPE, 0x00000002);
		words["ara"] = new Integer(RTYPE, 0x00000003);
		words["add"] = new Integer(RTYPE, 0x00000004);
		words["add"] = new Integer(RTYPE, 0x00000005);
		words["srav"] = new Integer(RTYPE, 0x00000006);
		inf.open(fp, ios::in);
	}
	~Lexer(){
		inf.close();
		words.clear();
		printf("~Lexer");
	}
	Token *scan()
	{
		int i = 0;
		char ch;
		do{
			inf.read(&ch, sizeof(ch));
			if (ch == ';'){
				while (ch != '\n'){
					//printf("skip:%c\n", ch);
					inf.read(&ch, sizeof(ch));
				}
			}
			if (ch == '\n')line++;
		} while (ch == ' ' || ch == '\n' || ch == '\t');
		if (inf.eof()){
			printf("end of file\n");
			return new Token(END);
		}
		if (isalpha(ch)){
			string str;
			do{
				str.push_back(ch);
				inf.read(&ch, sizeof(ch));
			} while (isalnum(ch));  //1状态
			inf.seekg(-1, ios::cur);//回退一个字符
			if (words.find(str) == words.end()){
				return new Word(ID, str);
			}
			return words[str];
		}
		if (isdigit(ch)){
			int value = 0;
			if (ch == '0'){
				inf.read(&ch, sizeof(ch));
				if (ch == 'x' || ch == 'X'){
					inf.read(&ch, sizeof(ch));
					if (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F')){
						do{
							if (isalpha(ch)){
								value = 16 * value + ch - 'A' + 10;
							}else{
								value = 16 * value + ch - '0';
							}
							inf.read(&ch, sizeof(ch));
						} while (isdigit(ch) || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F'));
						inf.seekg(-1, ios::cur);
						return new Integer(NUM, value);
					}else{
						printf("错误的十六进制!");
					}
				}else if (ch >= '0'&&ch <= '7'){
					//八进制整数
					do{
						value = 8 * value + ch - '0';
						inf.read(&ch, sizeof(ch));
					} while (ch >= '0'&&ch <= '7');
					inf.seekg(-1, ios::cur);
					return new Integer(NUM, value);
				}else{
					//十进制整数0
					inf.seekg(-1, ios::cur);
					return new Integer(NUM, 0);
				}
			}else{
				//除0外十进制整数,5状态
				do{
					value = 10 * value + ch - '0';
					inf.read(&ch, sizeof(ch));
				} while (isdigit(ch));
				inf.seekg(-1, ios::cur);//回退一个字符
				return new Integer(NUM, value);
			}
		}
		return new Token(ch);
	}
};

#endif
#include<iostream>
#include<fstream>
#include<io.h>
#include"def.h"
using namespace std;
size_t LogHash[MAX_LOG_NUM];	//消费日志校验码hash的保存处，保存在内存
int xflogLine;	//用于消费日志校验码的内存保存
/*操作日志Write函数*/
void GenerateLog(string meg) {
	//czlogWriter.open("E:\\课设\\log.txt", ios::out | ios::app);

	czlogWriter << (char*)meg.c_str() << endl;
	//czlogWriter.flush();
	return;
}
/*消费日志Write函数*/
void xflog(string meg) {
	xflogWriter << (char*)meg.c_str() << endl;
	LogHash[++xflogLine] = MyHash(meg);
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}
/*多路归并排序结果日志的Write函数*/
void sortxflog(string meg) {
	sortxflogWriter << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}
/*快速排序结果日志Write函数*/
void sortxflog_quick_sort(string meg) {
	sortxflogWriter_quick_sort << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}
/*Windows记录最终回绕成功的消费数目的日志Write函数*/
void windowslog(string meg) {
	windowlogWriter << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}
#include<iostream>
#include<fstream>
#include<io.h>
#include"def.h"
using namespace std;
void GenerateLog(string meg) {
	//czlogWriter.open("E:\\øŒ…Ë\\log.txt", ios::out | ios::app);

	czlogWriter << (char*)meg.c_str() << endl;
	//czlogWriter.flush();
	return;
}

void xflog(string meg) {
	xflogWriter << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}

void sortxflog(string meg) {
	sortxflogWriter << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}

void sortxflog_quick_sort(string meg) {
	sortxflogWriter_quick_sort << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}

void windowslog(string meg) {
	windowlogWriter << (char*)meg.c_str() << endl;
	//xflogWriter.flush();
	//fputs((char*)meg.c_str(), xflogWriter1);
	return;
}
#include<iostream>
#include<string>
#include<fstream>
#include "def.h"
using namespace std;
fstream czlogWriter, xflogWriter, sortxflogWriter, sortxflogWriter_quick_sort, windowlogWriter;
fstream xflogReader;
//FILE *xflogWriter1;
//errno_t err;
void batchKH() {
	readKHFile();
	return;
}
void batchCZ() {
	readCZFile();
	return;
}
bool ack() {
	cout << "是否操作?" << endl;
	char flag;
	cin >> flag;
	if (flag) {
		return true;
	}
	else {
		cout << "放弃操作" << endl;
		return false;
	}
}
void CZ() {
	string meg;
	cout << "输入学号、操作和信息,格式为 学号,操作代码(,姓名,如果有的话)(,其他信息,如充值金额)" << endl;
	cout << "操作代码列表:" << endl;
	cout << "1 开户 2 销户 3 发卡 4 挂失 5 解挂 6 补卡 7 充值" << endl;
	while (cin >> meg) {
		if (meg[0] == '0') break;
		string stuid = meg.substr(0, 10);
		char type = meg[11];
		if (type == '1') {
			string name = meg.substr(13, meg.size() - 13);
			cout << stuid << ' ' << name << endl;
			CreateAct(stuid, name, "2021090109220510");
		}
		else if (type == '2') {
			showActMeg(stuid);

			if (ack()) DeleteAct(stuid, "");
		}
		else if (type == '3') {
			showActMeg(stuid);
			if (ack()) SendCardID(stuid, "");
		}
		else if (type == '4') {
			showActMeg(stuid);
			if (ack()) ReportLoss(stuid, "");
		}
		else if (type == '5') {
			showActMeg(stuid);
			if (ack()) EraseLoss(stuid, "");
		}
		else if (type == '6') {
			showActMeg(stuid);
			if (ack()) MakeupCard(stuid, "");
		}
		else if (type == '7') {
			int val = stoi(meg.substr(13, meg.size() - 13));
			showActMeg(stuid);
			cout << "是否充值" << val << "元?" << endl;
			char flag;
			cin >> flag;
			if (flag) {
				Recharge(stuid, double(val), "");
			}
			else {
				cout << "放弃充值" << endl;
			}
		}
		else {
			cout << "Valid type!" << endl;
		}
		cout << "已执行完毕，请输入下一次操作" << endl;
	}
	cout << "结束卡片操作" << endl;
	system("pause");
	return;
}
void batchCard() {
	windowlogWriter.open("E:\\classdesign\\windowslog.txt", ios::out);
	czlogWriter.open("E:\\classdesign\\czlog.txt", ios::out);
	xflogWriter.open("E:\\classdesign\\xflog.txt", ios::out);
	sortxflogWriter.open("E:\\classdesign\\xflog(sorted by multimerge).txt", ios::out);
	sortxflogWriter_quick_sort.open("E:\\classdesign\\xflog(sorted by quicksort).txt", ios::out);
	/*
	errno_t err;
	if (err = fopen_s(&xflogWriter1, "E:\\classdesign\\xflog.txt", "w+") != 0) {
		printf("The file 'E:\\classdesign\\xflog.txt' was not opened\n");
	}*/
	batchKH();
	batchCZ();
	initWindows();
	read2XF();
	batchXF();
	MultipleMerge();
	QuickSort();
	czlogWriter.close();
	xflogWriter.close();
	sortxflogWriter.close();
	sortxflogWriter_quick_sort.close();
	windowlogWriter.close();
	/*
	if (fclose(xflogWriter1)) {
		printf("The file ''E:\\classdesign\\xflog.txt' was not closed\n");
	}*/
}
void XF() {
	string meg, cardid;
	while (1) {
		cout << "充值请输入卡号+充值金额,格式为卡号,金额" << endl;
		cin >> meg;
		cardid = meg.substr(0, 7);
		double val = stod(meg.substr(8));
		sConsume(cardid, val);
		system("pause");
	}
}
int main() {
	batchCard();
	//search();
	return 0;
}
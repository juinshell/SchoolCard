#include<iostream>
#include<string>
#include<fstream>
#include "def.h"
using namespace std;
fstream czlogWriter, xflogWriter, sortxflogWriter, sortxflogWriter_quick_sort, windowlogWriter;
fstream xflogReader;
extern int xflogLine;
/*批量开户的逻辑*/
void batchKH() {
	readKHFile();
	return;
}
/*批量卡操作（不包括充值和开户）*/
void batchCZ() {
	readCZFile();
	return;
}
/*表示用户是否确定执行操作*/
bool ack() {
	cout << "是否操作?" << endl;
	int flag;
	cin >> flag;
	if (flag) {
		return true;
	}
	else {
		cout << "放弃操作" << endl;
		return false;
	}
}
/*卡片管理模块的接口*/
void CZ() {
	string meg;
	cout << "输入学号、操作和信息,格式为 操作代码(,学号,如果有的话)(,姓名,如果有的话)(,其他信息,如充值金额)" << endl;
	cout << "操作代码列表:" << endl;
	cout << "1 开户 2 销户 3 发卡 4 挂失 5 解挂 6 补卡 7 充值 8 读入并进行批量操作(不允许已有数据) 0 退出" << endl;
	while (cin >> meg) {
		if (meg[0] == '0') break;
		if (meg[0] == '8') {
			batchKH();
			batchCZ();
			cout << "批量操作完成,即将返回主界面" << endl;
			system("pause");
			break;
		}
		string stuid = meg.substr(2, 10);
		char type = meg[0];
		if (type == '1') {
			string name = meg.substr(13, meg.size() - 13);
			cout << "充值学号" << stuid << " 姓名:" << name << endl;
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
			double val = stod(meg.substr(13));
			showActMeg(stuid);
			cout << "是否充值" << val << "元?" << endl;
			int flag;
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
	system("cls");
	return;
}
/*打开所有的Writer方便日志写入*/
void openWriter() {
	windowlogWriter.open("E:\\classdesign\\windowslog.txt", ios::out);
	czlogWriter.open("E:\\classdesign\\czlog.txt", ios::out);
	xflogWriter.open("E:\\classdesign\\xflog.txt", ios::out);
	sortxflogWriter.open("E:\\classdesign\\xflog(sorted by multimerge).txt", ios::out);
	sortxflogWriter_quick_sort.open("E:\\classdesign\\xflog(sorted by quicksort).txt", ios::out);
}
/*关闭所有的Writer保证安全*/
void closeWriter() {
	czlogWriter.close();
	xflogWriter.close();
	sortxflogWriter.close();
	sortxflogWriter_quick_sort.close();
	windowlogWriter.close();
}
/*批量挂失/补卡、充值与消费的逻辑*/
void batchCard() {

	batchKH();
	batchCZ();
	initWindows();
	read2XF();
	batchXF();
	MultipleMerge();
	QuickSort();
}

int main() {
	
	Welcome();		//初始化界面
	if (menu() == 0) {		//用户判断是否进入系统
		system("cls");
		cout << "退出系统!" << endl;
		return 0;
	}
	else {
		system("cls");
		cout << "进入系统!" << endl;
		system("pause");
	}
	system("cls");
	openWriter();
	while (1) {
		cout << "************************欢迎进入校园卡管理系统!************************" << endl;
		cout << "1.进入卡片管理模块" << endl;
		cout << "2.进入食堂管理模块" << endl;
		cout << "3.进入查询统计模块" << endl;
		cout << "4.退出系统" << endl;
		int choice;
		cin >> choice;
		if (choice == 1) {
			system("cls");
			CZ();
		}
		else if (choice == 2) {
			system("cls");
			ShowCartoon();
		}
		else if (choice == 3) {
			system("cls");
			search();
		}
		else if (choice == 4) {
			cout << "退出系统!" << endl;
			return 0;
		}
		else {
			cout << "非法输入，请重试" << endl;
		}
		system("cls");
	}
	closeWriter();
	
	return 0;
}
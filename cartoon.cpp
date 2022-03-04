#include<iostream>
#include<fstream>
#include "def.h"
#include<cassert>
#include<queue>
#include<time.h>
#include<unordered_map>
#include<io.h>
#include <sstream>
using namespace std;
string curTime;
int curWindow;
vector<WindowManager> Windows(100);
double sumMoney;
double sumMoneySec;
int sumTimes;
bool flag;
int batchPreDur;
string batchPreDate;
unordered_map<string, vector<double>> CardFlag;		//检查是否超过密码限制
double daysum[60];	//单项操作统计
extern string CardID2ActMap[CARD_MAX_NUM];		//将cardid转化为对应的校园卡账户学号
extern CardAct AllAccount[STU_MAJOR_MAX_NUM][STU_MAX_NUM_IN_MAJOR];		//所有校园卡账户的数组，第一维表示专业号，第二维表示专业内的流水号
extern fstream xflogWriter, czlogWriter;		//消费记录和操作记录的Writer，用于写日志
extern vector<XFMeg*> QuickSortVector;			//按时间顺序的成功消费记录
/*把time对应的时间段转化为数组的下标*/
int change(string time) {
	if (time >= "07" && time <= "09") return 0;
	else if (time >= "11" && time <="13") return 1;
	else if (time >= "17" && time <= "19") return 2;
	else return 3;
}
/*设置CardFlag，每次消费都需要更新检查，来判断是否需要密码*/
void checkFlag(string cardid, string date, string time, double val, XFMeg* meg) {
	if (!CardFlag.count(cardid)) {
		CardFlag[cardid] = vector<double>{0, 0, 0};
		int index = change(time.substr(0, 2));
		if (index == 3) {
			cout << "时间段错误" << endl;
			system("pause");
			return;
		}
		CardFlag[cardid][index] += val;
		if (CardFlag[cardid][index] > 20) {
			meg->secretneed = true;
			return;
		}
	}
	else {
		int index = change(time.substr(0, 2));
		if (index == 3) {
			cout << "时间段错误" << endl;
			system("pause");
			return;
		}
		CardFlag[cardid][index] += val;
		if (CardFlag[cardid][index] > 20) {
			meg->secretneed = true;
			return;
		}
	}
}
/*重载minheap小根堆的运算符，用于顺序执行不同分界点的消费记录*/
struct cmp
{
	bool operator()(XFMeg *a, XFMeg *b) const
	{
		return a->xfdate + a->xftime > b->xfdate + b->xftime;
	}
};
/*用于顺序执行不同分界点的消费记录*/
priority_queue<XFMeg*, vector<XFMeg*>, cmp> minheap;
/*初始化所有窗口Manager信息*/
void initWindows() {
	for (int i = 1; i < 58; ++i) {
		Windows[i].num = i;
		Windows[i].pointer = 0;
		Windows[i].xflist.clear();
	}
}
/*从xf014.txt读取数据*/
void read2XF() {
	initWindows();
	cout << "开始读入消费文件" << endl;
	ifstream xffile;
	clock_t start, end;
	start = clock();
	xffile.open("E:\\classdesign\\xf014.txt", ios::in);
	assert(xffile.is_open());
	string xfmeg;
	int curwin = 0;
	getline(xffile, xfmeg);
	while (getline(xffile, xfmeg)) {
		if (xfmeg[0] == 'W') curwin++;
		else {
			Windows[curwin].xflist.push_back(
				XFMeg{
					xfmeg.substr(0, 7),
					xfmeg.substr(8, 8),
					xfmeg.substr(17, 8),
					stod(xfmeg.substr(26, xfmeg.size() - 27)),
					curwin,
					true,
					0,
					false
				}
			);
		}
	}
	end = clock();
	cout << "xffile read finish! cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	xffile.close();
}
/*用于设置当前手工输入的消费时间和消费窗口*/
void setConfig(string curtime, int curwindow) {
	curTime = curtime;
	curWindow = curwindow;
}
/*批量消费的单个逻辑*/
bool Consume(int window, string cardid, string date, string time, double val, XFMeg* xfmeg) {//批量消费的单个逻辑
	if (date != batchPreDate || change(time) != batchPreDur) {
		CardFlag.clear();
		batchPreDate = date;
		batchPreDur = change(time);
	}
	string serialnum = cardid.substr(1, 5);
	string uid = CardID2ActMap[stoi(serialnum)];
	if (uid.size() != 10) return false;
	int major = stoi(uid.substr(4, 2));
	int order = stoi(uid.substr(6, 3));
	CardAct* curAct = &AllAccount[major][order];
	//检查是否为有效卡
	if (!curAct->cardList.size || curAct->cardList.tail->pre->serialNum != serialnum || !curAct->cardList.tail->pre->valid) {
		//xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + "该卡号无效!", window);
		return false;
	}
	if (curAct->cardList.tail->pre->balance < val) {
		//cout << "卡余额不足" << endl;
		//xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "卡余额不足!", window);
		return false;
	}
	checkFlag(cardid, date, time, val, xfmeg);
	CardID* curCard = curAct->cardList.tail->pre;
	curAct->cardList.tail->pre->balance -= val;
	xfmeg->balance = curAct->cardList.tail->pre->balance;
	if (xfmeg->secretneed) {
		xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance) + "需要密码输入!");
		int index = change(time.substr(0, 2));
		CardFlag[cardid][index] = 0;
	}
	else {
		xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance));
	}
	return true;
}
/*手工输入消费的单个逻辑，差别主要在日志的写入上*/
bool sConsume(int window, string cardid, string date, string time, double val, XFMeg* xfmeg) {

	string serialnum = cardid.substr(1, 5);
	string uid = CardID2ActMap[stoi(serialnum)];
	if (uid.size() != 10) return false;
	int major = stoi(uid.substr(4, 2));
	int order = stoi(uid.substr(6, 3));
	CardAct* curAct = &AllAccount[major][order];
	//检查是否为有效卡
	if (!curAct->cardList.size || curAct->cardList.tail->pre->serialNum != serialnum || !curAct->cardList.tail->pre->valid) {
		cout << "该卡号无效" << endl;
		//xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + "该卡号无效!", window);
		return false;
	}
	if (curAct->cardList.tail->pre->balance < val) {
		cout << "卡余额不足" << endl;
		//xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "卡余额不足!", window);
		return false;
	}
	checkFlag(cardid, date, time, val, xfmeg);
	CardID* curCard = curAct->cardList.tail->pre;
	curAct->cardList.tail->pre->balance -= val;
	xfmeg->balance = curAct->cardList.tail->pre->balance;
	if (xfmeg->secretneed) {
		cout << "当前时间段消费超过20元,请输入6位密码" << endl;
		string secret;
		while (cin >> secret) {
			if (secret.size() == 6) {
				cout << "密码输入正确" << endl;
				break;
			}
			else {
				cout << "密码错误,请重新输入" << endl;
			}
		}
		xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance) + "需要密码输入!");
		int index = change(time.substr(0, 2));
		CardFlag[cardid][index] = 0;
	}
	else {
		xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance));
	}
	return true;
}
/*进行最多六万条的数据回绕，从开头多余的数据valid值置0*/
void roundData() {
	for (int i = 1; i < 58; ++i) {
		int sum = 0;
		int n = Windows[i].xflist.size();
		int index = n - 1;
		while (index >= 0 && sum < min(60000, n)) {
			while (index >= 0 && Windows[i].xflist[index].valid != true) index--;
			sum++;
			index--;
		}
		//cout << sum << endl;
		for (int j = 0; j <= index; ++j) {
			Windows[i].xflist[j].valid = false;
		}
	}
	int sum = 0;
	for (int i = 1; i < 58; ++i) {
		int partsum = 0;
		for (auto &it : Windows[i].xflist) {
			if (it.valid) sum++, partsum++;
		}
		//cout << "窗口" << i << "的最终保存的消费记录有" << partsum << "条" << endl;
	}
	cout << "经过数据最大值6W回绕后,最终保存的消费记录有" << sum << "条" << endl;
}
/*批量消费的主要函数，按照时间顺序执行不同分段的消费、充值和其他卡操作*/
void batchXF() {
	clock_t start, end;
	ifstream czfile;
	czfile.open("E:\\classdesign\\cz002.txt", ios::in);
	assert(czfile.is_open());
	string czmeg;
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) continue;
		else break;
	}
	//开始9.4充值
	//cout << "开始9.4充值" << endl;
	start = clock();
	string cztime = czmeg.substr(0, 16);
	string cztype = czmeg.substr(17, 4);
	string stuid = czmeg.substr(22, 10);
	double val = stoi(czmeg.substr(33));
	int major = stoi(stuid.substr(4, 2));
	int order = stoi(stuid.substr(6, 3));
	Recharge(stuid, double(val), "");
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) break;
		else {
			cztime = czmeg.substr(0, 16);
			if (cztime.substr(4, 4) != "0904") break;
			cztype = czmeg.substr(17, 4);
			stuid = czmeg.substr(22, 10);
			val = stoi(czmeg.substr(33));
			major = stoi(stuid.substr(4, 2));
			order = stoi(stuid.substr(6, 3));
			Recharge(stuid, val, cztime);
		}
	}
	//end = clock();
	//cout << "9.4充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl << "9.6挂失补卡开始:" << endl;
	//结束9.4充值，开始9.6挂失补卡操作
	//start = clock();
	cztype = czmeg.substr(17, 4);
	stuid = czmeg.substr(22, 10);
	if (cztype == "挂失") {
		ReportLoss(stuid, cztime);
	}
	else if (cztype == "补卡") {
		MakeupCard(stuid, cztime);
	}
	while (getline(czfile, czmeg)) {
		if (czmeg.length() > 33) break;
		cztime = czmeg.substr(0, 16);
		cztype = czmeg.substr(17, 4);
		stuid = czmeg.substr(22, 10);
		if (cztype == "挂失") {
			ReportLoss(stuid, cztime);
		}
		else if (cztype == "补卡") {
			MakeupCard(stuid, cztime);
		}
	}
	string cardid, xfdate, xftime;
	int window;
	//end = clock();
	//cout << "9.6补卡结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl << "开始10.14前的消费:" << endl;
	//结束9.6补卡，开始到10.13日的消费
	//search();
	//start = clock();
	string path;
	ostringstream ostr;
	for (int i = 1; i < 58; ++i) {
		if (Windows[i].xflist[Windows[i].pointer].xfdate <= "20211013")
			minheap.push(&Windows[i].xflist[Windows[i].pointer++]);
	}
	while (!minheap.empty()) {
		XFMeg* oritime = minheap.top();
		minheap.pop();
		xfdate = oritime->xfdate;
		xftime = oritime->xftime;
		val = oritime->val;
		cardid = oritime->cardid;
		window = oritime->window;
		//Windows[window].pointer++;
		if (!Consume(window, cardid, xfdate, xftime, val, oritime)) {
			Windows[window].xflist[Windows[window].pointer - 1].valid = false;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].xfdate <= "20211013") {
			minheap.push(&Windows[window].xflist[Windows[window].pointer++]);
		}
	}
	//end = clock();
	//cout << "10.14前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "10.14凌晨充值开始" << endl;
	//start = clock();
	cztime = czmeg.substr(0, 16);
	cztype = czmeg.substr(17, 4);
	stuid = czmeg.substr(22, 10);
	val = stoi(czmeg.substr(33));
	major = stoi(stuid.substr(4, 2));
	order = stoi(stuid.substr(6, 3));
	Recharge(stuid, double(val), "");
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) break;
		else {
			cztime = czmeg.substr(0, 16);
			if (cztime.substr(4, 4) != "1014") break;
			cztype = czmeg.substr(17, 4);
			stuid = czmeg.substr(22, 10);
			val = stoi(czmeg.substr(33));
			major = stoi(stuid.substr(4, 2));
			order = stoi(stuid.substr(6, 3));
			Recharge(stuid, val, cztime);
		}
	}
	//end = clock();
	//cout << "10.14充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "开始11.4前的消费:" << endl;
	//start = clock();
	for (int i = 1; i < 58; ++i) {
		if (Windows[i].xflist[Windows[i].pointer].xfdate <= "20211103")
			minheap.push(&Windows[i].xflist[Windows[i].pointer++]);
	}
	while (!minheap.empty()) {
		XFMeg* oritime = minheap.top();
		minheap.pop();
		xfdate = oritime->xfdate;
		xftime = oritime->xftime;
		val = oritime->val;
		cardid = oritime->cardid;
		window = oritime->window;
		if (!Consume(window, cardid, xfdate, xftime, val, oritime)) {
			Windows[window].xflist[Windows[window].pointer - 1].valid = false;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].xfdate <= "20211103") {
			minheap.push(&Windows[window].xflist[Windows[window].pointer++]);
		}
	}
	//end = clock();
	//cout << "11.4前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "11.4凌晨充值开始:" << endl;
	//start = clock();
	cztime = czmeg.substr(0, 16);
	cztype = czmeg.substr(17, 4);
	stuid = czmeg.substr(22, 10);
	val = stoi(czmeg.substr(33));
	major = stoi(stuid.substr(4, 2));
	order = stoi(stuid.substr(6, 3));
	Recharge(stuid, double(val), "");
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) break;
		else {
			cztime = czmeg.substr(0, 16);
			if (cztime.substr(4, 4) != "1104") break;
			cztype = czmeg.substr(17, 4);
			stuid = czmeg.substr(22, 10);
			val = stoi(czmeg.substr(33));
			major = stoi(stuid.substr(4, 2));
			order = stoi(stuid.substr(6, 3));
			Recharge(stuid, val, cztime);
		}
	}
	//end = clock();
	//cout << "11.04充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "开始11.25前的消费:" << endl;
	//start = clock();
	for (int i = 1; i < 58; ++i) {
		if (Windows[i].xflist[Windows[i].pointer].xfdate <= "20211124")
			minheap.push(&Windows[i].xflist[Windows[i].pointer++]);
	}
	while (!minheap.empty()) {
		XFMeg* oritime = minheap.top();
		minheap.pop();
		xfdate = oritime->xfdate;
		xftime = oritime->xftime;
		val = oritime->val;
		cardid = oritime->cardid;
		window = oritime->window;
		if (!Consume(window, cardid, xfdate, xftime, val, oritime)) {
			Windows[window].xflist[Windows[window].pointer - 1].valid = false;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].xfdate <= "20211124") {
			minheap.push(&Windows[window].xflist[Windows[window].pointer++]);
		}
	}
	//end = clock();
	//cout << "11.25前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "11.25凌晨充值开始:" << endl;
	//start = clock();
	cztime = czmeg.substr(0, 16);
	cztype = czmeg.substr(17, 4);
	stuid = czmeg.substr(22, 10);
	val = stoi(czmeg.substr(33));
	major = stoi(stuid.substr(4, 2));
	order = stoi(stuid.substr(6, 3));
	Recharge(stuid, double(val), "");
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) break;
		else {
			cztime = czmeg.substr(0, 16);
			if (cztime.substr(4, 4) != "1125") break;
			cztype = czmeg.substr(17, 4);
			stuid = czmeg.substr(22, 10);
			val = stoi(czmeg.substr(33));
			major = stoi(stuid.substr(4, 2));
			order = stoi(stuid.substr(6, 3));
			Recharge(stuid, val, cztime);
		}
	}
	//end = clock();
	//cout << "11.25充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "开始12.16前的消费:" << endl;
	//start = clock();
	for (int i = 1; i < 58; ++i) {
		if (Windows[i].xflist[Windows[i].pointer].xfdate <= "20211215")
			minheap.push(&Windows[i].xflist[Windows[i].pointer++]);
	}
	while (!minheap.empty()) {
		XFMeg* oritime = minheap.top();
		minheap.pop();
		xfdate = oritime->xfdate;
		xftime = oritime->xftime;
		val = oritime->val;
		cardid = oritime->cardid;
		window = oritime->window;
		if (!Consume(window, cardid, xfdate, xftime, val, oritime)) {
			Windows[window].xflist[Windows[window].pointer - 1].valid = false;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].xfdate <= "20211215") {
			minheap.push(&Windows[window].xflist[Windows[window].pointer++]);
		}
	}
	//end = clock();
	//cout << "12.16前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "12.16凌晨充值开始:" << endl;
	//start = clock();
	cztime = czmeg.substr(0, 16);
	cztype = czmeg.substr(17, 4);
	stuid = czmeg.substr(22, 10);
	val = stoi(czmeg.substr(33));
	major = stoi(stuid.substr(4, 2));
	order = stoi(stuid.substr(6, 3));
	Recharge(stuid, double(val), "");
	while (getline(czfile, czmeg)) {
		if (czmeg.length() <= 33) break;
		else {
			cztime = czmeg.substr(0, 16);
			if (cztime.substr(4, 4) != "1216") break;
			cztype = czmeg.substr(17, 4);
			stuid = czmeg.substr(22, 10);
			val = stoi(czmeg.substr(33));
			major = stoi(stuid.substr(4, 2));
			order = stoi(stuid.substr(6, 3));
			Recharge(stuid, val, cztime);
		}
	}
	//end = clock();
	//cout << "12.16充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	//cout << "开始12.31(包括)之前的消费:" << endl;
	//start = clock();
	for (int i = 1; i < 58; ++i) {
		if (Windows[i].xflist[Windows[i].pointer].xfdate <= "20211231")
			minheap.push(&Windows[i].xflist[Windows[i].pointer++]);
	}
	while (!minheap.empty()) {
		XFMeg* oritime = minheap.top();
		minheap.pop();
		xfdate = oritime->xfdate;
		xftime = oritime->xftime;
		val = oritime->val;
		cardid = oritime->cardid;
		window = oritime->window;
		if (!Consume(window, cardid, xfdate, xftime, val, oritime)) {
			Windows[window].xflist[Windows[window].pointer - 1].valid = false;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].xfdate <= "20211231") {	//防止越界
			minheap.push(&Windows[window].xflist[Windows[window].pointer++]);
		}
	}
	end = clock();
	czfile.close();
	int sumLog = 0, successLog = 0;
	for (int i = 1; i < 58; ++i) {
		//int sumwindow = 0;
		for (auto &it : Windows[i].xflist) {
			if (it.valid) {
				++successLog;
				//++sumwindow;
			}
			++sumLog;
		}
		//windowslog("窗口" + to_string(i) + "没有回绕前的成功消费记录有" + to_string(sumwindow));
	}
	cout << "消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "sumLog:" << sumLog << " successLog:" << successLog << endl;
	roundData();
	for (int i = 1; i < 58; ++i) {
		int sumwindow = 0;
		for (auto &it : Windows[i].xflist) {
			if (it.valid) {
				++successLog;
				++sumwindow;
			}
			++sumLog;
		}
		windowslog("窗口" + to_string(i) + "回绕后的成功消费记录有" + to_string(sumwindow));
	}
	system("pause");
	return;
}
/*食堂管理模块的接口*/
void ShowCartoon() {
	int choice;
	cout << "进入食堂管理模块,请选择你需要的操作:" << endl;
	while (1) {
		cout << "1.单个校园卡的消费" << endl;
		cout << "2.批量消费(紧接第一个模块的批量操作来调用)" << endl;
		cout << "3.退出操作" << endl;
		cin >> choice;
		if (choice == 1) {
			system("cls");
			cout << "进入手工输入消费模式" << endl;
			string predate;
			bool flag = 0;
			while (1) {
				cout << "请输入当前时间和窗口,时间为第一行,窗口为第二行,退出按任意数字/字母键+回车" << endl;
				string time;
				int window;
				cin >> time;
				if (time.size() == 1) {
					cout << "退出!" << endl;
					system("pause");
					break;
				}
				cin >> window;
				setConfig(time, window);
				cout << "设置当前时间为" << curTime << " 当前窗口为" << curWindow << endl;
				string cardid;
				double val;
				cout << "请输入卡号和消费金额,卡号在第一行,消费金额在第二行" << endl;
				cin >> cardid >> val;
				XFMeg curMeg = {
					cardid,
					curTime.substr(0, 8),
					curTime.substr(9, 8),
					val,
					curWindow,
					true,
					0,
					false
				};
				Windows[curWindow].xflist.push_back(curMeg);
				if (!sConsume(curWindow, cardid, curTime.substr(0, 8), curTime.substr(9, 8), val, &curMeg)) {
					curMeg.valid = false;
				}
				if (!flag && curMeg.valid) {
					flag = 1;
					predate = curMeg.xfdate;
					daysum[curMeg.window] += curMeg.val;
				}
				else if (flag && curMeg.xfdate == predate && curMeg.valid) {
					daysum[curMeg.window] += curMeg.val;
				}
				else if (flag && curMeg.valid && curMeg.xfdate != predate) {
					for (int i = 1; i < 58; ++i) {
						if (daysum[i] > 0) cout << "窗口" << i << "在日期" << predate << "的总成功消费金额为" << daysum[i] << endl;
						daysum[i] = 0;
					}
					daysum[curMeg.window] += curMeg.val;
					predate = curMeg.xfdate;
				}
				system("pause");
			}
			QuickSort();
			int sumSuccessLog = QuickSortVector.size();
			for (int j = 1; j < 58; ++j) {
				if (daysum[j] > 0) {
					cout << "窗口" << j << "在日期" << QuickSortVector[QuickSortVector.size() - 1]->xfdate << "的总消费记录金额为" << daysum[j] << endl;
					daysum[j] = 0;
				}
			}
			cout << "共有" << sumSuccessLog << "成功的消费记录" << endl;
			system("cls");
			/*string predate = QuickSortVector[0]->xfdate;
			for (int i = 0; i < QuickSortVector.size(); ++i) {
				if (!QuickSortVector[i]->valid) continue;
				else {
					if (QuickSortVector[i]->xfdate != predate) {
						for (int j = 1; j < 58; ++j) {
							if (daysum[j] > 0) {
								cout << "窗口" << j << "在日期" << predate << "的总消费记录金额为" << daysum[j] << endl;
								daysum[j] = 0;
							}
						}
						predate = QuickSortVector[i]->xfdate;
						daysum[QuickSortVector[i]->window] = QuickSortVector[i]->val;
					}
					else {
						daysum[QuickSortVector[i]->window] += QuickSortVector[i]->val;
					}
				}
			}
			for (int j = 1; j < 58; ++j) {
				if (daysum[j] > 0) {
					cout << "窗口" << j << "在日期" << QuickSortVector[QuickSortVector.size() - 1]->xfdate << "的总消费记录金额为" << daysum[j] << endl;
					daysum[j] = 0;
				}
			}
			*/
		}
		else if (choice == 2) {
			system("cls");
			QuickSortVector.clear();
			initWindows();
			read2XF();
			batchXF();
			cout << "退出食堂管理模块" << endl;
			system("pause");
			system("cls");
			return;
		}
		else if (choice == 3) {
			cout << "退出食堂管理模块" << endl;
			system("pause");
			system("cls");
			return;
		}
	}
}
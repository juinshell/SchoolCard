#include<iostream>
#include<fstream>
#include "def.h"
#include<cassert>
#include<queue>
#include<time.h>
#include<io.h>
#include <sstream>
using namespace std;
string curTime;
int curWindow;
vector<WindowManager> Windows(100);
double sumMoney;
int sumTimes;

extern string CardID2ActMap[CARD_MAX_NUM];
extern CardAct AllAccount[STU_MAJOR_MAX_NUM][STU_MAX_NUM_IN_MAJOR];
extern fstream xflogWriter, czlogWriter;

struct cmp
{
	bool operator()(XFMeg *a, XFMeg *b) const
	{
		return a->xfdate + a->xftime > b->xfdate + b->xftime;
	}
};

priority_queue<XFMeg*, vector<XFMeg*>, cmp> minheap;

void initWindows() {
	for (int i = 1; i < 58; ++i) {
		Windows[i].num = i;
		Windows[i].pointer = 0;
	}
}
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
					0
				}
			);
		}
	}
	end = clock();
	cout << "xffile read finish! cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	xffile.close();
}


void setConfig(string curtime, int curwindow) {
	curTime = curtime;
	curWindow = curwindow;
}
bool Consume(int window, string cardid, string date, string time, double val, XFMeg* xfmeg) {//批量消费的单个逻辑
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
	/*string curhour = time.substr(1, 2);
	if ((curhour >= "07" && curhour <= "09") || (curhour >= "11" && curhour <= "13") || (curhour >= "17" && curhour <= "19")) {

	}*/
	CardID* curCard = curAct->cardList.tail->pre;
	curAct->cardList.tail->pre->balance -= val;
	xfmeg->balance = curAct->cardList.tail->pre->balance;
	xflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance));
	return true;
}

void sConsume(string cardid, double val) {
	string serialnum = cardid.substr(1, 5);
	string uid = CardID2ActMap[stoi(serialnum)];
	if (uid.size() != 10) {
		cout << "该卡号不存在" << endl;
		return;
	}
	int major = stoi(uid.substr(4, 2));
	int order = stoi(uid.substr(6, 3));
	CardAct* curAct = &AllAccount[major][order];
	//检查是否为有效卡
	if (!curAct->cardList.size || curAct->cardList.tail->pre->serialNum != serialnum || !curAct->cardList.tail->pre->valid) {
		cout << "该卡号无效!" << endl;
		return;
	}
	if (curAct->cardList.tail->pre->balance < val) {
		cout << "卡余额不足" << endl;
		return;
	}
	/*string curhour = time.substr(1, 2);
	if ((curhour >= "07" && curhour <= "09") || (curhour >= "11" && curhour <= "13") || (curhour >= "17" && curhour <= "19")) {

	}*/
	CardID* curCard = curAct->cardList.tail->pre;
	curAct->cardList.tail->pre->balance -= val;
	cout << "成功消费" << val << endl;
	cout << "卡号:" << cardid << " 余额:" << curCard->balance << endl;
	xflog(curTime + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(curWindow) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(curAct->cardList.tail->pre->balance));
	sumMoney += val;
	sumTimes++;
	return;
}
void SumDay() {
	cout << curTime.substr(0, 8) << "的累计收费金额为" << sumMoney << " 窗口" << curWindow << "的累计交易次数为" << curTime << endl;
	return;
}

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
	cout << "开始9.4充值" << endl;
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
	end = clock();
	cout << "9.4充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl << "9.6挂失补卡开始:" << endl;
	//结束9.4充值，开始9.6挂失补卡操作
	start = clock();
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
	end = clock();
	cout << "9.6补卡结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl << "开始10.14前的消费:" << endl;
	//结束9.6补卡，开始到10.13日的消费
	//search();
	start = clock();
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
	end = clock();
	cout << "10.14前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "10.14凌晨充值开始" << endl;
	start = clock();
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
	end = clock();
	cout << "10.14充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "开始11.4前的消费:" << endl;
	start = clock();
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
	end = clock();
	cout << "11.4前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "11.4凌晨充值开始:" << endl;
	start = clock();
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
	end = clock();
	cout << "11.04充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "开始11.25前的消费:" << endl;
	start = clock();
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
	end = clock();
	cout << "11.25前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "11.25凌晨充值开始:" << endl;
	start = clock();
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
	end = clock();
	cout << "11.25充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "开始12.16前的消费:" << endl;
	start = clock();
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
	end = clock();
	cout << "12.16前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "12.16凌晨充值开始:" << endl;
	start = clock();
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
	end = clock();
	cout << "12.16充值结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "开始12.31(包括)之前的消费:" << endl;
	start = clock();
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
	cout << "12.31(包括)前消费结束,cost:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
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
	return;
}


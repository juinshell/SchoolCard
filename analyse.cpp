#include"def.h"
#include<iostream>
#include<queue>
#include<time.h>
using namespace std;

extern vector<WindowManager> Windows;
extern string CardID2ActMap[CARD_MAX_NUM];
extern CardAct AllAccount[STU_MAJOR_MAX_NUM][STU_MAX_NUM_IN_MAJOR];

vector<XFMeg*> QuickSortVector;
struct cmpp
{
	bool operator()(XFMeg *a, XFMeg *b) const
	{
		return a->xfdate + a->xftime > b->xfdate + b->xftime;
	}
};
priority_queue<XFMeg*, vector<XFMeg*>, cmpp> multiMergeHeap;
void MultipleMerge() {
	clock_t start, end;
	start = clock();
	for (int i = 0; i < 58; ++i) {
		Windows[i].pointer = 0;
	}
	for (int i = 1; i < 58; ++i) {
		while (!Windows[i].xflist[Windows[i].pointer].valid) Windows[i].pointer++;	//没有判断边界是安全的
		multiMergeHeap.push(&Windows[i].xflist[Windows[i].pointer]);
		Windows[i].pointer++;
	}
	int window;
	string date, time, cardid;
	double val;
	CardAct* curAct;
	while (!multiMergeHeap.empty()) {
		XFMeg* oritime = multiMergeHeap.top();
		multiMergeHeap.pop();

		window = oritime->window;
		cardid = oritime->cardid;
		time = oritime->xftime;
		date = oritime->xfdate;
		val = oritime->val;

		string serialnum = cardid.substr(1, 5);
		string uid = CardID2ActMap[stoi(serialnum)];
		if (uid.size() != 10) {
			cout << "Error!" << cardid << endl;
			return;
		}
		int major = stoi(uid.substr(4, 2));
		int order = stoi(uid.substr(6, 3));
		curAct = &AllAccount[major][order];

		sortxflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(oritime->balance));
		while (Windows[window].pointer < Windows[window].xflist.size() && !(Windows[window].xflist[Windows[window].pointer].valid)) {
			Windows[window].pointer++;
		}
		if (Windows[window].pointer < Windows[window].xflist.size() && Windows[window].xflist[Windows[window].pointer].valid) {
			multiMergeHeap.push(&Windows[window].xflist[Windows[window].pointer]);
			Windows[window].pointer++;
		}
	}
	end = clock();
	cout << "多路归并排序消费记录完成! 用时:" << (double)(end - start) / CLOCKS_PER_SEC << "秒" << endl;
	return;
}

void quick_sort(int l, int r)
{
	if (l >= r) return;

	int i = l - 1, j = r + 1;
	XFMeg* x = QuickSortVector[l + r >> 1];
	while (i < j)
	{
		do i++; while (QuickSortVector[i]->xfdate + QuickSortVector[i]->xftime < x->xfdate + x->xftime);
		do j--; while (QuickSortVector[j]->xfdate + QuickSortVector[j]->xftime > x->xfdate + x->xftime);
		if (i < j) swap(QuickSortVector[i], QuickSortVector[j]);
	}
	quick_sort(l, j), quick_sort(j + 1, r);
}

void QuickSort() {
	clock_t start, end;
	start = clock();
	for (int i = 1; i < 58; ++i) {
		for (auto &it : Windows[i].xflist) {
			if (it.valid) QuickSortVector.push_back(&it);
		}
	}
	quick_sort(0, QuickSortVector.size() - 1);
	string date, time, cardid;
	double val, balance;
	int window;
	for (auto &it : QuickSortVector) {
		date = it->xfdate;
		time = it->xftime;
		cardid = it->cardid;
		val = it->val;
		window = it->window;
		balance = it->balance;
		sortxflog_quick_sort(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(balance));
	}
	end = clock();
	cout << "快速排序消费记录完成! 用时:" << (double)(end - start) / CLOCKS_PER_SEC << "秒" << endl;
	return;
}

vector<XFMeg*> sumDuration(string startDateTime, string endDateTime, string uid) {
	vector<XFMeg*> ret;
	int startIndex = 0, endIndex;
	XFMeg* curMeg;
	while (QuickSortVector[startIndex]->xfdate + QuickSortVector[startIndex]->xftime < startDateTime) {
		startIndex++;
	}
	endIndex = startIndex;
	while (QuickSortVector[endIndex]->xfdate + QuickSortVector[endIndex]->xftime <= endDateTime) {
		endIndex = min(endIndex + 1, int(QuickSortVector.size()) - 1);
		if (endIndex == int(QuickSortVector.size()) - 1) break;
	}
	while (startIndex <= endIndex) {
		string serialnum = QuickSortVector[startIndex]->cardid.substr(1, 5);
		string curuid = CardID2ActMap[stoi(serialnum)];
		if (curuid != uid) {	//验证是否是查询的学号对应的卡
			startIndex++;
			continue;
		}
		ret.push_back(QuickSortVector[startIndex]);
		startIndex++;
	}
	return ret;
}

void search() {
	string choice;
	cout << "进入查询模块,以下为功能菜单:" << endl;
	cout << "1.提供姓名或者学号查询账户(支持模糊匹配)" << endl;
	cout << "2.提供时间段、学号、姓名和消费金额范围的消费记录查询" << endl;
	cin >> choice;
	if (choice.size() == 1 && choice[0] == '1') {
		system("clear");
		cout << "请输入查询的学号或者姓名,姓名开头为@，学号开头为&" << endl;
		string user_pattern;
		cin >> user_pattern;
		bool flag = (user_pattern[0] == '@') ? true : false;
		BlurSearch(user_pattern.substr(1), flag);
	}
	else if (choice[0] == '2') {
		system("clear");
		string meg;
		cout << "请输入查询的时间段、学号、姓名和消费金额范围,格式为:开始时间-截止时间,学号,姓名,消费最低金额-消费最高金额" << endl;
		cin >> meg;
		string startDate = meg.substr(0, 8);
		string startTime = meg.substr(9, 8);
		string uid = meg.substr(18, 8);
		string name = meg.substr(27, 8);
		int index = meg.find_last_of("-");
		double minMoney = stod(meg.substr(36, index - 36));
		double maxMoney = stod(meg.substr(index + 1));
		cout << startDate << " " << startTime << " " << uid << " " << name << " " << minMoney << "-" << maxMoney << endl;
	}
	return;
}
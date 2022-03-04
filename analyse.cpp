#include"def.h"
#include<iostream>
#include<queue>
#include<time.h>
#include<unordered_map>
#include <functional>
using namespace std;

extern fstream xflogReader;
extern vector<WindowManager> Windows;
extern string CardID2ActMap[CARD_MAX_NUM];
extern CardAct AllAccount[STU_MAJOR_MAX_NUM][STU_MAX_NUM_IN_MAJOR];
extern size_t LogHash[MAX_LOG_NUM];

vector<XFMeg*> QuickSortVector;
/*重载multiMergeHeap需要的运算符，表示小根堆*/
struct cmpp
{
	bool operator()(XFMeg *a, XFMeg *b) const
	{
		return a->xfdate + a->xftime > b->xfdate + b->xftime;
	}
};
/*重载DataDig中TimesHeap需要的运算符，表示大顶堆*/
struct compare
{
	bool operator()(pair<string, int> a, pair<string, int> b) const
	{
		return b.second < a.second;
	}
};

priority_queue<XFMeg*, vector<XFMeg*>, cmpp> multiMergeHeap;	//多路归并排序需要的小根堆，按时间排序
/*将WindowsManager数组中记录的所有有效的消费记录进行排序*/
void MultipleMerge() {
	//多路归并排序，将所有窗口的信息4
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

		//sortxflog(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(oritime->balance));
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
/*QuickSort的分治逻辑*/
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
/*快速排序WindowsManager数组中记录的所有有效消费记录*/
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
		//sortxflog_quick_sort(date + "," + time + "消费==>" + "卡号:" + cardid + " 窗口:" + to_string(window) + " 金额:" + to_string(val) + "消费成功! 余额" + to_string(balance));
	}
	end = clock();
	cout << "快速排序消费记录完成! 用时:" << (double)(end - start) / CLOCKS_PER_SEC << "秒" << endl;
	return;
}
/*从QuickSortVector中找出负荷而时间段并且学号匹配的消费记录并返回其vector*/
vector<XFMeg*> sumDuration(string startDateTime, string endDateTime, string uid) {	//查询起止日期内满足学号的所有消费
	vector<XFMeg*> ret;
	int startIndex = 0, endIndex;
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
/*将时间一个int型整数，用于数据挖掘比较相邻消费时间*/
int time2int(string time) {
	int sum = 0;
	sum += time[7] - '0';
	sum += 10 * (time[6] - '0');
	sum += 100 * (time[5] - '0');
	sum += 1000 * (time[4] - '0');
	sum += 6000 * (time[3] - '0');
	sum += 60000 * (time[2] - '0');
	sum += 360000 * (time[1] - '0');
	sum += 3600000 * (time[0] - '0');
	return sum;
}
/*数据挖掘逻辑，手机指定学号的每次消费的前后五分钟的相邻两个窗口的所有消费记录，然后取最高的十个人，认定为有关*/
void DataDig(string uid) {
	vector<int> DataDigVector;
	for (int i = 0; i < QuickSortVector.size(); ++i) {
		if (CardID2ActMap[stoi(QuickSortVector[i]->cardid.substr(1, 5))] == uid) DataDigVector.push_back(i);
	}
	unordered_map<string, int> times;
	cout << "该学生一共有" << DataDigVector.size() << "个消费记录" << endl;
	for (int i = 0; i < DataDigVector.size(); ++i) {
		int backIndex = DataDigVector[i] - 1, frontIndex = DataDigVector[i] + 1;
		while (backIndex >= 0) {
			if (abs(QuickSortVector[backIndex]->window - QuickSortVector[DataDigVector[i]]->window) <= 2 && time2int(QuickSortVector[DataDigVector[i]]->xftime) - time2int(QuickSortVector[backIndex]->xftime) <= 30000) {
				times[CardID2ActMap[stoi(QuickSortVector[backIndex]->cardid.substr(1, 5))]] ++;
			}
			else if (time2int(QuickSortVector[DataDigVector[i]]->xftime) - time2int(QuickSortVector[backIndex]->xftime) > 30000) {
				break;
			}
			backIndex--;
		}
		while (frontIndex < QuickSortVector.size()) {
			if (abs(QuickSortVector[frontIndex]->window - QuickSortVector[DataDigVector[i]]->window) <= 2 && time2int(QuickSortVector[frontIndex]->xftime) - time2int(QuickSortVector[DataDigVector[i]]->xftime) <= 30000) {
				times[CardID2ActMap[stoi(QuickSortVector[frontIndex]->cardid.substr(1, 5))]] ++;
			}
			else if (time2int(QuickSortVector[frontIndex]->xftime) - time2int(QuickSortVector[DataDigVector[i]]->xftime) > 30000) {
				break;
			}
			frontIndex++;
		}
	}
	priority_queue<pair<string, int>, vector<pair<string, int>>, compare> TimesHeap;
	for (auto &it : times) {
		TimesHeap.push({ it.first, it.second });
		if (TimesHeap.size() > 10) TimesHeap.pop();
	}
	cout << "经过数据挖掘后,得出的可能有关学号为:" << endl;
	while (!TimesHeap.empty()) {
		pair<string, int> st = TimesHeap.top();
		TimesHeap.pop();
		cout << st.first << ":" << st.second << endl;
	}
	cout << endl;
	system("pause");
	return;
}
/*Hash校验方式的Hash函数*/
size_t MyHash(string s) {
	std::hash<std::string> h;
	size_t n = h(s);
	return n;
}
/*用于判断指定范围行的消费记录是否被篡改并在控制台输出*/
void checkLines(int startline, int endline) {
	string meg;
	int line = startline;
	while (getline(xflogReader, meg)) {
		size_t meg_hash = MyHash(meg);
		if (meg_hash != LogHash[line++]) {
			cout << "校验码验证失败," << line - 1 << "行数据已被篡改" << endl;
		}
		else {
			cout << "校验码验证成功," << line - 1 << "行数据未被篡改" << endl;
		}
		if (line > endline) break;
	}
	return;
}
/*search模块的接口*/
void search() {
	string choice;
	cout << "进入统计查询模块,以下为功能菜单:" << endl;
	while (1) {
		cout << "1.提供姓名或者学号查询账户(支持模糊匹配)" << endl;
		cout << "2.提供时间段、学号、姓名和消费金额范围的消费记录查询" << endl;
		cout << "3.多路归并排序(批量)" << endl;
		cout << "4.快速排序" << endl;
		cout << "5.进行数据挖掘" << endl;
		cout << "6.进行校验码核对" << endl;
		cout << "7.校验消费日志指定范围行" << endl;
		cout << "8.退出当前模块" << endl;
		cin >> choice;
		if (choice.size() == 1 && choice[0] == '1') {
			system("cls");
			cout << "请输入查询的学号或者姓名,姓名开头为@，学号开头为&" << endl;
			string user_pattern;
			cin >> user_pattern;
			bool flag = (user_pattern[0] == '@') ? true : false;
			BlurSearch(user_pattern.substr(1), flag);
			system("pause");
		}
		else if (choice[0] == '2') {
			system("cls");
			string meg;
			cout << "请输入查询的时间段、学号、姓名和消费金额范围,格式为:开始时间-截止时间,学号,姓名,消费最低金额-消费最高金额" << endl;
			cin >> meg;
			string startDate = meg.substr(0, 8);
			string startTime = meg.substr(9, 8);
			string endDate = meg.substr(18, 8);
			string endTime = meg.substr(27, 8);
			string uid = meg.substr(36, 10);
			int index1 = meg.find_last_of(",");
			string name = meg.substr(47, index1 - 47);
			int index2 = meg.find_last_of("-");
			double minMoney = stod(meg.substr(index1 + 1, index2 - index1 - 1));
			double maxMoney = stod(meg.substr(index2 + 1));
			cout << "查询信息如下:" << endl;
			cout << startDate << startTime << "到" << endDate << endTime << " 学号为" << uid << " 姓名为" << name << " 消费金额为" << minMoney << "-" << maxMoney << endl;
			int major = stoi(uid.substr(4, 2));
			int order = stoi(uid.substr(6, 3));
			CardAct* curAct = &AllAccount[major][order];
			if (!checkActValid(uid) || curAct->name != name) {
				cout << "学号与姓名输入有误" << endl;
				system("pause");
				continue;
			}
			vector<XFMeg*> sumVector = sumDuration(startDate + startTime, endDate + endTime, uid);
			for (auto &it : sumVector) {
				if (it->val >= minMoney && it->val <= maxMoney)
					cout << it->xfdate << "," << it->xftime << "卡号:" << it->cardid << " 窗口:" << it->window << " 消费金额:" << it->val << " 余额:" << it->balance << endl;
			}
			system("pause");
		}
		else if (choice[0] == '3') {
			MultipleMerge();
			system("pause");
		}
		else if (choice[0] == '4') {
			QuickSort();
			system("pause");
		}
		else if (choice[0] == '5') {
			system("cls");
			cout << "请输入需要分析的学号:" << endl;
			string meg;
			cin >> meg;
			DataDig(meg);
		}
		else if (choice[0] == '6') {
			system("cls");
			string meg;
			cout << "请输入需要检验的消费记录行数与现存内容" << endl;
			cin >> meg;
			int line = stoi(meg);
			cin.ignore();
			getline(cin, meg);
			//cout << line << endl << meg << endl;
			size_t meg_hash = MyHash(meg);
			if (meg_hash != LogHash[line]) {
				cout << "校验码验证失败,该行数据已被篡改" << endl;
			}
			else {
				cout << "校验码验证成功,该行数据未被篡改" << endl;
			}
			system("pause");
		}
		else if (choice[0] == '7') {
			system("cls");
			xflogReader.open("E:\\classdesign\\xflog.txt", ios::in);
			assert(xflogReader.is_open());
			int line1, line2;
			cout << "请输入起止与结束行的行数,按空格或回车分隔:" << endl;
			cin >> line1 >> line2;
			cout << "即将校验" << line1 << "到" << line2 << "中的所有消费记录" << endl;
			checkLines(line1, line2);
			xflogReader.close();
			system("pause");
		}
		else if (choice[0] == '8') {
			return;
		}
		system("cls");
	}
	return;
}
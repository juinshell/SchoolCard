#include<string>
#include<vector>
#include<fstream>
#include<sstream>
#include<cassert>
using namespace std;
#define OK "OK"
#define UNVALID "UNVALID"
#define STU_MAJOR_MAX_NUM 100
#define STU_MAX_NUM_IN_MAJOR 1000 
#define CARD_MAX_NUM 100000
#define VALID_TIME "20240715"
#define MAX_BALANCE 999.99
extern fstream czlogWriter, xflogWriter, sortxflogWriter, sortxflogWriter_quick_sort, windowlogWriter;
extern FILE *xflogWriter1;

struct CardID {
	int			headNum;    //发卡点编号，默认为3
	string		serialNum;  //流水号
	int			checkCode;
	bool		valid;
	double		balance;
	CardID*		pre;
	CardID*		next;
};
struct CardList {
	CardID* head;
	CardID* tail;
	int		size;
};
struct CardAct {
	string		stuID;
	CardList    cardList;
	string      name;
	string      validTime;
	string      status;
};

struct XFMeg {
	string cardid;
	string xfdate;
	string xftime;
	double val;
	int window;
	bool valid;
	double balance;
};
struct WindowManager {
	int num;
	int pointer;
	vector<XFMeg> xflist;
};
//student.cpp
void readKHFile(void);
void readCZFile(void);
vector<CardAct> BlurSearch(string, bool);
void CreateAct(string, string, string);
void DeleteAct(string, string);
void EraseLoss(string, string);
void MakeupCard(string, string);
void Recharge(string, double, string);
void SendCardID(string, string);
void ReportLoss(string, string);
void showActMeg(string);
//log.cpp
void GenerateLog(string);
void xflog(string);
void sortxflog(string);
void sortxflog_quick_sort(string);
void windowslog(string);
//cartoon.cpp
void sConsume(string, double);
void read2XF(void);
void initWindows(void);
void read2XF(void);
void batchXF(void);
void SumDay(void);
void setConfig(string, int);

void search(void);
//analyse.cpp
void MultipleMerge(void);
void QuickSort(void);



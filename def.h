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
#define MAX_LOG_NUM 1800000
extern fstream czlogWriter, xflogWriter, sortxflogWriter, sortxflogWriter_quick_sort, windowlogWriter;
extern FILE *xflogWriter1;

struct CardID {
	int			headNum;    //发卡点编号，默认为3
	string		serialNum;  //流水号
	int			checkCode;	//校验码
	bool		valid;		//表示该卡是否有效
	double		balance;	//余额
	CardID*		pre;		//双向链表，下同
	CardID*		next;
};
struct CardList {	//双向链表manager
	CardID* head;
	CardID* tail;
	int		size;
};
struct CardAct {			//校园卡账户主体
	string		stuID;		//校园卡卡号
	CardList    cardList;	//表示历史卡的双向链表
	string      name;		//校园卡账户所有者名
	string      validTime;	//过期时间
	string      status;		//账户状态
};

struct XFMeg {			//消费记录
	string cardid;		//卡号
	string xfdate;		//消费日期
	string xftime;		//消费时间
	double val;			//消费金额
	int window;			//消费窗口
	bool valid;			//消费是否有效
	double balance;		//消费后的该卡月
	bool secretneed;	//表示是否需要输入密码
};
struct WindowManager {		//窗口manager
	int num;				//窗口号
	int pointer;			//排序时需要的位置定位为
	vector<XFMeg> xflist;	//记录窗口对应的所有消费
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
bool checkActValid(string);
//log.cpp
void GenerateLog(string);
void xflog(string);
void sortxflog(string);
void sortxflog_quick_sort(string);
void windowslog(string);
//cartoon.cpp
void ShowCartoon(void);
void read2XF(void);
void initWindows(void);
void read2XF(void);
void batchXF(void);
void setConfig(string, int);
void ShowCartoon(void);
//analyse.cpp
void search(void);
void MultipleMerge(void);
void QuickSort(void);
size_t MyHash(string);
//menu.cpp
int menu(void);
void Welcome(void);


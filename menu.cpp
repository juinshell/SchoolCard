#include<iostream>
#include<windows.h>
#include <graphics.h>
#include<conio.h>
using namespace std;
/*改变console光标位置*/
void gotoxy(int x, int y) {
	COORD c;
	c.X = x;
	c.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
/*设置控制台窗口字体颜色和背景色*/
int color(int c) {
	//SetConsoleTextAttribute是API设置控制台窗口字体颜色和背景色的函数
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);        //更改文字颜色
	return 0;
}

void output_board(int i, int j, int chang, int kuan) {
	//(i,j)表示边框的左上角
	//i代表第几行,j代表第几列 
	int last_line = i + kuan - 1;
	int last_col = j + chang - 1;
	int first_j = j;
	int first_i = i;
	for (; i <= last_line; i++) {
		//先遍历行 
		j = first_j;
		for (; j <= last_col; j++) {
			//再遍历列 
			gotoxy(j, i);
			if (i == first_i || i == last_line) {
				printf("-");
			}
			else if (j == first_j || j == last_col) {
				printf("|");
			}
		}
	}

	gotoxy(0, 0);//控制台输出位置归零
}
/*显示Welcome界面*/
void Welcome() {
	initgraph(640, 480, EW_SHOWCONSOLE);

	IMAGE bg;
	loadimage(&bg, _T("./Hustseals.png"));
	putimage(150, 50, &bg);
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 20;
	settextstyle(&f);
	char wel_1[] = "WELCOME TO HERE!";
	char wel_2[] = "--by husterdjx";
	outtextxy(230, 350, wel_1);
	outtextxy(390, 370, wel_2);
	Sleep(2000);
	cleardevice();
	closegraph();
}
/*初始化菜单*/
int menu() {
	//显示菜单
	system("title 校园卡管理系统");
	int i, j;
	color(2);

	gotoxy(43, 18);
	//淡蓝色，居中输出
	color(11);
	printf("校园卡管理系统");

	color(14);
	//白色，输出边框
	output_board(20, 27, 47, 6);

	color(10);
	//绿色,输出选项 
	gotoxy(35, 22);
	printf("1.进入校园卡管理系统");
	gotoxy(55, 22);
	printf("0.退出");

	gotoxy(29, 27);
	//蓝色，输出选项 
	color(3);
	printf("请选择[1 0]:[ ]");

	//作者署名
	color(7);
	gotoxy(90, 27);
	printf("by  husterdjx");

	HANDLE hout;
	COORD coord;
	coord.X = 42;
	coord.Y = 27;
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hout, coord);

	int choice;
	cin >> choice;
	color(7);       // 恢复控制台的颜色
	gotoxy(0, 0);    // 恢复控制台输出的位置
	system("color f0");
	return choice;
}

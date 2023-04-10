#include<fstream>
#include<iostream>
#include<string>
#include<direct.h>
using namespace std;
int portnum=0;
int flownum=0;
int bg=0;
int et=0;
void Output(string& path,int& No)
{
	ofstream output;
	output.open(path + "/flow.txt", ios::out);
	output << "没什么用拿来占位的第一行，这是第" + to_string(No) + "个文件"<<endl;
	/**/
	for (int i = 0; i < flownum; i++)
	{
		int speed = rand() % 1000 + 1;//流带宽1-1000;
		int begintime = rand() % bg;
		int endtime = rand() % et+1;
		output << i << ',' << speed << ',' << begintime << ',' << endtime << endl;
	}
	output.close();

	output.open(path + "/port.txt", ios::out);
	output << "没什么用拿来占位的第一行，这是第" + to_string(No) + "个文件"<<endl;
	/**/
	for (int i = 0; i < portnum; i++)
	{
		int speed = rand() % 10 + 3;//端口带宽3000-12000
		speed *= 1000;
		output << i << ',' << speed << endl;
	}
	output.close();
}
int main()
{
	int No = 0;
	string path;
	int seed = 20230409;
	srand(seed);
	for (No = 0; No < 10; No++)
	{
		path = "../data/" + to_string(No);
		
		_mkdir(path.c_str());
		portnum = rand() % 10 + 5;//端口数量5-14
		flownum = rand() % 10 + 5;
		flownum *= 1000;//流数量5000-14000
		bg = rand() % 50 + 50;//流最大开始时间50-99
		et = rand() % 50 + 50;//流最大需要时间50-99
		Output(path, No);
	}
}
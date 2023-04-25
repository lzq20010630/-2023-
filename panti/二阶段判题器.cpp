#include<fstream>
#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<map>
#include<limits.h>
#include<deque>
#include <iomanip>
#include<cmath>
using namespace std;
class Flow
{
public:
	int id;
	int speed;
	int begintime;
	int sendtime;
	int needtime;
	bool issend;
	Flow(int i, int s, int b, int n);
};
class Port
{
public:
	int id;
	int speed;
	int maxspeed;
	multimap<int, Flow> flowqueue;
	deque<Flow> waitqueue;
	Port(int i, int s);
};
class Result
{
public:
	int flowid;
	int portid;
	int sendtime;
	Result(int f, int p, int s);
};
Flow::Flow(int i, int s, int b, int n)
{
	id = i;
	speed = s;
	begintime = b;
	needtime = n;
	issend = false;
	sendtime = -1;
}
Port::Port(int i, int s)
{
	id = i;
	speed = s;
	maxspeed = speed;
}
Result::Result(int f, int p, int s)
{
	flowid = f;
	portid = p;
	sendtime = s;
}

/*负责数据的输入部分，将两个文件里的数据读入处理*/
bool Input(string path, vector<Flow>& flows, vector<Port>& ports, vector<Result>& results, int& maxcachesize)
{
	ifstream input;
	int allspeed = 0;
	int alltime = 0;
	int allportspeed = 0;
	int flowcount = 0;
	int portcount = 0;
	string path1 = path + "/flow.txt";
	string path2 = path + "/port.txt";
	string path3 = path + "/result.txt";
	input.open(path1, ios::in);
	if (!input.is_open())
		return false;
	input.ignore(100, '\n');
	/*输入flow*/
	while (!input.eof())
	{
		Flow flow(-1, 0, 0, 0);
		char t;
		input >> flow.id >> t >> flow.speed >> t >> flow.begintime >> t >> flow.needtime;
		if (flow.id == -1)
			break;
		allspeed += flow.speed;
		alltime += flow.needtime;
		++flowcount;
		flows.push_back(flow);
	}
	input.close();
	/*flow输入完毕*/
	input.open(path2, ios::in);
	if (!input.is_open())
		return false;
	input.ignore(100, '\n');
	/*输入port*/
	while (!input.eof())
	{
		Port port(-1, 0);
		char t;
		input >> port.id >> t >> port.maxspeed;
		port.speed = port.maxspeed;
		if (port.id == -1)
			break;
		allportspeed += port.speed;
		++portcount;
		ports.push_back(port);
	}
	input.close();
	//cout << "流带宽总和    ：" << allspeed << endl;
	//cout << "流占用时间总和：" << alltime << endl;
	//cout << "端口带宽总和  ：" << allportspeed << endl;
	//cout << "流数量        ：" << flowcount << endl;
	//cout << "端口数量      ：" << portcount << endl;
	//cout << "流带宽平均值  ：" << allspeed / double(flowcount) << endl;
	//cout << "端口带宽平均值：" << allportspeed / double(portcount) << endl;
	//cout << "流占用时间平均值：" << alltime / double(flowcount) << endl;
	//cout << endl;
	/*port输入完毕*/
	input.open(path3, ios::in);
	if (!input.is_open())
	{
		cout << "找不到结果文件" << endl;
		return false;
	}

	while (!input.eof())
	{
		Result res(-1, -1, -1);
		char t;
		input >> res.flowid >> t >> res.portid >> t >> res.sendtime;
		if (res.sendtime == -1)
			break;
		results.push_back(res);
	}
	maxcachesize = ports.size() * 20;
	sort(flows.begin(), flows.end(), [](const Flow& x, const Flow& y) {return x.begintime < y.begintime; });
	return true;
}
/*更新端口状态*/
void updateport(vector<Port>& ports, const int& time)
{

	for (auto& port : ports)//对每个端口进行处理
	{
		for (auto j = port.flowqueue.begin(); j != port.flowqueue.end();)//对端口中已发送的流检查是否发送完毕
		{
			if (j->first > time)//map为升序 所以最小值大于当前时间就代表没有发送完毕的流了
			{
				break;
			}
			else//反之还有发送完毕的流 把它占用的端口腾出来
			{
				port.speed += j->second.speed;//将端口带宽还原回去
				port.flowqueue.erase(j);//在在缓冲区中删除
				j = port.flowqueue.begin();
			}
		}
		while (!port.waitqueue.empty() && port.waitqueue.front().sendtime <= time)//对每个端口中等待队列发送时间小于等于当前时间的流检测一遍是否能发送
		{
			if (port.waitqueue.front().speed <= port.speed)//端口剩余空间足够，可以发送
			{
				port.flowqueue.insert(pair<int, Flow>(time + port.waitqueue.front().needtime, port.waitqueue.front()));//将这个流放入已发送队列
				port.speed -= port.waitqueue.front().speed;//将端口可用空间减去流需要占用的空间
				port.waitqueue.pop_front();//出等待队列
			}
			else
			{
				break;
			}
		}
	}

	//int maxtime = time;

	//return maxtime;
}
/*检查端口队列区情况，有溢出则清除溢出并计算加权时间*/
int checkport(vector<Port>& ports)
{
	int overflowtime = 0;
	for (auto& port : ports)
	{
		if (port.waitqueue.size() <= 30)
			continue;
		else
		{
			while (port.waitqueue.size() > 30)
			{
				int i = port.waitqueue.size() - 1;
				overflowtime += port.waitqueue[i].needtime;
				port.waitqueue.erase(port.waitqueue.begin() + i);
			}
		}
	}
	return overflowtime * 2;//2倍加权时间
}
/*数据处理*/
int algorithm(vector<Flow>& flows, vector<Port>& ports, vector<Result>& res, int& maxcachesize)
{

	if (res.size() < flows.size())
	{
		cout << "有流缺失，或数据输出格式有误" << endl;
		return 0;
	}
	vector<int>flowid(flows.size());
	for (int i = 0; i < flows.size(); ++i)
	{
		flowid[flows[i].id] = i;
	}

	int time = 0;
	int resultid = 0;
	int overflowtime = 0;
	while (true)
	{
		for (; resultid < res.size(); ++resultid)
		{
			int t = res[resultid].sendtime;
			if (t > time)//当前结果还没到发送时间
				break;
			if (res[resultid].flowid >= flows.size() || res[resultid].flowid < 0)
			{
				cout << "流id不存在，错误结果为" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (res[resultid].portid >= ports.size() || res[resultid].portid < 0)
			{
				cout << "端口id不存在，错误结果为" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}

			Flow& flow = flows[flowid[res[resultid].flowid]];
			Port& port = ports[res[resultid].portid];
			if (t < flow.begintime)
			{
				cout << "流发送时间小于进入设备时间，错误结果为" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (flow.speed > port.maxspeed)
			{
				cout << "流带宽大于端口最大带宽，错误结果为" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (flow.issend)
			{
				cout << "流被重复发送，错误结果为" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			flow.sendtime = t;
			port.waitqueue.push_back(flow);
			flow.issend = true;
		}


		updateport(ports, time);
		overflowtime += checkport(ports);
		int count = 0;
		for (const auto& flow : flows)
		{
			if (flow.begintime > time)
				break;
			if (!flow.issend)
			{
				++count;
			}
		}
		if (count > maxcachesize)
		{
			cout << "流调度区爆了！" << endl;
			return 0;
		}
		if (resultid >= res.size())
			break;
		++time;
	}
	while (true)//把排队区的所有流都发送出去
	{
		int count = 0;
		for (const auto& port : ports)
		{
			if (port.waitqueue.empty())
				++count;
		}
		if (count == ports.size())
			break;
		++time;
		updateport(ports, time);
	}



	for (const auto& flow : flows)
	{
		if (!flow.issend)
		{
			cout << "有流未被发送，未发送的流编号为" << flow.id << endl;
			return 0;
		}
	}
	int maxtime = time;

	for (auto& port : ports)//遍历所有端口已发送的队列，找到最晚发送完毕的时间并返回
	{
		if (port.flowqueue.empty())
			continue;
		else
		{
			auto last = port.flowqueue.end();
			--last;
			maxtime = max(maxtime, last->first);
		}
	}

	maxtime += overflowtime;
	return maxtime;
}
double best(vector<Flow>& flows, vector<Port>& ports)
{
	long long int needspeed = 0;
	long long int cansendspeed = 0;
	for (int i = 0; i < flows.size(); ++i)
	{
		needspeed += (long long int)flows[i].speed * flows[i].needtime;
	}
	for (int i = 0; i < ports.size(); ++i)
	{
		cansendspeed += ports[i].maxspeed;
	}
	return needspeed / double(cansendspeed);
}
int main()
{
	int No = 0;
	vector<Flow> flows;
	vector<Port> ports;
	vector<Result> res;
	int alltime = 0;
	double allbest = 0;
	double score = 0;
	double bestscore = 0;
	int maxcachesize = 0;
	string path;
	while (true)
	{
		path = "../data/" + to_string(No);
		if (!Input(path, flows, ports, res, maxcachesize))
			break;
		stable_sort(res.begin(), res.end(), [](const Result& x, const Result& y) {return x.sendtime < y.sendtime; });
		int thistime = algorithm(flows, ports, res, maxcachesize);
		double thisbest = best(flows, ports);
		alltime += thistime;
		allbest += thisbest;
		cout << "第" << No << "号文件："<<endl;
		cout <<"理论最优：" << thisbest << endl;
		cout <<"实际结果：" << thistime << endl;
		cout << "分数：" << 300 / (log(thistime) / log(10)) << endl;
		cout << "理论最高分数：" << 300 / (log(thisbest) / log(10)) << endl;
		cout << endl;
		score += 300 / (log(thistime) / log(10));
		bestscore += 300 / (log(thisbest) / log(10));
		++No;
		flows.clear();
		ports.clear();
		res.clear();
	}
	//cout << "总和理论最优：" << allbest << endl;
	//cout << "总和实际结果：" << alltime << endl;
	cout << "总分数：" << setprecision(10) << score / No << endl;
	cout << "总理论最高分数：" << setprecision(10) << bestscore / No << endl;

	return 0;
}

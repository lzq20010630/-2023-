#include<fstream>
#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<map>
#include<limits.h>
#include<deque>
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
bool Input(string path, vector<Flow>& flows, vector<Port>& ports, vector<Result>& results)
{
	ifstream input;
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
		ports.push_back(port);
	}
	input.close();
	/*port输入完毕*/
	input.open(path3, ios::in);
	if (!input.is_open())
	{
		cout << "找不到结果文件" << endl;
		return false;
	}

	while (!input.eof())
	{
		Result res(-1,-1,-1);
		char t;
		input >> res.flowid >> t >> res.portid >> t >> res.sendtime;
		if (res.sendtime == -1)
			break;
		results.push_back(res);
	}

	return true;
}
/*更新端口状态*/
int updateport(vector<Port>& ports)
{
	int time = 0;
	while (true)//循环直到所有端口都没有待发送的流
	{
		int waitqueueemptycount = 0;
		for (auto& port : ports)//对每个端口进行处理
		{
			if (port.waitqueue.empty())//这个端口已经没有待发送的流了
				++waitqueueemptycount;
			
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
		++time;
		if (waitqueueemptycount == ports.size())
			break;
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
	return maxtime;
}
/*数据处理*/
int algorithm(vector<Flow>& flows, vector<Port>& ports, vector<Result>& res)
{
	if (res.size() < flows.size())
	{
		cout << "有流缺失，或数据输出格式有误" << endl;
		return 0;
	}
	for(const auto& iter:res)
	{
		int t = iter.sendtime;
		if (iter.flowid >= flows.size() || iter.flowid < 0)
		{
			cout << "流id不存在，错误结果为" << t << ',' << iter.flowid << ',' << iter.portid << endl;
			return 0;
		}
		if (iter.portid >= ports.size() || iter.portid < 0)
		{
			cout << "端口id不存在，错误结果为" << t << ',' << iter.flowid << ',' << iter.portid << endl;
			return 0;
		}

		Flow &flow = flows[iter.flowid];
		Port &port = ports[iter.portid];
		if (t < flow.begintime)
		{
			cout << "流发送时间小于进入设备时间，错误结果为" << t << ',' << iter.flowid << ',' << iter.portid << endl;
			return 0;
		}
		if (flow.speed > port.maxspeed)
		{
			cout << "流带宽大于端口最大带宽，错误结果为" << t << ',' << iter.flowid << ',' << iter.portid << endl;
			return 0;
		}
		if (flow.issend)
		{
			cout << "流被重复发送，错误结果为" << t << ',' << iter.flowid << ',' << iter.portid << endl;
			return 0;
		}
		/*___________________________________________________________________________________________________*/
		flow.sendtime = t;
		port.waitqueue.push_back(flow);
		flow.issend = true;
	}
	return updateport(ports);
}
double best(vector<Flow>& flows, vector<Port>& ports)
{
	long long int needspeed = 0;
	long long int cansendspeed = 0;
	for (int i = 0; i < flows.size(); ++i)
	{
		needspeed += (long long int)(flows[i].speed) * flows[i].needtime;
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
	string path;
	while (true)
	{
		path = "../data/" + to_string(No);
		if (!Input(path, flows, ports,res))
			break;
		int thistime= algorithm(flows, ports, res);
		double thisbest = best(flows, ports);
		alltime += thistime;
		allbest += thisbest;
		cout <<"理论最优：" << thisbest << endl;
		cout <<"实际结果：" << thistime << endl;
		++No;
		flows.clear();
		ports.clear();
		res.clear();
	}
	cout << "总和理论最优：" << allbest << endl;
	cout << "总和实际结果：" << alltime;
	return 0;
}

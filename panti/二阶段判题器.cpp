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

/*�������ݵ����벿�֣��������ļ�������ݶ��봦��*/
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
	/*����flow*/
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
	/*flow�������*/
	input.open(path2, ios::in);
	if (!input.is_open())
		return false;
	input.ignore(100, '\n');
	/*����port*/
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
	//cout << "�������ܺ�    ��" << allspeed << endl;
	//cout << "��ռ��ʱ���ܺͣ�" << alltime << endl;
	//cout << "�˿ڴ����ܺ�  ��" << allportspeed << endl;
	//cout << "������        ��" << flowcount << endl;
	//cout << "�˿�����      ��" << portcount << endl;
	//cout << "������ƽ��ֵ  ��" << allspeed / double(flowcount) << endl;
	//cout << "�˿ڴ���ƽ��ֵ��" << allportspeed / double(portcount) << endl;
	//cout << "��ռ��ʱ��ƽ��ֵ��" << alltime / double(flowcount) << endl;
	//cout << endl;
	/*port�������*/
	input.open(path3, ios::in);
	if (!input.is_open())
	{
		cout << "�Ҳ�������ļ�" << endl;
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
/*���¶˿�״̬*/
void updateport(vector<Port>& ports, const int& time)
{

	for (auto& port : ports)//��ÿ���˿ڽ��д���
	{
		for (auto j = port.flowqueue.begin(); j != port.flowqueue.end();)//�Զ˿����ѷ��͵�������Ƿ������
		{
			if (j->first > time)//mapΪ���� ������Сֵ���ڵ�ǰʱ��ʹ���û�з�����ϵ�����
			{
				break;
			}
			else//��֮���з�����ϵ��� ����ռ�õĶ˿��ڳ���
			{
				port.speed += j->second.speed;//���˿ڴ���ԭ��ȥ
				port.flowqueue.erase(j);//���ڻ�������ɾ��
				j = port.flowqueue.begin();
			}
		}
		while (!port.waitqueue.empty() && port.waitqueue.front().sendtime <= time)//��ÿ���˿��еȴ����з���ʱ��С�ڵ��ڵ�ǰʱ��������һ���Ƿ��ܷ���
		{
			if (port.waitqueue.front().speed <= port.speed)//�˿�ʣ��ռ��㹻�����Է���
			{
				port.flowqueue.insert(pair<int, Flow>(time + port.waitqueue.front().needtime, port.waitqueue.front()));//������������ѷ��Ͷ���
				port.speed -= port.waitqueue.front().speed;//���˿ڿ��ÿռ��ȥ����Ҫռ�õĿռ�
				port.waitqueue.pop_front();//���ȴ�����
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
/*���˿ڶ������������������������������Ȩʱ��*/
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
	return overflowtime * 2;//2����Ȩʱ��
}
/*���ݴ���*/
int algorithm(vector<Flow>& flows, vector<Port>& ports, vector<Result>& res, int& maxcachesize)
{

	if (res.size() < flows.size())
	{
		cout << "����ȱʧ�������������ʽ����" << endl;
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
			if (t > time)//��ǰ�����û������ʱ��
				break;
			if (res[resultid].flowid >= flows.size() || res[resultid].flowid < 0)
			{
				cout << "��id�����ڣ�������Ϊ" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (res[resultid].portid >= ports.size() || res[resultid].portid < 0)
			{
				cout << "�˿�id�����ڣ�������Ϊ" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}

			Flow& flow = flows[flowid[res[resultid].flowid]];
			Port& port = ports[res[resultid].portid];
			if (t < flow.begintime)
			{
				cout << "������ʱ��С�ڽ����豸ʱ�䣬������Ϊ" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (flow.speed > port.maxspeed)
			{
				cout << "��������ڶ˿�������������Ϊ" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
				return 0;
			}
			if (flow.issend)
			{
				cout << "�����ظ����ͣ�������Ϊ" << res[resultid].flowid << ',' << res[resultid].portid << ',' << t << endl;
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
			cout << "�����������ˣ�" << endl;
			return 0;
		}
		if (resultid >= res.size())
			break;
		++time;
	}
	while (true)//���Ŷ����������������ͳ�ȥ
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
			cout << "����δ�����ͣ�δ���͵������Ϊ" << flow.id << endl;
			return 0;
		}
	}
	int maxtime = time;

	for (auto& port : ports)//�������ж˿��ѷ��͵Ķ��У��ҵ���������ϵ�ʱ�䲢����
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
		cout << "��" << No << "���ļ���"<<endl;
		cout <<"�������ţ�" << thisbest << endl;
		cout <<"ʵ�ʽ����" << thistime << endl;
		cout << "������" << 300 / (log(thistime) / log(10)) << endl;
		cout << "������߷�����" << 300 / (log(thisbest) / log(10)) << endl;
		cout << endl;
		score += 300 / (log(thistime) / log(10));
		bestscore += 300 / (log(thisbest) / log(10));
		++No;
		flows.clear();
		ports.clear();
		res.clear();
	}
	//cout << "�ܺ��������ţ�" << allbest << endl;
	//cout << "�ܺ�ʵ�ʽ����" << alltime << endl;
	cout << "�ܷ�����" << setprecision(10) << score / No << endl;
	cout << "��������߷�����" << setprecision(10) << bestscore / No << endl;

	return 0;
}

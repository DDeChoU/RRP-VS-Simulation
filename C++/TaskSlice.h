#ifndef TASKSLICE_H
#define TASKSLICE_H
#include <chrono>
#include <sstream>
using namespace std::chrono;
using std::istringstream;
class TaskSlice
{
private:
	string job_id;
	string task_id;
	double slice_length;
	double wcet_left;
	bool is_hard_rt;
	bool is_on_time;
	system_clock::time_point accomplished_time;
	system_clock::time_point arb_ddl;
	string partition_id;

public:
	TaskSlice(string j_id, string t_id, double slice_l, double t_left, bool hard_rt, system_clock::time_point acc_time, system_clock::time_point ddl, string p_id);


	TaskSlice(string wrapped_info);

	string wrap_info();

	bool isOnTime() const{return is_on_time;}
	bool isHardRT() const{return is_hard_rt;}
	bool isDone() const{return wcet_left<=0.000001;}
	string getJobID() const{return job_id;}
	string getTaskID() const{return task_id;}
	system_clock::time_point getTaskSliceEnd() const{return accomplished_time;}
	double getTaskSliceLen() const{return slice_length;}
	double getTimeLeft() const{return wcet_left;}
	system_clock::time_point getDDL() const{return arb_ddl;}
	int getPhase() const;
	string getPartitionId(){return partition_id;}

};
TaskSlice::TaskSlice(string j_id, string t_id, double slice_l, double t_left, bool hard_rt, system_clock::time_point acc_time, system_clock::time_point ddl, string p_id)
{
	job_id = j_id;
	task_id = t_id;
	slice_length = slice_l;
	wcet_left = t_left;
	is_hard_rt = hard_rt;
	accomplished_time = acc_time;
	arb_ddl = ddl;
	if(ddl<accomplished_time)
		is_on_time = false;
	else
		is_on_time = true;
	partition_id = p_id;
}

TaskSlice::TaskSlice(string wrapped_info)
{
	istringstream ss(wrapped_info);
	string token;
	int counter = 0;
	while(std::getline(ss, token, ','))
	{
		counter ++;
		switch(counter)
		{
			case 1:
			{
				job_id = token;
				break;
			}
			case 2:
			{
				slice_length = std::stod(token);
				break;
			}
			case 3:
			{
				wcet_left = std::stod(token);
				break;
			}
			case 4:
			{
				istringstream temp(token);
				temp>>is_hard_rt;
				break;
			}
			case 5:
			{
				istringstream temp(token);
				temp>>is_on_time;
				break;
			}
			case 6:
			{
				long long msNum = std::stoll(token);
				milliseconds dur(msNum);
				accomplished_time += dur;
				break;
			}
			case 7:
			{
				long long msNum = std::stoll(token);
				milliseconds dur(msNum);
				arb_ddl += dur;
				break;
			}
			case 8:
			{
				task_id = token;
				break;
			}
			case 9:
			{
				partition_id = token;
				break;
			}
			default:
				break;
		}
	}
}
string TaskSlice::wrap_info()
{
	string result = "";
	result += job_id+",";
	result += std::to_string(slice_length)+",";
	result += std::to_string(wcet_left)+",";
	result += std::to_string(is_hard_rt)+",";
	result += std::to_string(is_on_time)+",";
	auto ms = duration_cast<milliseconds>(accomplished_time.time_since_epoch());
	result += std::to_string(ms.count())+",";
	ms = duration_cast<milliseconds>(arb_ddl.time_since_epoch());
	result += std::to_string(ms.count())+",";
	result += task_id+",";
	result +=partition_id+"\n";

	return result;

}

int TaskSlice::getPhase() const
{
	int index = job_id.find('-');
	string temp = job_id.substr(index+1);
	int result = atoi(temp.c_str());
	return result;
}

#endif
/*
* author: @Guangli Dai
* Date created: June 12, 2019
* Last modified: July 1st, 2019
*/
#ifndef TASK_H
#define TASK_H
#include <string>
using std::string;

class Task
{
private:
	double WCET;
	int period;
	int relative_ddl;
	double arbitrary_ddl;
	double arrival_time;
	bool is_periodic;
	string task_id;
	bool is_hard_rt;
	string partition_id;
	string domain_id;
public:
	//arguments:
	//w: Worst Case Execution Time
	//p: period
	//r_ddl: relative deadline
	//periodic: whether the task is a periodic task, true by default
	//hard_rt: whether the task is hard real time, false by default
	Task(double w, int p, int r_ddl, double arr_time, string t_id, bool periodic=true, bool hard_rt=false, string dom_id = "DOM#1")
	{
		WCET = w;
		period = p;
		relative_ddl = r_ddl;
		is_periodic = periodic;
		is_hard_rt = hard_rt;
		arrival_time = arr_time;
		task_id = t_id;
		domain_id = dom_id;// domain id is set to "DOM#1" by default 
	}

	double getWCET() const {return WCET;} 
	int getPeriod() const {return period;}
	int getRelativeDdl() const{return relative_ddl;}
	double getArrTime() const{return arrival_time;}
	bool isPeriodic() const{return is_periodic;}
	string getID() const{return task_id;}
	bool isHardRT() const{return is_hard_rt;}
	string getPartitionId() const{return partition_id;}
	void setPartitionId(string pid){partition_id = pid;}

	bool operator<(const Task &b) const
	{
		return arrival_time<b.arrival_time;
	}
	//for testing
	string printInfo()
	{
		string result = "";
		result += std::to_string(WCET)+","+std::to_string(period)+",";
		result += std::to_string(relative_ddl)+","+std::to_string(is_periodic)+",";
		result += std::to_string(is_hard_rt)+","+std::to_string(arrival_time)+",";
		result += task_id;
		return result;
	}

	
	

};


#endif
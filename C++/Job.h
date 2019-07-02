/*
*	author:@Guangli Dai
*	Created: June 12th, 2019
*	last modified: July 1st, 2019
*
*/


#ifndef JOB_H
#define JOB_H
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>
using std::string;
using std::stringstream;
using std::istringstream;
using namespace std::chrono;


class Job
{
private:
	double computation_time;
	system_clock::time_point arbitrary_ddl;
	bool is_hard_rt;
	string task_id;
	string job_id;
public:
	//constructor
	Job(double c, system_clock::time_point D, string t_id, int phase, bool hard_rt = false)
	{
		computation_time = c;
		arbitrary_ddl = D;
		is_hard_rt = hard_rt;
		task_id = t_id;
		job_id = task_id+"-"+std::to_string(phase);
	}

	//copy constructor
	Job(const Job &a)
	{
		computation_time = a.computation_time;
		arbitrary_ddl = a.arbitrary_ddl;
		is_hard_rt = a.is_hard_rt;
		task_id = a.task_id;
		job_id = a.job_id;
	}

	//construct a job object with a string produced by wrap_info
	Job(string wrapped_info)
	{
		istringstream ss(wrapped_info);

		string token;
		int counter = 0;
		while(std::getline(ss, token, ','))
		{
			counter++;
			switch(counter)
			{
				case 1:
				{
					job_id = token;
					break;
				}
				case 2:
				{
					computation_time = std::stod(token);
					break;
				}
				case 3:
				{
					//convert the millisecond to a time_point
					long long msNum = std::stoll(token);
					milliseconds dur(msNum);
					arbitrary_ddl += dur;
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
					task_id = token;
					break;
				}
				default:
				{
					break;
				}
			}
		}
	};

	//serialize all info of the object and construct a string
	string wrap_info()
	{
		//use stringstream to wrap everything up
		string result = "";
		result += job_id +","+std::to_string(computation_time)+",";

		//convert the time to a fractional timestamp
		//stringstream ss;
		time_t et = system_clock::to_time_t(arbitrary_ddl);
		const auto nowMS = duration_cast<milliseconds>(arbitrary_ddl.time_since_epoch());
		//ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
		result += std::to_string(nowMS.count())+",";
		result += std::to_string(is_hard_rt)+",";
		result += task_id+"\n";
		return result;


	}


	//serialize all info into string, but the time_point is shown in a more readable way.
	string print_info()
	{
		string result = "";
		result += job_id +", "+std::to_string(computation_time)+",";

		//convert the time to a fractional timestamp
		stringstream ss;
		time_t et = system_clock::to_time_t(arbitrary_ddl);
		const auto nowMS = duration_cast<milliseconds>(arbitrary_ddl.time_since_epoch())%1000;
		ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
		result+= ss.str()+",";
		result += std::to_string(is_hard_rt)+",";
		result += task_id+"\n";
		return result;
	}

	double getComputationTime(){return computation_time;}
	void setComputationTime(double c){computation_time = c;}
	system_clock::time_point getDDL(){return arbitrary_ddl;}
	bool isHardRT(){return is_hard_rt;}
	string getTaskId(){return task_id;}
	string getJobId(){return job_id;}
private:
	string printTP(system_clock::time_point time_now)
	{
		stringstream ss;
		time_t et = system_clock::to_time_t(arbitrary_ddl);
		const auto nowMS = duration_cast<milliseconds>(arbitrary_ddl.time_since_epoch())%1000;
		ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
		return ss.str();
	}

};

#endif
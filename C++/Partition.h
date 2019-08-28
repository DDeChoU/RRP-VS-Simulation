#ifndef PARTITION_H
#define PARTITION_H
#include <list>
#include <math.h>
#include "Task.h"
#include "Job.h"
#include "string"

using std::string;
using std::list;
class Partition
{
private:
	double availability_factor;
	int regularity;
	string partition_id;
	double aaf;
	int aaf_up;
	int aaf_down;
	double aaf_left;

	list<Job> job_queue;

public:
	//constructor takes in the availability factor and regularity. There is no way to change once it is set.
	Partition(double af, int reg, string id)
	{
		availability_factor = af;
		regularity = reg;
		partition_id = id;
		aaf = availability_factor;
	}
	Partition(const Partition &a)
	{
		availability_factor = a.availability_factor;
		regularity = a.regularity;
		partition_id = a.partition_id;
	}
	double getAF() const{return availability_factor;} 
	double getReg() const{return regularity;}
	string getID() const{return partition_id;}
	void setAAF(double aaf_in){aaf = ceil(aaf_in*1000)/(double)1000; aaf_left = aaf;}//keep 4-digits' precision
	void setAAFFrac(int a, int b){aaf_up = a; aaf_down = b; reduceAAF();}
	void setAAFLeft(double al){aaf_left = al;}
	int getAAFUp(){return aaf_up;}
	int getAAFDown(){return aaf_down;}
	double getAAF() const{return aaf;}
	double getAAFLeft() const{return aaf_left;}
	string printInfo()
	{
		string result = partition_id+":";
		result+= std::to_string(availability_factor)+","+std::to_string(regularity);
		return result;

	}

	//comparison function overloaded for the usage of priority_queue
	bool operator()(const Partition &a, const Partition &b) const
	{
		return a.availability_factor<b.availability_factor;
	}

	//return the job to be executed. Simply return the first one in the list.
	bool schedule(Job &j);

	//insert the one into the proper place so that the list is EDF. To be implemented.
	void insertJob(Job j);


private:
	void reduceAAF()
	{
		int a = aaf_down, b = aaf_up;
		while(b!=0)
		{
			int temp = b;
			b = a%b;
			a = temp;
		}
		aaf_down /= a;
		aaf_up /= a;
	}


};

bool Partition:: schedule(Job &j)
{
	if(job_queue.empty())
	{
		return false;
	}
	j = job_queue.front();
	job_queue.pop_front();
	return true;
}

void Partition::insertJob(Job j)
{
	//find the place to insert by using the arbitrary deadline.
	auto it = job_queue.begin();
	while(it!=job_queue.end())
	{
		if((*it).getDDL()>j.getDDL())
			break;
	}
	job_queue.insert(it, j);
}



#endif
#include <iostream>
#include <vector>
#include <list>
#include "Partition.h"
#include "PCPU.h"
#include "Scheduler.h"
//#include "Scheduler.h"
using std::vector;
using std::cout;
using std::endl;
using std::list;

int main()
{
	//test partition single
	// test passed.
	/*
	list<Partition *> p_s;
	Partition p1(0.42857, 1, "P1");
	p1.setAAFFrac(3,7);
	p_s.push_back(&p1);
	Partition p2(0.28571, 1, "P2");
	p2.setAAFFrac(2,7);
	p_s.push_back(&p2);
	Partition p3(0.28571, 1, "P3");
	p3.setAAFFrac(2,7);
	p_s.push_back(&p3);
	PCPU P_now("PCPU1");
	P_now.partition_single(p_s);
	*/

	/*
	list<Partition *> p_s;
	Partition p1(0.03571, 1, "P1");
	p1.setAAFFrac(1,28);
	p_s.push_back(&p1);
	Partition p2(0.0714, 1, "P2");
	p2.setAAFFrac(2,28);
	p_s.push_back(&p2);
	Partition p3(0.8571, 1, "P3");
	p3.setAAFFrac(24,28);
	p_s.push_back(&p3);
	PCPU P_now("PCPU1");
	P_now.partition_single(p_s);
	*/
	/*
	list<Partition *> p_s;
	Partition p1(0.03571, 1, "P1");
	p1.setAAFFrac(1,28);
	p_s.push_back(&p1);
	Partition p2(0.0714, 1, "P2");
	p2.setAAFFrac(2,28);
	p_s.push_back(&p2);
	Partition p3(0.8571, 1, "P3");
	p3.setAAFFrac(24,28);
	p_s.push_back(&p3);
	PCPU P_now("PCPU1");
	P_now.partition_single(p_s);
	*/
	/* test passed
	list<Partition *> p_s;
	Partition p1(0.0714, 1, "P1");
	p1.setAAFFrac(2,28);
	p_s.push_back(&p1);
	Partition p2(0.0714, 1, "P2");
	p2.setAAFFrac(2,28);
	p_s.push_back(&p2);
	Partition p3(0.035714, 1, "P3");
	p3.setAAFFrac(1,28);
	p_s.push_back(&p3);
	PCPU P_now("PCPU1");
	P_now.partition_single(p_s);
	cout<<P_now.showTTable();
	*/
	// test MulZ
	Scheduler s(4);
	vector<Partition> partition_list;
	Partition p1(0.3, 1, "P1");
	Partition p2(0.65, 1, "P2");
	Partition p3(0.6, 1, "P3");
	Partition p4(0.35, 1, "P4");
	Partition p5(0.5, 1, "P5");
	Partition p6(0.55, 1, "P6");
	Partition p7(0.25, 1, "P7");
	Partition p8(0.25, 1, "P8");
	Partition p9(0.25, 1, "P9");
	partition_list.push_back(p1);
	partition_list.push_back(p2);
	partition_list.push_back(p3);
	partition_list.push_back(p4);
	partition_list.push_back(p5);
	partition_list.push_back(p6);
	partition_list.push_back(p7);
	partition_list.push_back(p8);
	partition_list.push_back(p9);
	vector<Task> task_list;
	s.set_partitions(partition_list);
	s.run(task_list, cout,1, 1000);

	return 0;
}
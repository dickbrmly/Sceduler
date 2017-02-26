//*****************************************************************************
//          Richard Bromley 5001075854 
//          CS370 Project #3
//
// Purpose: This program is designed to simulate the operating of a operating
//          system scheduler.  
//
// Input: contained in program
// 		       
//
// Output:   list of prioritized tasks
//*****************************************************************************
#include "process.h"


int main()
{
	double averageTAT = 0;
	double averageWT = 0;
	double averageCPU = 0;
	double count = 0;

	bool fin = false;
	int pid = 0;
	string entry;

	cpuQue *inprocess = new cpuQue;
	startUpQue *start = new startUpQue;

	activeExpiredQue *activeQue = new activeExpiredQue;
	activeExpiredQue *expiredQue = new activeExpiredQue;

	ioQue *io = new ioQue;
	finishedQue *finished = new finishedQue;
	activeExpiredQue *temp;

	process *program, *program2;
	

	getline(cin, entry);
	// load processes
	while (entry != "***")
	{
		start->loadProcess(pid, entry);
		++pid;
		getline(cin, entry);
	}

	while (!fin)
	{
		if (start->arrival())
		{
			program = start->unloadProcess();
			activeQue->loadProcess(program);

			program->timeSlice = (int)((1 - (program->priority / 150.0)) * 375.0 + 0.5) + 5;

			cout << "[" << ticker << "] <" << program->pid << "> Enters Ready Queue (Priority: " << program->priority << ", TimeSlice: " << program->timeSlice << ") \n";

		}
		if (inprocess->empty())
		{
			program = activeQue->unloadProcess();
			if (program != NULL)
			{
				cout << "[" << ticker << "] <" << program->pid << "> Enters the CPU \n";
				inprocess->loadProcess(program);
			}
		}

		if (activeQue->empty() && !expiredQue->empty()) //swap ques
		{
			cout << "[" << ticker << "] " << "*** Queue Swap" << "\n";
				temp = activeQue;
				activeQue = expiredQue;
				expiredQue = temp;
		}
		switch (inprocess->decrementProcess())
		{
			case 1:
				program = inprocess->unloadProcess();
				cout << "[" << ticker << "] <" << program->pid << "> Moves to the IO Queue \n";
				io->loadProcess(program); // TO: IO process
				break;
			case 2:
				program = inprocess->unloadProcess();
				cout << "[" << ticker << "] <" << program->pid << "> Finishes and moves to the Finished Queue \n";
				finished->loadProcess(program); // To: Finished
				break;
			case 3:
				program = inprocess->unloadProcess();
				cout << "[" << ticker << "] <" << program->pid << "> Moves to the Expired Queue \n";
				program->timeSlice = (int)((1 - (program->priority / 150.0)) * 375.0 + 0.5) + 5;
				expiredQue->loadProcess(program); // To: Finished
				break;
			default:
				if (inprocess->priority() <= activeQue->priority()) break;
				else program = inprocess->unloadProcess();
				if (program != NULL)
				{
					activeQue->loadProcess(program); // To: Active  
					cout << "[" << ticker << "] <" << program->pid << "> Enters Ready Queue (Priority: " << program->priority << ", TimeSlice: " << program->timeSlice << ") \n";
				}
				break;
		}
		io->decrementProcesses();
		while (io->completed())
		{
			program = io->unloadProcess();
			if (program->cpuBurst.top() == 0)
			{
				program->cpuBurst.pop();
				expiredQue->loadProcess(program);
				cout << "[" << ticker << "] <" << program->pid << "> Finishes IO and moves to the Expired Queue \n";
			}
			else activeQue->loadProcess(program);
			cout << "[" << ticker << "] <" << program->pid << "> Finishes IO and moves to the Ready Queue \n";
		}
		fin = start->empty() && activeQue->empty() && expiredQue->empty() && io->empty() && inprocess->empty();
	}
	while (!finished->empty())
	{
		count += 1;
		program = finished->unloadProcess();
		cout << " \n \n <" << program->pid << "> completed \n" << "Turn around time (TAT) =" << program->finished - program->arrival
			<< " \n" << "Total CPU time (TCT) = " << program->totalCPU2 << " \n"
			<< "Waiting time (WT) = " << program->totalIO2 << " \n"
			<< "Percentage of CPU utilization time (CUT) = " << program->totalCPU2 / (program->finished - program->arrival)
			<< " \n \n";
		averageTAT += (double) (program->finished - program->arrival);
		averageCPU += program->totalCPU2;
		averageWT += program->totalIO2;
		delete program;
	}
		cout << "Average Waiting Time: " << (averageWT / count) << "\n" << "Average Turnaround Time: " << (averageTAT / count) << " \n"
			 << "Average CPU Utilization: " << (averageCPU / count) << "\n";
	return 0;
}
//*****************************************************************************
//
//*****************************************************************************

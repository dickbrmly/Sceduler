//*****************************************************************************
//          Richard Bromley 5001075854 
//          CS370 Project #3
//
// Purpose: This program is designed to simulate the action of a operating
//          system scheduler. Any number of ques can be created for
//			threaded processing in the future (note).
//
// Que: startUp
// functions:
// 		       
//
// Output:   list of prioritized tasks
//*****************************************************************************
#ifndef bromley_process
#define bromley_process

#include<sstream>
#include<iostream>
#include<string>
#include<stack>

using namespace std;

//global varables
int ticker; //for simulation
//*****************************************************************************
//                              Single Process 
//*****************************************************************************
    struct process //The program control block PCB
    {    
        public:
        int pid,  niceValue, arrival, priority, bonus, timeSlice, 
            totalCPU, totalIO, finished;
		double totalCPU2, totalIO2;

        stack <int> cpuBurst;
        stack <int> ioBurst;
        
        process *child; //linked list next item

		process() { child = NULL; totalCPU2 = 0; totalIO2 = 0; }
    };    
//*****************************************************************************
//                 StartUp Que Assending by Arrival time //Ready Que
//*****************************************************************************
    class startUpQue  //The submit que....
    {
		private:
		process *start, *entry, *temp;
		stack <int> sequencer;

		public: 
			startUpQue() { entry = new process; start = NULL; }
//*****************************************************************************
        void loadProcess(int processNumber, string str)
        {
            int Bursts,value;

            entry->pid = processNumber;
            stringstream stng(str);
            
            stng >> entry->niceValue >> entry->arrival;
            stng >> Bursts;
            
            entry->bonus = 0;
            entry->finished = 0;
            
            entry->priority = (int) ((entry->niceValue + 20.0) / 39.0 
            * 30.0 + 0.5) + 105.0;
            
            while(Bursts > 1)
            {
                stng >> value; entry->cpuBurst.push(value);
				entry->totalCPU2 += (double) value;
                stng >> value; entry->ioBurst.push(value);
				entry->totalIO2 += (double) value;
                Bursts--;
            }
            stng >> value; entry->cpuBurst.push(value);
			entry->totalCPU2 += (double)value;

			// load CPU bursts
            while(!entry->cpuBurst.empty()) { sequencer.push(
				entry->cpuBurst.top()); entry->cpuBurst.pop();
			}
            while(!sequencer.empty()) { entry->cpuBurst.push(sequencer.top()); 
            sequencer.pop(); }

			//load IO bursts 
			while(!entry->ioBurst.empty()) { sequencer.push(
            entry->ioBurst.top()); entry->ioBurst.pop(); }
            while(!sequencer.empty()) { entry->ioBurst.push(sequencer.top()); 
            sequencer.pop(); }
 
			// process Order = Lowest start Time
            if(start == NULL)
            {
				start = entry;
                entry = new process;
                return;
            }
			else if (entry->arrival < start->arrival)
			{
				entry->child = start;
				start = entry;
				entry = new process;
				return;
			}

			temp = start;

			while(temp->child != NULL)
            {
                if (entry->arrival < temp->child->arrival)
                {
					entry->child = temp->child;
					temp = entry->child;
                    temp->child = entry;
					entry = new process;
                }
                temp = temp->child;
            }
			temp->child = entry;
        }
//*****************************************************************************
        bool empty() { return start == NULL; }
//*****************************************************************************
		bool arrival(void) 
		{
			if (start == NULL) return false;
			if (start->arrival <= ticker) return true;
			else return false;
		}
//*****************************************************************************
		process* unloadProcess(void)
		{
			process *temp;
			if (start == NULL) return NULL;
			temp = start;
			start = start->child;
			return temp;
		}
    };
//*****************************************************************************
//                         Active Expired Que
//*****************************************************************************
    class activeExpiredQue //part of Ready Que
    {
		private:
		bool loaded;
		process *start, *temp;

		public: 
			activeExpiredQue() { start = NULL;  temp = NULL; }
//*****************************************************************************
        void loadProcess(process *pointer)
        { 
			if (pointer == NULL) return;
			else pointer->child = NULL;

			if (pointer->totalCPU < pointer->totalIO)
				pointer->priority = (int)(pointer->priority + (((1.0 - ((double)pointer->totalCPU / (double)pointer->totalIO) * -5.0) - 0.5)));
			else pointer->priority = (int)(pointer->priority + (((1.0 - ((double)pointer->totalCPU / (double)pointer->totalIO) * 5.0) + 0.5)));

			if (start == NULL) 
            {
                start = pointer;
				start->child = NULL;
                return;
            }	
			if (pointer->priority < start->priority)
			{
				pointer->child = start;
				start = pointer;
				return;
			}

			temp = start;
			
			while(temp->child != NULL)
			{
				if (pointer->priority < temp->child->priority)
				{
					pointer->child = temp->child;
					temp->child = pointer;
					return;
				}
				temp = temp->child;
            }
			temp->child = pointer;
        }
//*****************************************************************************
		int priority() 
		{ 
			if (start == NULL) return 1000;
			else return start->priority; 
		}
//*****************************************************************************
        bool empty() { return start == NULL; } //startup
//*****************************************************************************
		process* unloadProcess(void)
		{
			if(start == NULL || start->arrival > ticker) return NULL;
			temp = start;
			start = start->child;
			return temp;
		}
//*****************************************************************************
    };
//*****************************************************************************
//                                 IO Que
//*****************************************************************************
    class ioQue //Short Term Scheduler (Short Term Que)
    {
		private:
		process *start, *comparison;

		public: 
        ioQue () { start = NULL; } 
//*****************************************************************************
        void loadProcess(process *pointer) //if true process completed
        {
			if (pointer == NULL) return;
			else pointer->child = NULL;

            comparison = start;
 
			if (start == NULL) 
            {
                start = pointer;
				start->child = NULL;
                return;
            }
            else if(pointer->ioBurst.top() < comparison->ioBurst.top())
            {
				pointer->child = comparison;
				start = pointer;
				return;
            }
             
            while(comparison->child != NULL)
            {
                if (pointer->ioBurst.top() < comparison->child->ioBurst.top())
                {
					pointer->child = comparison->child;
					comparison->child = pointer;
                }
                comparison = comparison->child;
            }
            comparison->child = pointer;
        }
//*****************************************************************************
        bool empty() { return start == NULL; }
//*****************************************************************************
//*****************************************************************************
		bool completed() 
		{ 
			if (start == NULL) return false;
			return start->ioBurst.top() == 0;
		}
//*****************************************************************************
		process* unloadProcess(void)
		{
			process *temp;
			if (start == NULL) return NULL;
			start->ioBurst.pop();
			temp = start;
			start = start->child;
			return temp;
		}
//*****************************************************************************
		void decrementProcesses(void)
		{
			comparison = start;
			while (comparison != NULL)
			{
				++comparison->totalIO;
				comparison->ioBurst.top() -= 1;
				comparison = comparison->child;
			}
		}
//*****************************************************************************
    };
//*****************************************************************************
//                              Finished Que
//*****************************************************************************
	class finishedQue
	{
	private:
		process *start, *last;

	public:
		finishedQue() { start = NULL;  last = NULL; }
//*****************************************************************************
		void loadProcess(process *pointer)
		{
			if (pointer == NULL) return;
			if (start == NULL)
			{
				start = pointer;
				last = pointer;
			}
			last->child = pointer;
			pointer->finished = ticker;
			last = pointer;
		}
//*****************************************************************************
		process* unloadProcess(void)
		{
			process *temp;
			if (start == NULL) return NULL;
			temp = start;
			start = start->child;
			return temp;
		}
//*****************************************************************************
        bool empty() { return start == NULL; }
//*****************************************************************************
	};
//*****************************************************************************
//                          Process Being Executed
//*****************************************************************************
    class cpuQue
	{
		private:
			process *running;
        public: 
			cpuQue() { running = NULL; }
//*****************************************************************************
		void loadProcess(process *pointer)
		{
			if (pointer == NULL) return;
			else if (pointer->cpuBurst.empty()) return;
			if (empty())
			{
				pointer->child = NULL;
				running = pointer;
			}
		}
//*****************************************************************************
		int priority() 
		{ 
			if (running == NULL) return 1000;
			else return running->priority; 
		}
//*****************************************************************************
		bool empty() { return running == NULL; }
//*****************************************************************************
		int decrementProcess(void) //returns 0 if more time
								   // 1 for completed CPU cycle
		{						   // 2 for completed process
			++ticker;
			if (running == NULL) return 0;
			if (running->timeSlice == 0)
			{
				return 3;
			}
			if (running->cpuBurst.top() == 0)
			{	
				running->cpuBurst.pop();
				if (running->cpuBurst.empty()) return 2;
				else return 1;
			}
			++running->totalCPU;
			--running->timeSlice;
			--running->cpuBurst.top();
			return 0;
		}
//*****************************************************************************
		process* unloadProcess(void)
		{
			process *temp;

			temp = running;
			running = NULL;

			return temp;
		}
//*****************************************************************************
	};
//*****************************************************************************
#endif

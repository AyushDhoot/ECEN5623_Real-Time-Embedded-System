#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TRUE 1
#define FALSE 0
#define u32_t unsigned int

/*========= STATIC DATA DECLARATIONS =========*/

// U=0.7333
static u32_t ex0_period[] = {2, 10, 15};
static u32_t ex0_wcet[] = {1, 1, 2};

// U=0.9857
static u32_t ex1_period[] = {2, 5, 7};
static u32_t ex1_wcet[] = {1, 1, 2};

// U=0.9967
static u32_t ex2_period[] = {2, 5, 7, 13};
static u32_t ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
static u32_t ex3_period[] = {3, 5, 15};
static u32_t ex3_wcet[] = {1, 2, 3};

// U=1.0
static u32_t ex4_period[] = {2, 4, 16};
static u32_t ex4_wcet[] = {1, 1, 4};

//==============================//
//			Added Stuff			//
//==============================//

// U=1.0
static u32_t ex5_period[] = {2, 5, 10};
static u32_t ex5_wcet[] = {1, 2, 1};

// U=0.9967
static u32_t ex6_period[] = {2, 5, 7, 13};
static u32_t ex6_wcet[] = {1, 1, 1, 2};

// U=1.0
static u32_t ex7_period[] = {3, 5, 15};
static u32_t ex7_wcet[] = {1, 2, 4};

// U=.9967
static u32_t ex8_period[] = {2, 5, 7, 13};
static u32_t ex8_wcet[] = {1, 1, 1, 2};

// U=1.0
static u32_t ex9_period[] = {6, 8, 12, 24};
static u32_t ex9_wcet[] = {1, 2, 4, 6};

/*========= TYPE DECLARATIONS =========*/

typedef struct
{
	u32_t C;
	u32_t T;
	u32_t C_rem;
	u32_t T_next;
	u32_t prio;
} task_t;

typedef enum {EDF, LLF, RM}sched_t;

/*========= FUNCTION DECLARATIONS =========*/

// Runs the completion time test on the provided service set
// This code was provided as part of the example code
u32_t completion_time_feasibility(u32_t numServices, u32_t period[], u32_t wcet[], u32_t deadline[]);

// Runs the scheduling point test on the provided service set
// This code was provided as part of the example code
u32_t scheduling_point_feasibility(u32_t numServices, u32_t period[], u32_t wcet[], u32_t deadline[]);

// Wrapper function that runs all tests on a provided set (comp time, sched point, and RM LLF and EDF over the LCM)
// Also prints the results of each test
void run_tests(u32_t ex_num, u32_t num_services, u32_t* ex_period, u32_t* ex_wcet);

// Simulates the execution of the task set over the least common multiple of periods, using the specified scheduler
// If print_sched is true, it also prints the schedule (which task is executed for every time slice)
u32_t test_schedule_over_lcm(u32_t num_services, u32_t* ex_period, u32_t* ex_wcet, bool print_sched, sched_t scheduler);

// Finds the least common multiple of a set of numbers
u32_t lcm_set(u32_t num_services, u32_t* ex_period);

// Finds the least common multiple of a pair of numbers (used in lcm_set)
// Not original work, see function definition for credit link
u32_t lcm_pair(u32_t a, u32_t b);

// Finds the greatest common divisor of a pair of numbers (used in lcm_pair)
// Not original work, see function definition for credit link
u32_t gcd(u32_t a, u32_t b);


/*========= MAIN =========*/

int main(void)
{
	// Below code shows an example of printing the schedule simulation, commented out for full test suite
	// test_schedule_over_lcm(sizeof(ex3_period)/sizeof(u32_t), ex3_period, ex3_wcet, true, LLF);
	// exit(0);

	u32_t numServices;
    
    printf("*************Feasibility Tests*************\n");
	printf("Using Completion Time and Scheduling Point\n\n");

	// Run the test for ever set of tasks
	numServices = sizeof(ex0_period)/sizeof(u32_t);
	run_tests(0, numServices, ex0_period, ex0_wcet);

	numServices = sizeof(ex1_period)/sizeof(u32_t);
	run_tests(1, numServices, ex1_period, ex1_wcet);
	
	numServices = sizeof(ex2_period)/sizeof(u32_t);
	run_tests(2, numServices, ex2_period, ex2_wcet);

	numServices = sizeof(ex3_period)/sizeof(u32_t);
	run_tests(3, numServices, ex3_period, ex3_wcet);

	numServices = sizeof(ex4_period)/sizeof(u32_t);
	run_tests(4, numServices, ex4_period, ex4_wcet);

	numServices = sizeof(ex5_period)/sizeof(u32_t);
	run_tests(5, numServices, ex5_period, ex5_wcet);

	numServices = sizeof(ex6_period)/sizeof(u32_t);
	run_tests(6, numServices, ex6_period, ex6_wcet);

	numServices = sizeof(ex7_period)/sizeof(u32_t);
	run_tests(7, numServices, ex7_period, ex7_wcet);

	numServices = sizeof(ex8_period)/sizeof(u32_t);
	run_tests(8, numServices, ex8_period, ex8_wcet);

	numServices = sizeof(ex9_period)/sizeof(u32_t);
	run_tests(9, numServices, ex9_period, ex9_wcet);

	printf("\n");
	printf("TEST COMPLETE \n");

	exit(0);
}


/*========= FUNCTION DEFINITIONS =========*/


// Wrapper function that runs all tests on a provided set (comp time, sched point, and RM LLF and EDF over the LCM)
// Also prints the results of each test
void run_tests(u32_t ex_num, u32_t num_services, u32_t* ex_period, u32_t* ex_wcet)
{
	printf("EX-%d with services:\n", ex_num);

	u32_t index = 0;
	double utilization = 0;
	for(index = 0; index < num_services; index++)
	{
		// Add up the utilization
		utilization += (ex_wcet[index]*1.0)/(ex_period[index]*1.0);
		// Print out the service WCET and Period
		printf("\tC%d = %d, T%d = %d\n", index+1, ex_wcet[index], index+1, ex_period[index]);
	}

	printf("Utilization = %4.3f\n", utilization);

    if(completion_time_feasibility(num_services, ex_period, ex_wcet, ex_period) == TRUE)
    {
		printf("Completion Time = FEASIBLE\n");
	}
	else
    {
		printf("Completion Time = INFEASIBLE\n");
	}

	if(scheduling_point_feasibility(num_services, ex_period, ex_wcet, ex_period) == TRUE)
    {
		printf("Scheduling Point = FEASIBLE\n");
	}
    else
	{
        printf("Scheduling Point = INFEASIBLE\n");
	}

	if(test_schedule_over_lcm(num_services, ex_period, ex_wcet, false, RM))
	{
		printf("RM  over LCM = FEASIBLE\n");
	}
	else
	{
		printf("RM  over LCM = INFEASIBLE\n");
	}

	if(test_schedule_over_lcm(num_services, ex_period, ex_wcet, false, LLF))
	{
		printf("LLF over LCM = FEASIBLE\n");
	}
	else
	{
		printf("LLF over LCM = INFEASIBLE\n");
	}

	if(test_schedule_over_lcm(num_services, ex_period, ex_wcet, false, EDF))
	{
		printf("EDF over LCM = FEASIBLE\n");
	}
	else
	{
		printf("EDF over LCM = INFEASIBLE\n");
	}
	printf("\n");
}

// Runs the completion time test on the provided service set
// This code was provided as part of the example code
u32_t completion_time_feasibility(u32_t numServices, u32_t period[], u32_t wcet[], u32_t deadline[])
{
  u32_t i, j;
  u32_t an, anext;
  
  // assume feasible until we find otherwise
  u32_t set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
  
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;
       
       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}

// Runs the scheduling point test on the provided service set
// This code was provided as part of the example code
u32_t scheduling_point_feasibility(u32_t numServices, u32_t period[], u32_t wcet[], u32_t deadline[])
{
   u32_t rc = TRUE, i, j, k, l, status, temp;

   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      for (k=0; k<=i; k++) 
      {
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

               if (temp <= (l*period[k]))
			   {
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }
      if (!status) rc=FALSE;
   }
   return rc;
}

// Simulates the execution of the task set over the least common multiple of periods, using the specified scheduler
// If print_sched is true, it also prints the schedule (which task is executed for every time slice)
u32_t test_schedule_over_lcm(u32_t num_services, u32_t* ex_period, u32_t* ex_wcet, bool print_sched, sched_t scheduler)
{
	// Current time in simulation
	u32_t sim_time = 0;
	// Allocate structures to hold the task set
	task_t* tasks = (task_t*)malloc(sizeof(task_t)*num_services);

	// Calc LCM for Simulation time (LCM is end of simulation)
	u32_t lcm = lcm_set(num_services, ex_period);

	// Fill the  Task structs
	u32_t i = 0;
	for(i = 0; i < num_services; i++)
	{
		tasks[i].C = ex_wcet[i];
		tasks[i].C_rem = ex_wcet[i];
		tasks[i].T = ex_period[i];
		tasks[i].T_next = ex_period[i];
	}

	// Print Header for the Test
	if(print_sched == true)
	{
		switch(scheduler)
		{
			case EDF:
				printf("Scheduler is EDF\n");
			break;

			case LLF:
				printf("Scheduler is LLF\n");
			break;

			case RM:
				printf("Scheduler is RM\n");
			break;
			
			default:
				printf("Unknown Scheduler\nFailure\n");
			return 0;
		}

		for(i = 0; i < num_services; i++)
		{
			printf("Task %d with T=%d, and C=%d\n", i, tasks[i].T, tasks[i].C);
		}

		printf("\n");
		for(i = 0; i < num_services; i++)
		{
			printf("%d",i);
		}

		printf("\n");
	}

	// Run test
	task_t* max_prio_task = NULL;	// This holds a pointer to the task that has the highest priority this time slice

	// Loop through all sim time
	for(sim_time = 0; sim_time < lcm; sim_time++)
	{
		// At each time slice, loop through all the tasks
		// Calc priorities, choose what runs, run it
		for(i = 0; i < num_services; i++)
		{
			// For all tasks that still need to run
			if(tasks[i].C_rem > 0)
			{
				// If this is the first run through the task loop, reset the max_priority task pointer
				if(max_prio_task == NULL)
				{
					max_prio_task = &tasks[i];
				}

				// Calc Priority
				switch(scheduler)
				{
					case EDF:
						tasks[i].prio = tasks[i].T_next;
					break;

					case LLF:
						tasks[i].prio = tasks[i].T_next - sim_time - tasks[i].C_rem;
					break;

					case RM:
						tasks[i].prio = tasks[i].T;
					break;
					
					default:
						printf("Unknown Scheduler\nFailure\n");
					return 0;
				}

				// If Prio < Curmax Prio (lower is better)
				if(tasks[i].prio < max_prio_task->prio)
				{
					// Set this task as highest priority
					max_prio_task = &tasks[i];
				}
			} // End for all tasks that still need to run
		} // End for all tasks loop

		// Print the running schedule
		if(print_sched == true)
		{
			// Loop through all services and print whichever one has been selected
			// as the "max_prio_task". If none selected, (idle slice) print all dashes
			for(i = 0; i < num_services; i++)
			{
				if(i == (max_prio_task - tasks))
				{
					printf("x");
				}
				else
				{
					printf("-");
				}
			}
			printf("\n");
		}

		// If task is not null, something needs to run (not idle slice)
		if(max_prio_task != NULL)
		{
			max_prio_task->C_rem--;		// "Run" the highest priority task
			max_prio_task = NULL;		// Reset the max_prio to null
		}

		// Loop through all the services and check for deadline failures
		// Reset things that have passed deadlines
		for(i = 0; i < num_services; i++)
		{
			// Check for services that are ready to run again (deadline has passed)
			if(tasks[i].T_next == sim_time+1)
			{
				// Check if they still need to run (they missed their deadline)
				if(tasks[i].C_rem != 0)
				{
					return 0;	// Missed a deadline, return failure
				}
				else
				{
					// Ready to run again, reset computation time and deadline
					tasks[i].C_rem = tasks[i].C;
					tasks[i].T_next += tasks[i].T;
				}
			}
		}
	} // End loop through all simulation time

	if(print_sched == true)
	{
		printf("\n");
	}

	return 1;	// If we got this far without a failure, the schedule is feasible
}

// Finds the least common multiple of a set of numbers
u32_t lcm_set(u32_t num_services, u32_t* ex_period)
{
	u32_t* i = ex_period;
	u32_t lcm = 0;

	lcm = lcm_pair(ex_period[0], ex_period[1]);

	for(i = (ex_period + 2); i<(ex_period + num_services); i++)
	{
		lcm = lcm_pair(lcm, *i);
	}
	
	return lcm;
}

// Finds the least common multiple of a pair of numbers (used in lcm_set)
u32_t lcm_pair(u32_t a, u32_t b)
{
	/* LCM of a pair of numbers from: */
	/* http://www.programming-algorithms.net/article/42865/Least-common-multiple */
	return (a*b)/gcd(a,b);
}

// Finds the greatest common divisor of a pair of numbers (used in lcm_pair)
u32_t gcd(u32_t a, u32_t b)
{
	/* GCD function from: */
	/* http://www.programming-algorithms.net/article/42865/Least-common-multiple */
	u32_t r = 0;

	do
	{
		r = a % b;
		a = b;
		b = r;
	} while (b != 0);
	return a;
}

/* FreeRTOS includes. */
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOSConfig.h"
#include "mid_cli.h" 
#include "hal_cli.h" 

build_var(info, "Device information.", 0);
build_var(date, "Display current time.", 0);
#if ( configUSE_TRACE_FACILITY == 1 )
build_var(top, "List all the tasks state.", 0);
#endif

static void app_cli_register(void)
{
	mid_cli_register(&info);
	mid_cli_register(&date);
	mid_cli_register(&top);
}

cmd_handle(info)
{
	unsigned char index;
	
	const static char *dev_info[] =
	{
		" 	Dev:	Intel(R) Core(TM)\r\n",
		" 	Cpu:	i5-7200U CPU\r\n",
		" 	Freq: 	2.50GHz 2.71GHz\r\n",
		" 	Mem:	256GB SSD\r\n",
		" 	Ram:	16.0GB(in total) 15.9GB(available)\r\n",
		NULL
	};

	(void) help_info;
	(void) argv;
	configASSERT(dest);

	for(index = 0; dev_info[index] != NULL; index++)
	{
		strcat(dest, dev_info[index]);
	}

	return pdFALSE;
}

cmd_handle(date)
{
	time_t nowtime;
	struct tm *timeinfo;

	(void) help_info;
	(void) argv;
	configASSERT(dest);

	time( &nowtime );
	timeinfo = localtime( &nowtime );
	sprintf(dest, "    Current time: %d-%d-%d %d %02d:%02d:%02d\n", 
		timeinfo->tm_year + 1900, 
		timeinfo->tm_mon + 1, 
		timeinfo->tm_mday, 
		timeinfo->tm_wday, 
		timeinfo->tm_hour, 
		timeinfo->tm_min, 
		timeinfo->tm_sec);

	return pdFALSE;
}

//static BaseType_t top_main( char *dest, argv_attribute argv, const char * const help_info)
#if (configGENERATE_RUN_TIME_STATS == 1)
cmd_handle(top)
{
	/* �������������е�����������ʵ��������������ֵʱ�����ʾΪ MAX_TASKS������������ */
	#define MAX_TASKS 	(20)	
	
	static unsigned char curr_task = 0;
	static TaskStatus_t ptasks[MAX_TASKS];
	TaskStatus_t *p;
	unsigned int run_time;
	unsigned char num_of_tasks = uxTaskGetNumberOfTasks();
	char temp_str[30];
	
	(void) help_info;
	configASSERT(dest);

	if(ptasks == NULL)
		return pdFALSE;
	vTaskDelay(10);
	
	if(curr_task == 0)
	{
		if(num_of_tasks > MAX_TASKS)
		{
			num_of_tasks = MAX_TASKS;
			sprintf(dest, "Warning: real num(%d) of tasks more than defines(%d)!\r\n", num_of_tasks, MAX_TASKS);
		}
		uxTaskGetSystemState(ptasks, num_of_tasks, NULL);
		sprintf(dest, "        PRI     STATE   MEM(W)  %%TIME   NAME\r\n");
	}
	p = &ptasks[curr_task];
	sprintf(temp_str, "\t%d\t", p->uxCurrentPriority);
	strcat(dest, temp_str);
	switch(p->eCurrentState)
	{
		case eRunning: strcat(dest, "run"); break;
		case eReady: strcat(dest, "ready"); break;
		case eBlocked: strcat(dest, "block"); break;
		case eSuspended: strcat(dest, "suspend"); break;
		case eDeleted: strcat(dest, "deleted"); break;
		case eInvalid: strcat(dest, "invalid"); break;
		default: strcat(dest, "null"); break;
	}
	run_time = p->ulRunTimeCounter / portGET_RUN_TIME_COUNTER_VALUE() * 1000;
	sprintf(temp_str, "\t%d\t%c%d\t", p->usStackHighWaterMark, run_time < 10 ? '<' : ' ', run_time < 10 ? 1 : run_time / 10);
	strcat(dest, temp_str);
	strcat(dest, p->pcTaskName);
	strcat(dest, "\r\n");
	curr_task ++;
	if(curr_task == num_of_tasks)
	{
		curr_task = 0;
		return pdFALSE;
	}
	else
		return pdTRUE;
}
#endif

void app_cli_init(unsigned char priority, char *t, TaskHandle_t *handle)
{
	mid_cli_init(400, priority, t, handle);

	app_cli_register();
}


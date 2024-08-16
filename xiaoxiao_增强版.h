#ifndef _XIAO_XIAO_H
#define _XIAO_XIAO_H

//如有疑问或指正，请电话或WX 13631645567

#define MAXTASKS 5

#ifdef SYS_PARAMETER
volatile unsigned char timers[MAXTASKS];
volatile unsigned char bdata wake_up_flag;
sbit wake_task_0 = wake_up_flag ^ 0;
sbit wake_task_1 = wake_up_flag ^ 1;
sbit wake_task_2 = wake_up_flag ^ 2;
sbit wake_task_3 = wake_up_flag ^ 3;
sbit wake_task_4 = wake_up_flag ^ 4;
#else
extern unsigned char timers[];
extern unsigned char bdata wake_up_flag;
extern bit wake_task_0;
extern bit wake_task_1;
extern bit wake_task_2;
extern bit wake_task_3;
extern bit wake_task_4;
#endif


//...
#define IS_WAKED_UP(N, M) if(wake_task_##M==0) \
						{ \
							switch(STATE){ STATES##N;} \
						} \
						else \
						{STATE=0x0; wake_task_##M=0;}
//...
#define _SS_W(N, TaskID) static unsigned char STATE=0; \
						IS_WAKED_UP(N, TaskID)

#define _SS(N) static unsigned char STATE=0; switch(STATE){ STATES##N;}

#define _EE    ; STATE=0; return 255;

#define WaitX(A,tickets)  do {STATE=A; return tickets ;} while(0); ST(A):

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0);
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }

#define CallSubX(A,SubTaskName) do {unsigned char currdt; STATE=A; return 0; ST(A):  currdt=SubTaskName(); if(currdt!=255) return currdt;} while(0);
#define UpdateTimers() {unsigned char i; for(i=0;i<MAXTASKS;i++){if((timers[i]!=0)&&(timers[i]!=255)) timers[i]--;}}//...

#define SUSPEND_TASK_A(TaskID) timers[TaskID]=0xff	//使其它任务休眠
#define SUSPEND_TASK_B() return 255		//使本任务休眠
#define SUSPEND_TASK_C() {STATE=A; return 0xff;}//使本任务休眠,唤醒后接着运行

#define SET_WAKE_UP_FLAG(N) wake_task_##N=1

#define WAKEUP_TASK(TaskID) \
		timers[TaskID]=0x0; \
		SET_WAKE_UP_FLAG(TaskID)

#define RESTART_TASK(TaskID) \
		timers[TaskID]=0x0; \
		SET_WAKE_UP_FLAG(TaskID)


#define IS_RUNNING_TASK(TaskID) ((timers[TaskID] != 0xff) ? 1 : 0)

#define EXIT   do {STATE=0; return 255;} while(0);
#define RESTART  do {STATE=0; return 0;} while(0);

#define ST(A) ST##_##A
#define GO(A)  case A: goto ST(A);

#define STATES1  GO(1);
#define STATES2  GO(1);GO(2);
#define STATES3  GO(1);GO(2);GO(3);
#define STATES4  GO(1);GO(2);GO(3);GO(4);
#define STATES5  GO(1);GO(2);GO(3);GO(4);GO(5);
#define STATES6  GO(1);GO(2);GO(3);GO(4);GO(5);GO(6);
#define STATES45 GO(1);GO(2);GO(3);GO(4);GO(5);GO(6);GO(7);GO(8);GO(9);GO(10);GO(11);GO(12);GO(13);GO(14);GO(15);GO(16);GO(17);GO(18);GO(19);GO(20);GO(21);GO(22);GO(23);GO(24);GO(25);GO(26);GO(27);GO(28);GO(29);GO(30);GO(31);GO(32);GO(33);GO(34);GO(35);GO(36);GO(37);GO(38);GO(39);GO(40);GO(41);GO(42);GO(43);GO(44);GO(45);


//#define SEM unsigned int
//#define InitSem(sem) sem=0;
//#define WaitSem(A,sem) do{ sem=1; WaitX(A,0); if (sem>0) return 1;} while(0);
//#define WaitSemX(A,sem,tickets)  do { sem=tickets+1; WaitX(A,0); if(sem>1){ sem--;  return 1;} } while(0);
//#define SendSem(sem)  do {sem=0;} while(0);

#if USE_SEM
#define SEM_TASK_NUM	2
uint8_t sem_1_task[SEM_TASK_NUM];//定义一个数组,元素个数是使用此信号量的任务个数
#define WaitSem(A,n,TaskID) do{STATE=A; sem_task[n]=TaskID;} while(0);
#define SendSem(sem) do{timers[sem_1_task[0]]=0x0; timers[sem_1_task[1]]=0x0;} while(0);
#endif

#endif
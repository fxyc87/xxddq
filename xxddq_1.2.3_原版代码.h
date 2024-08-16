#ifndef __xxddq_H
#define __xxddq_H
/*
 * @Project      : 小小调度器问星整理版
 * @Descripttion : 可设置选择8/16/32版本，在原来的基础上进一步增强
 * @version      : 1.2.3 整合版
 * @Author       : smset
 * @Date         : 2015-06-02 09:02:31
 * @LastEditors  : 问星
 * @LastEditTime : 2020-11-06 19:48:20

 * 2020-11-06
 * 1、增加 #define WAITUTILTRTYPE 0 	//WaitUtilTR 超时等待判断类型选择宏
 *    0=简单,WaitUtilTR(A,TimeOut,TINDEX) TINDEX:超时等待索引只能0-7,每个WaitUtilTR的索引不能重复，只支持8个实时性高的超时判断;
 *	  1=灵活 自定义变量，WaitUtilTR(A,TimeOut,T1FLAY) T1FLAY:1个调度器时钟周期标志变量,需在调度器时钟周期定时器中断里将变量置1。
 *   简单调用示例：WaitUtilTR(f.b0,2,0);
 *   
 * 2020-11-05
 * 1、根据群主（smset）的意见，修改“_count”赋初值的位置，使之理合理
 * 2、修改“WaitUtilTR(A,TimeOut)、WAITUNTILTR(A,VAL,TimeOut)”定义，解决实时性高的判断等待超时时间不准的问题
 *    增加“T1FLAY”一个调度器周期标志变量参数，变为“WaitUtilTR(A,TimeOut,T1FLAY)、WAITUNTILTR(A,VAL,TimeOut,T1FLAY)”
 *    使用方式请参考例子
	//定义一个调度器周期标志联合体变量（经测试联合体包含结构体应用在宏里，C51编译器编译会有BUG）
	//用于实时性高的带超时判断的等待判断，
	//在更新调度器的定时器中断里加入 “T1F.bs=0xFF;”
	//如果超过8个需实时性高的判断，请修改“T1F.bs”为16位并添加n个“unsigned bn: 1;” 定义
	union
	{
	    struct
	    {
	        unsigned b0: 1;
	        unsigned b1: 1;
	        unsigned b2: 1;
	        unsigned b3: 1;
	        unsigned b4: 1;
	        unsigned b5: 1;
	        unsigned b6: 1;
	        unsigned b7: 1; 
	    } b;
	    unsigned char bs;
	} T1F;  //一个调度器周期标志
	
 	//任务0
	TASK task0(void)
	{
	    _SS
	    while(1)
	    {
	        //高实时性的等待变量f.b0==1，超时2个调度器周期
	        //f.b0:等待的条件变量;
	        //2:超时2个调度器时钟周期;
	        //T1F.b.b0:当前超时调度器时钟周期标志位，不可重复。
	        //下一个请传入“T1F.b.b1”，以此类推，
	        WaitUtilTR(f.b0, 2,T1F.b.b0); //高实时性的等待变量f.b0==1，超时2个调度器周期
	        if (f.b0 == 0) //超时执行
	        {
	            LED0 ^= 1;  //翻转LED0
	        }
	        else    //符合条件执行
	        {
	            LED1 ^= 1;
	        }
	    }
	    _EE
	}
	//定时器0中断
	void INTT0(void) interrupt 1 using 1
	{
	    //10ms
	    TL0 = 0Xff;
	    TH0 = 0XDB;
	    UpdateTimers(); //更新任务
	    T1F.bs=0xFF;    //将一个调度器时钟周期标志变量位全置1 增加
	    // RunTask(task0, 0); //高级别任务，在中断里执行，可剥夺低级别任务。
	}
 *
 * 2020-11-04
 * 1、修复超时判断只能生效1次的BUG（群友"上善若水"发现）
 *    在WaitUtilT(A,TimeOut)、WAITUNTILT(A,VAL,TimeOut)、WaitUtilTR(A,TimeOut)、WAITUNTILTR(A,VAL,TimeOut)定义里增加“_count=TimeOut;”
 *
 * 2020-10-15
 * 1、整合SETJMP版.可通过定义SETJMP切换
 *    #define SETJMP 0            //版本选择:0=SWINCH版 1=SETJMP版

 * 2020-10-14
 * 1、增加LCBIT宏定义来设置_lc变量类型.
 *	  #define LCBIT 0			//选择_lc的变量类型: 0=8位;1=16位.
 *	  PICC高版本编译器宏不支持__LINE__%255运算,LCBIT定义需设置为1。代价就是每个任务多占一个字节的RAM.
 *    其它编译器如果不报错，建议 LCBIT 设置为0,和原来一样.
 *
 * 2020-07-03
 * 1、增加 普通函数无阻塞延时开关宏定义，方便在不使用普通函数无阻塞延时功能时减少RAM占用和更新任务的时间：
 *     NORMALDELAY  默认为0关闭，修改为1则打开
 *
 * 2020-04-17
 *  1、同步smset的小小调度器1.11（16位定时器版）,新增的等待某个条件或超时溢出
 *      WaitUtilT(A,TimeOut)
 *  2、在smset的WaitUtilT(A,TimeOut)基础上,增加等待某个条件为某个值或超时溢出
 *      WAITUNTILT(A,VAL,TimeOut)
 *
 * 2019-05-30
 * 1、增加等待某个条件为某个值或超时溢出
 *     A条件变量;VAL条件值;TaskVAL超时值，不能为0，和TaskVAL的位数调度器定义的位数一样;ONLY超时判断唯一序号
 *     WAITUNTILO(A,VAL,TaskVAL,ONLY)
 *     每增加一个超时判断会增加一个调度器位数对应内存占用
 * 2、添加 void runTasks(void); 声明，确保部分编译器不会报错
 *
 * 2019-05-29
 *  1、整合群主semet的普通函数无阻塞延时
 *  2、添加变量位数类型定义，方便根据编译器环境定义修改
 *  注意：无阻塞延时 只能被主任务，以及主任务调用的函数，及主任务调用的函数调用的函数使用
 */

/****小小调度器开始********************************************/
//可根据编译器环境修改定义变量位数类型，如果出现重复定义错误提示，可以注释掉变量类型定义
#define U8 unsigned char	//8位变量类型定义
#define U16 unsigned int	//16位变量类型定义
#define U32 unsigned long	//32位变量类型定义

#define MAXTASKS 5         //定义调度器主任务数：1-255
#define BITS 8             //定义调度器位数可选值：8/16/32
#define SETJMP 0            //版本选择:0=SWINCH版 1=SETJMP版
#define NORMALDELAY 0       //普通函数无阻塞延时开关：0=关；1=开
#define WAITUTILTRTYPE 0 	//WaitUtilTR 超时等待判断类型： 
							//0=简单,WaitUtilTR(A,TimeOut,TINDEX) TINDEX:超时等待索引只能0-7,每个WaitUtilTR的索引不能重复;
							//1=灵活自定义变量，WaitUtilTR(A,TimeOut,T1FLAY) T1FLAY:1个调度器时钟周期标志变量,需在调度器时钟周期定时器中断里将变量置1。
#define SEM U16    			//信号量定义宏

#define LCBIT 0			//选择_lc的变量类型: 0=8位;1=16位.SETJMP版此项无效

#if (BITS==8)
#define TASK U8
#define TICKET_MAX 0xFF
#endif

#if (BITS==16)
#define TASK U16
#define TICKET_MAX 0xFFFF
#endif

#if (BITS==32)
#define TASK U32
#define TICKET_MAX 0xFFFFFFFF
#endif

volatile TASK timers[MAXTASKS];

#if (NORMALDELAY==1)
volatile TASK delaycount;
void runTasks(void);
#endif
#if (WAITUTILTRTYPE==0)
U8 T1FS=0;
#endif
//**********     SWINCH版-开始    **********************************//
#if (SETJMP==0)
//任务头
#if (LCBIT==0)
#define _SS static U8 _lc=0; switch(_lc){default:
#endif
#if (LCBIT==1)
#define _SS static U16 _lc=0; switch(_lc){default:
#endif
//任务尾
#define _EE ;}; _lc=0; return TICKET_MAX;
//等待X个时钟周期
#if (LCBIT==0)
#define WaitX(tickets)  do { _lc=(__LINE__&255)+1; return (tickets) ;case (__LINE__&255)+1:;} while(0);
#endif
#if (LCBIT==1)
#define WaitX(tickets)  do { _lc=__LINE__; return (tickets) ;case __LINE__:;} while(0);
#endif
//等待某个条件
#define WaitUntil(A)    do { while(!(A)) WaitX(1);} while(0);
//只能用在RunTask调用的任务里 实时性更高 等待某个条件
#define WaitUntilR(A)    do { while(!(A)) WaitX(0);} while(0);
//等待某个条件为某个值	A条件变量;VAL条件值
#define WAITUNTIL(A,VAL)  do { WaitX(1);} while((A)!=VAL);
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值	A条件变量;VAL条件值
#define WAITUNTILR(A,VAL)  do { WaitX(0);} while((A)!=VAL);
//等待某个条件或超时溢出
#define WaitUtilT(A,TimeOut)  do {static TASK _count; _count=(TimeOut); do { WaitX(1); _count--; } while((!(A))&&(_count>0));} while(0);
#if (WAITUTILTRTYPE==1)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件或超时溢出 A条件变量;TimeOut超时值;T1FLAY一个调度器周期标志变量 使用方法参考例子
#define WaitUtilTR(A,TimeOut,T1FLAY)  do {static TASK _count; _count=(TimeOut); do { WaitX(0); if(T1FLAY){_count--;T1FLAY=0;}} while((!(A))&&(_count>0));} while(0);
#endif
#if (WAITUTILTRTYPE==0)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件或超时溢出 A条件变量;TimeOut超时值;TINDEX实时判断索引，取值范围 0-7，每个判断的索引不能重复，最多支持8个实时判断
#define WaitUtilTR(A,TimeOut,TINDEX)  do {static TASK _count; _count=(TimeOut); do { WaitX(0); if(T1FS&(1<<(TINDEX))){_count--;T1FS&=~(1<<(TINDEX));}} while((!(A))&&(_count>0));} while(0);
#endif
//等待某个条件为某个值或超时溢出
#define WAITUNTILT(A,VAL,TimeOut)  do {static TASK _count; _count=(TimeOut); do { WaitX(1); _count--; } while(((A)!=VAL)&&(_count>0));} while(0);
#if (WAITUTILTRTYPE==1)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值或超时溢出 A条件变量;VAL条件值;TimeOut超时值;T1FLAY一个调度器周期标志变量 使用方法参考例子
#define WAITUNTILTR(A,VAL,TimeOut,T1FLAY)  do {static TASK _count; _count=(TimeOut); do { WaitX(0); if(T1FLAY){_count--;T1FLAY=0;}} while(((A)!=VAL)&&(_count>0));} while(0);
#endif
#if (WAITUTILTRTYPE==0)
// 只能用在RunTask调用的任务里 实时性更高 等待某个条件或超时溢出 A条件变量;VAL条件值;TimeOut超时值;TINDEX实时判断索引，取值范围 0-7，每个判断的索引不能重复，最多支持8个实时判断
#define WAITUNTILTR(A,VAL,TimeOut,TINDEX)  do {static TASK _count; _count=(TimeOut); do { WaitX(0); if(T1FS&(1<<(TINDEX))){_count--;T1FS&=~(1<<(TINDEX));}} while(((A)!=VAL)&&(_count>0));} while(0);
#endif
//运行任务
#define RunTask(TaskName,TaskID)  do { if (timers[TaskID]==0) {TASK d=TaskName(); while(timers[TaskID]!=d) timers[TaskID]=d;} } while(0);
//运行任务，前面的任务优先保证执行
#define RunTaskA(TaskName,TaskID) do { if (timers[TaskID]==0) {TASK d=TaskName(); while(timers[TaskID]!=d) timers[TaskID]=d;continue;} } while(0);
//调用子任务
#if (LCBIT==0)
#define CallSub(SubTaskName) do {TASK currdt; _lc=(__LINE__%255)+1; return 0; case (__LINE__%255)+1:  currdt=SubTaskName(); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
#if (LCBIT==1)
#define CallSub(SubTaskName) do {TASK currdt; _lc=__LINE__; return 0; case __LINE__:  currdt=SubTaskName(); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
//调用子任务,传入一个参数,传入的参数需在释放CPU前使用
#if (LCBIT==0)
#define CallSub1(SubTaskName,p1) do {TASK currdt; _lc=(__LINE__%255)+1; return 0; case (__LINE__%255)+1:  currdt=SubTaskName(p1); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
#if (LCBIT==1)
#define CallSub1(SubTaskName,p1) do {TASK currdt; _lc=__LINE__; return 0; case __LINE__:  currdt=SubTaskName(p1); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
//调用子任务,传入二个参数,传入的参数需在释放CPU前使用
#if (LCBIT==0)
#define CallSub2(SubTaskName,p1,p2) do {TASK currdt; _lc=(__LINE__%255)+1; return 0; case (__LINE__%255)+1:  currdt=SubTaskName(p1,p2); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
#if (LCBIT==1)
#define CallSub2(SubTaskName,p1,p2) do {TASK currdt; _lc=__LINE__; return 0; case __LINE__:  currdt=SubTaskName(p1,p2); if(currdt!=TICKET_MAX) return currdt;} while(0);
#endif
//初始化任务调度
#define InitTasks() do {U8 i; for(i=MAXTASKS;i>0 ;i--) timers[i-1]=0; } while(0);

#if (WAITUTILTRTYPE==0)
#define ST1FS (T1FS=0xFF)
#endif
#if (WAITUTILTRTYPE==1)
#define ST1FS 
#endif

#if (NORMALDELAY==0)
//更新任务
#define UpdateTimers() do{U8 i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=TICKET_MAX)) timers[i-1]--;} ST1FS;} while(0);
#endif
#if (NORMALDELAY==1)
//更新任务
#define UpdateTimers() do{U8 i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=TICKET_MAX)) timers[i-1]--;} if(delaycount>0) delaycount--; ST1FS;} while(0);
//普通函数无阻塞延时 只能被主任务，以及主任务调用的函数，及主任务调用的函数调用的函数 使用
#define DELAYX(tickets) do{delaycount=tickets;while(delaycount>0) {runTasks();}}while(0);
#endif

#endif
//**********     SWINCH版-结束    **********************************//

//**********     SETJMP版-开始    **********************************//
#if (SETJMP==1)
#include <setjmp.h>
//任务头
#define _SS static jmp_buf env; static TASK started; if(started) longjmp(env,1); started=1;
//任务尾
#define _EE started=0; return TICKET_MAX;
//等待X个调度器周期
#define WaitX(tickets) do { if(setjmp(env)==0) return tickets;} while(0);
//等待某个条件
#define WaitUntil(A) do { WaitX(1);} while((A)==0);
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值	A条件变量
#define WaitUntilR(A) do { WaitX(0);} while((A)==0);
//等待某个条件为某个值  A条件变量;VAL条件值
#define WAITUNTIL(A,VAL)  do { WaitX(1);} while((A)!=VAL);
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值	A条件变量;VAL条件值
#define WAITUNTILR(A,VAL) do { WaitX(0);} while((A)!=VAL);
//等待某个条件或超时溢出
#define WaitUtilT(A,TimeOut)  do {static TASK _count; _count=(TimeOut); do { WaitX(1); _count--; } while((!(A))&&(_count>0));} while(0);
#if (WAITUTILTRTYPE==1)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值或超时溢出 A条件变量;TimeOut超时值;T1FLAY一个调度器周期标志变量 使用方法参考例子
#define WaitUtilTR(A,TimeOut,T1FLAY) do {static TASK _count; _count=(TimeOut); do { WaitX(0);if(T1FLAY){_count--;T1FLAY=0;}} while((!(A))&&(_count>0));} while(0);
#endif
#if (WAITUTILTRTYPE==0)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件或超时溢出 A条件变量;TimeOut超时值;TINDEX实时判断索引，取值范围 0-7，每个判断的索引不能重复，最多支持8个实时判断
#define WaitUtilTR(A,TimeOut,TINDEX) do {static TASK _count; _count=(TimeOut); do { WaitX(0);if(T1FS&(1<<(TINDEX))){_count--;T1FS&=~(1<<(TINDEX));}} while((!(A))&&(_count>0));} while(0);
#endif
//等待某个条件为某个值或超时溢出
#define WAITUNTILT(A,VAL,TimeOut)  do {static TASK _count; _count=(TimeOut); do { WaitX(1); _count--; } while(((A)!=VAL)&&(_count>0));} while(0);
#if (WAITUTILTRTYPE==1)
//只能用在RunTask调用的任务里 实时性更高 等待某个条件为某个值或超时溢出 A条件变量;VAL条件值;TimeOut超时值;T1FLAY一个调度器周期标志变量 使用方法参考例子
#define WAITUNTILTR(A,VAL,TimeOut,T1FLAY) do {static TASK _count; _count=(TimeOut); do { WaitX(0);if(T1FLAY){_count--;T1FLAY=0;}} while(((A)!=VAL)&&(_count>0));} while(0);
#endif
#if (WAITUTILTRTYPE==0)
// 只能用在RunTask调用的任务里 实时性更高 等待某个条件或超时溢出 A条件变量;VAL条件值;TimeOut超时值;TINDEX实时判断索引，取值范围 0-7，每个判断的索引不能重复，最多支持8个实时判断
#define WAITUNTILTR(A,VAL,TimeOut,TINDEX) do {static TASK _count; _count=(TimeOut); do { WaitX(0);if(T1FS&(1<<(TINDEX))){_count--;T1FS&=~(1<<(TINDEX));}} while(((A)!=VAL)&&(_count>0));} while(0);
#endif
//运行任务
#define RunTask(TaskName,TaskID)  do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0);
//运行任务，前面的任务优先保证执行
#define RunTaskA(TaskName,TaskID) do { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;}}while(0);   //前面的任务优先保证执行
//调用子任务
#define CallSub(SubTaskName) do {TASK currdt; WaitX(0);  currdt=SubTaskName(); if(currdt!=TICKET_MAX) return currdt;} while(0);
//调用子任务
#define CallSub1(SubTaskName,p1) do {TASK currdt; WaitX(0);  currdt=SubTaskName(p1); if(currdt!=TICKET_MAX) return currdt;} while(0);
//调用子任务
#define CallSub2(SubTaskName,p1,p2) do {TASK currdt; WaitX(0);  currdt=SubTaskName(p1,p2); if(currdt!=TICKET_MAX) return currdt;} while(0);
//初始化任务调度
#define InitTasks() do {U8 i; for(i=MAXTASKS;i>0 ;i--) timers[i-1]=0; } while(0);

#if (WAITUTILTRTYPE==0)
#define ST1FS T1FS=0xFF
#endif
#if (WAITUTILTRTYPE==1)
#define ST1FS 
#endif

//更新任务
#if (NORMALDELAY==0)
#define UpdateTimers() do{U8 i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=TICKET_MAX)) timers[i-1]--;}ST1FS;} while(0);
#endif
#if (NORMALDELAY==1)
//更新任务
#define UpdateTimers() do{U8 i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=TICKET_MAX)) timers[i-1]--;} if(delaycount>0) delaycount--;ST1FS;} while(0);
//普通函数无阻塞延时 只能被主任务，以及主任务调用的函数，及主任务调用的函数调用的函数 使用
#define DELAYX(tickets) do{delaycount=tickets;while(delaycount>0) {runTasks();}}while(0);
#endif

#endif
//**********     SETJMP版-结束   **********************************//

//初始化信号量
#define InitSem(sem) do{sem=0;}while(0);
//等待信号量
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//发送信号量
#define SendSem(sem)  do {sem=0;} while(0);
//等待信号量或定时器溢出
#define WaitSemX(sem, tickets) do{sem=tickets+1;WaitX(0);if(sem>1){sem--;return 1;}}while (0);

/*****小小调度器结束*******************************************************/
#endif
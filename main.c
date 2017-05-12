#include"LPC11xx.h"

#define COM4_ON LPC_GPIO2->DATA &= ~(1<<8)       //第一位数码管点亮
#define  COM3_ON LPC_GPIO2->DATA &= ~(1<<9)       //第二位数码管点亮
#define COM2_ON LPC_GPIO2->DATA &= ~(1<<10)      //第三位数码管点亮
#define COM1_ON LPC_GPIO2->DATA &= ~(1<<11)      //第四位数码管点亮
#define ALL_OFF LPC_GPIO2->DATA |= (1<<8) + (1<<9) + (1<<10) + (1<<11)         //数码管全灭

uint16_t table[10] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};         //数码管显示码表
uint8_t DispBuffer[4];          //显示缓冲区
uint8_t time[2];                //分，秒计数单元
uint32_t timeCounter;           //中断次数计数单元
uint8_t time_1s_ok;             //1S时间到标志位

void DisplayScan()              //数码管动态扫描刷新程序，该函数需要被周期性的调用推荐在定时中断内调用
{
	static uint16_t com;        //扫描计数变量
	com++;                      //每次调用后切换一次显示
	if(com >= 4)                  //com的值在0.1.2.3之间切换
		com = 0;
	ALL_OFF ;                   //切换之前将全部显示暂时关闭，避免虚影
	switch(com)
	{
		case 0:
            LPC_GPIO2->DATA &= ~0xff;
            LPC_GPIO2->DATA |= DispBuffer[0];       //显示第一位			
            COM1_ON;
		    break;
		case 1:            
            LPC_GPIO2->DATA &= ~0xff;
            LPC_GPIO2->DATA |= DispBuffer[1];       //显示第二位
		    COM2_ON;           
		    break;
		case 2:
            LPC_GPIO2->DATA &= ~0xff;
            LPC_GPIO2->DATA |= DispBuffer[2];       //显示第三位
		    COM3_ON;            
		    break;
		case 3:
            LPC_GPIO2->DATA &= ~0xff;
            LPC_GPIO2->DATA |= DispBuffer[3];       //显示第四位
		    COM4_ON;           
		    break;
	}
}

void TimeToDisplayBuffer(void)              //将时间数据送人显示缓冲区
{
	uint8_t i ,j = 0;
	for(i = 0 ;i < 2 ;i++)
	{
		DispBuffer[j++] = table[time[i] % 10];          //取代显示时间数据的个位断码
		DispBuffer[j++] = table[time[i] / 10];          //取代显示时间数据的十位断码
	}
}

void TIMER32_0_IRQHandler(void)                //32位定时计数器0中断服务函数
{
	DisplayScan();
	if(++timeCounter >= 500)              //1秒定时时间到
	{
		timeCounter = 0;                  //秒计数器清零，开始下一秒计数
		time_1s_ok = 1;                   //1秒时间到标识位置1
	}
	LPC_TMR32B0->IR = 0x01;               //向匹配通道0写1清除中断
}	

void Timer0Init(void)                   //32位定时计数器0初始化函数    //允许中断，同时定时时间为1/500，即2ms
{
   LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);          //使能TIM32B0时钟
	 LPC_TMR32B0->IR = 0x01;                 //MR0中断复位，即清中断（bit0：MR0 ,bit1;MR1 ,bit2;MR2 ,bit3;MR3 ,bit4;MR4）
	 LPC_TMR32B0->PR = 0;                        //每一个PCLK加1
	 LPC_TMR32B0->MCR = 0x03;                     //MR0匹配时复位TC，并产生中断
	 LPC_TMR32B0->MR0 = SystemCoreClock/500;          //匹配时间，也就是定时时间，2ms
	 LPC_TMR32B0->TCR = 0x01;                         //启动定时器;TCR[0] = 1；
	NVIC_EnableIRQ(TIMER_32_0_IRQn);                //使能中断
}

int main()
{
    LPC_GPIO2->DIR |= 0xFFF;      //设置P2_0 ～ P2_11为输出
    Timer0Init();
    time[1]=00;
    time[0]=0;
    TimeToDisplayBuffer();
   while(1)
   { 
		 if(time_1s_ok == 1)
		 {  
			 time_1s_ok = 0;
			 if(++time[0] >= 60)
			 {
				 time[0] = 0;
				 if(++time[1] >= 60)
					 time[1] = 0;
			 }
			 TimeToDisplayBuffer();
		 }
	 }		 
}



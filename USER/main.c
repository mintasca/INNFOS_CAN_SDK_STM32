/**
  ******************************************************************************
  * @文	件 ： main.c
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.1
  * @日	期 ： 2019.9.10
  * @摘	要 ： 主程序入口
  ******************************************************************************/
  
/* Includes ----------------------------------------------------------------------*/
#include "bsp.h"
#include "SCA_API.h"
#include "SCA_APP.h"

/* Variable defines --------------------------------------------------------------*/
uint8_t cmd = 0;					//外部控制命令

/* Forward Declaration -----------------------------------------------------------*/
static void Log(void);
static void CMD_Handler(uint8_t cmd);

/**
  * @功	能	主程序入口
  * @参	数	无
  * @返	回	无
  */
int main(void)
{ 
	/* 底层驱动初始化 */
	BSP_Init();			

	/* 等待执行器稳定 */
	delay_ms(500);	
	
	/* 串口1打印LOG信息 */
	Log();
	
	/* 等待命令传入 */
	while(1)
	{	
		if(cmd)
		{
			CMD_Handler(cmd);
			cmd = 0;
		}
		else
			delay_ms(10);
	}       
}

/**
  * @功	能	串口打印提示信息
  * @参	数	无
  * @返	回	无
  */
static void Log()
{
	printf("\r\n欢迎使用 INNFOS SCA 驱动测试！\r\n");
	printf("详细通信协议参见 INNFOS WIKI ！\r\n");
	printf("发送 1 轮询总线上的执行器ID ！\r\n");
	printf("发送 2 使用默认ID初始化SCA控制器 ！\r\n");
	printf("发送 3 进入位置归零测试程序 ！\r\n");
	printf("发送 4 进入正反转测试程序 ！\r\n");
	printf("发送 5 进入高低速测试程序 ！\r\n");
	printf("发送 6 将执行器关机 ！\r\n");
	printf("发送 7 显示帮助信息 ！\r\n");
}

/**
  * @功	能	串口命令处理函数
  * @参	数	cmd：接收到的指令
  * @返	回	无
  */
static void CMD_Handler(uint8_t cmd)
{
	switch(cmd)
	{
		case 1:
			printf("\r\n执行轮询程序！\r\n");
		
			/* 调用轮询程序 */
			SCA_Lookup();
		
			printf("轮询结束！\r\n");
		break;
		
		case 2:
			printf("\r\nSCA初始化！\r\n");
		
			/* 调用初始化程序 */
			SCA_Init();
		
			/* 等待执行器稳定 */
			delay_ms(500);
		
			printf("SCA初始化结束！\r\n");
			break;
		
		case 3:
			printf("\r\n进入位置归零测试！\r\n");
		
			/* 调用测试程序 位置归零 */
			SCA_Homing();
		
			printf("位置归零测试结束！\r\n");
			break;
			
		case 4:
			printf("\r\n进入正反转切换测试！\r\n");
		
			/* 调用测试程序 正反转切换 */
			SCA_Exp1();
		
			printf("正反转切换测试结束！\r\n");
			break;
		
		case 5:
			printf("\r\n进入高低速切换测试！\r\n");
			
			/* 调用测试程序 高低速切换 */
			SCA_Exp2();
		
			printf("高低速切换测试结束！\r\n");
			break;
		
		case 6:
			printf("\r\n执行器关机！\r\n");
			
			/* 关闭所有执行器 */
			disableAllActuators();
		
			printf("执行器关机结束！\r\n");
			break;
		
		default:
			Log();
			break;
	}
	
}









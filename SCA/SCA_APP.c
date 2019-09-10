/**
  ******************************************************************************
  * @文	件 ： SCA_APP.c
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.1
  * @日	期 ： 2019.09.10
  * @摘	要 ： SCA 测试程序
  ******************************************************************************/ 
/* Update log --------------------------------------------------------------------*/
//V1.1.0 2019.08.05 测试程序修改为最新API接口
//V1.5.0 2019.08.20 测试程序修改为最新API接口，更改初始化方式
//V1.5.1 2019.09.10 增加轮询功能

/* Includes ----------------------------------------------------------------------*/
#include "bsp.h"
#include "SCA_APP.h"
#include "SCA_API.h"

/* Variable defines --------------------------------------------------------------*/
SCA_Handler_t* pSCA_ID1 = NULL;		//读写指针，可用于获取执行器参数或调用FAST型函数
SCA_Handler_t* pSCA_ID2 = NULL;		

/* CAN端口信息定义，用于绑定SCA句柄，实现多端口控制。移植时根据实际数量定义，默认使用2个 */
CAN_Handler_t CAN_Port1,CAN_Port2;

/* Funcation defines -------------------------------------------------------------*/

/**
  * @功	能	在CAN总线上查找存在的SCA，并打印找到的ID
  * @参	数	无
  * @返	回	无
  * @注	意	每台执行器都有自己的ID，若初次使用不知道
  *			对应的ID，可用此函数查找。此功能需要开SCA_DEBUGER
  */
void SCA_Lookup()
{
	/* 初始化CAN端口参数 */
	CAN_Port1.CanPort = 1;			//标记端口号
	CAN_Port1.Retry = 2;			//失败重发次数
	CAN_Port1.Send = CAN1_Send_Msg;	//CAN1端口发送函数
	
	CAN_Port2.CanPort = 2;			
	CAN_Port2.Retry = 2;			
	CAN_Port2.Send = CAN2_Send_Msg;	//CAN2端口发送函数
	
	/* 调用函数查找对应总线上存在的ID */
	lookupActuators(&CAN_Port1);	//轮询CAN1总线
	lookupActuators(&CAN_Port2);	//轮询CAN2总线
}

/**
  * @功	能	控制器初始化
  * @参	数	无
  * @返	回	无
  */
void SCA_Init()
{
	/* 初始化CAN端口参数 */
	CAN_Port1.CanPort = 1;			//标记端口号
	CAN_Port1.Retry = 2;			//失败重发次数
	CAN_Port1.Send = CAN1_Send_Msg;	//CAN1端口发送函数
	
	CAN_Port2.CanPort = 2;			
	CAN_Port2.Retry = 2;			
	CAN_Port2.Send = CAN2_Send_Msg;	//CAN2端口发送函数
	
	/* 装载执行器的ID与所使用的CAN端口号 */
	setupActuators( 1, &CAN_Port1);	//ID1 绑定CAN1
	setupActuators( 2, &CAN_Port2);	//ID2 绑定CAN2
	
	/* 获取ID1和2的参数句柄 */
	pSCA_ID1 = getInstance(1);
	pSCA_ID2 = getInstance(2);
	
	/* 启动所有执行器 */
	enableAllActuators();
}

/**
  * @功	能	位置归零
  * @参	数	无
  * @返	回	无
  */
void SCA_Homing()
{
	/* 未开机直接退出 */
	if(isEnable(0x01) == Actr_Disable)	return;
	if(isEnable(0x02) == Actr_Disable)	return;
	
	/* 切换执行器操作模式到梯形位置模式 */
	activateActuatorMode(0x01,SCA_Profile_Position_Mode,Block);
	activateActuatorMode(0x02,SCA_Profile_Position_Mode,Block);
	
	/* 归零 1号执行器 */
	setPosition(0x01,0);
	
	/* 等待归零成功 */
	do
	{
		getPosition(0x01,Unblock);
		delay_ms(100);
	}
	while((pSCA_ID1->Position_Real > 0.1f)||(pSCA_ID1->Position_Real < -0.1f));
	
	/* 归零 2号执行器*/
	setPosition(0x02,0);
	
	/* 等待归零成功 */
	do
	{
		getPosition(0x02,Unblock);
		delay_ms(100);
	}
	while((pSCA_ID2->Position_Real > 0.1f)||(pSCA_ID2->Position_Real < -0.1f));
}

/**
  * @功	能	正反转切换两次
  * @参	数	无
  * @返	回	无
  */
void SCA_Exp1()
{
	/* 未开机直接退出 */
	if(isEnable(0x01) == Actr_Disable)	return;
	if(isEnable(0x02) == Actr_Disable)	return;
	
	/* 归零 */
	SCA_Homing();
	
	/* 开启正反转 */
	setPosition(0x01,2);			//普通函数以ID调用 0x01号执行器
	setPositionFast(pSCA_ID2,2);	//FAST型函数以指针形式调用0x02号执行器
	delay_ms(1000);
	
	setPosition(0x01,0);
	setPositionFast(pSCA_ID2,0);
	delay_ms(1000);
	
	setPosition(0x01,2);
	setPositionFast(pSCA_ID2,2);
	delay_ms(1000);
	
	setPosition(0x01,0);
	setPositionFast(pSCA_ID2,0);
	delay_ms(1000);
} 

/**
  * @功	能	高低速切换
  * @参	数	无
  * @返	回	无
  */
void SCA_Exp2()
{
	/* 未开机直接退出 */
	if(isEnable(0x01) == Actr_Disable)	return;
	if(isEnable(0x02) == Actr_Disable)	return;
	
	/* 切换执行器操作模式到梯形速度模式 */
	activateActuatorMode(0x01,SCA_Profile_Velocity_Mode,Block);
	activateActuatorMode(0x02,SCA_Profile_Velocity_Mode,Block);
	
	/* 开启正反转 */
	setVelocity(0x01,300);
	setVelocityFast(pSCA_ID2,300);
	delay_ms(1000);
	
	setVelocity(0x01,600);
	setVelocityFast(pSCA_ID2,600);
	delay_ms(1000);
	
	setVelocity(0x01,300);
	setVelocityFast(pSCA_ID2,300);
	delay_ms(1000);
	
	setVelocity(0x01,600);
	setVelocityFast(pSCA_ID2,600);
	delay_ms(1000);
	
	/* 停止 */
	setVelocity(0x01,0);
	setVelocityFast(pSCA_ID2,0);
}




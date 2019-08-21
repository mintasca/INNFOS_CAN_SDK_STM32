/**
  ******************************************************************************
  * @文件名	： bsp_can.c
  * @作者	： INNFOS Software Team
  * @版本	： V1.0.0
  * @日期	： 2019.6.14
  * @摘要	： CAN底层源文件
  ******************************************************************************/

/* Includes ----------------------------------------------------------------------*/
#include "bsp_can.h"
#include "SCA_API.h"

/* Funcation defines -------------------------------------------------------------*/

/**
  * @功	能	CAN1端口初始化
  * @参	数	无
  * @返	回	无
  */
void CAN1_Initializes(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	CAN_InitTypeDef		CAN_InitStructure;
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	
#if CAN1_INT_EN
	NVIC_InitTypeDef	NVIC_InitStructure;
#endif

	/* 使能时钟 */
	RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(CAN1_GPIO_CLK, ENABLE);			

	/* 引脚配置 */
	GPIO_InitStructure.GPIO_Pin = CAN1_RX_PIN | CAN1_TX_PIN;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	/* 引脚复用映射配置 */
	GPIO_PinAFConfig(CAN1_GPIO_PORT,GPIO_PinSource11,GPIO_AF_CAN1); //GPIOA11复用为CAN1
	GPIO_PinAFConfig(CAN1_GPIO_PORT,GPIO_PinSource12,GPIO_AF_CAN1); //GPIOA12复用为CAN1

	/* 配置CAN参数 */
	CAN_InitStructure.CAN_TTCM = DISABLE;	/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;	/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;	/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = ENABLE;	/* 使能仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;	/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;	/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* 设置CAN模式 */

	/* 配置CAN波特率 = 1MBps(45MHz/9/(1+2+2)) */
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 9;
	CAN_Init(CAN1, &CAN_InitStructure);

	/* CAN过滤配置 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

#if CAN1_INT_EN
	/* NVIC配置 */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_RX_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* 使能中断 */
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);	//使能FIFO0挂号中断
#endif
}

/**
  * @功	能	CAN1中断接收
  * @参	数	无
  * @返	回	无
  */
void CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMsg;

	/* 从FIFO中接收数据包 */
	CAN_Receive(CAN1, CAN_FIFO0, &RxMsg);
	
	/* 数据包传入协议层进行解析 */
	canDispatch(&RxMsg);
}

/**
  * @功	能	CAN1数据发送
  * @参	数	ID：CAN数据帧的ID
  *			msg：要发送的数据地址
  *			len：要发送的数据长度
  * @返	回	0：发送成功
  *			其他：发送失败
  */
uint8_t CAN1_Send_Msg(uint8_t ID, uint8_t* msg, uint8_t len)
{	
	uint8_t mbox;
	uint16_t i=0;
	CanTxMsg TxMessage;
	
	TxMessage.StdId=ID;	 	// 标准标识符为0
	TxMessage.ExtId=ID;	 	// 设置扩展标示符（29位）
	TxMessage.IDE=0;		// 使用扩展标识符
	TxMessage.RTR=0;		// 消息类型为数据帧，一帧8位
	TxMessage.DLC=len;		// 数据长度
	for(i=0;i<len;i++)		// 装载发送数据
		TxMessage.Data[i]=msg[i];	
		
	i=0xFFF;
	do
		mbox= CAN_Transmit(CAN1, &TxMessage);   		//发送数据，返回使用的邮箱
	while((mbox == CAN_TxStatus_NoMailBox) && (i--));	//若邮箱已满，则超时重发
	
	if(mbox == CAN_TxStatus_NoMailBox)	return 2;		//邮箱始终满载，发送失败
	
	i = 0;
	while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))	i++;//等待发送结束
	if(i>=0XFFF)	return 1;	//发送超时返回
	return 0;		
}

/**
  * @功	能	CAN1数据查询接收
  * @参	数	RxMessage：数据接收缓冲区地址
  * @返	回	接收数据的长度，无数据时返回0
  */
uint8_t CAN1_Receive_Msg(CanRxMsg* RxMessage)
{
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//没有接收到数据,直接退出 
    CAN_Receive(CAN1, CAN_FIFO0,RxMessage);//读取数据	
	return RxMessage->DLC;	
}

/**
  * @功	能	CAN2端口初始化
  * @参	数	无
  * @返	回	无
  */
void CAN2_Initializes(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	CAN_InitTypeDef		CAN_InitStructure;
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;

#if CAN2_INT_EN
	NVIC_InitTypeDef	NVIC_InitStructure;
#endif
	
	/* 使能时钟 */
	RCC_APB1PeriphClockCmd(CAN2_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(CAN2_GPIO_CLK, ENABLE);			  //使能PORTA时钟
	
	/* 引脚配置 */
	GPIO_InitStructure.GPIO_Pin = CAN2_RX_PIN | CAN2_TX_PIN;	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(CAN2_GPIO_PORT, &GPIO_InitStructure);
	
	//引脚复用映射配置
	GPIO_PinAFConfig(CAN2_GPIO_PORT,GPIO_PinSource5,GPIO_AF_CAN2); //GPIOB5复用为CAN2
	GPIO_PinAFConfig(CAN2_GPIO_PORT,GPIO_PinSource6,GPIO_AF_CAN2); //GPIOB6复用为CAN2

	/* 配置CAN参数 */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	
	/* 配置CAN波特率 = 1MBps(45MHz/9/(1+2+2)) */
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 9;
	CAN_Init(CAN2, &CAN_InitStructure);
	
	/* CAN过滤配置 */
	CAN_SlaveStartBank(14);	//指定过滤器起始地址
	CAN_FilterInitStructure.CAN_FilterNumber = 14;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;	//使用FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
#if CAN2_INT_EN	
	/* NVIC配置 */
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN2_RX_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* 使能中断 */
	CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);
#endif	
}
	
/**
  * @功	能	CAN2中断接收
  * @参	数	无
  * @返	回	无
  */
void CAN2_RX1_IRQHandler(void)
{
	CanRxMsg RxMsg;

	/* 从FIFO中接收数据包 */
	CAN_Receive(CAN2, CAN_FIFO1, &RxMsg);
	
	/* 数据包传入协议层进行解析 */
	canDispatch(&RxMsg);
}

/**
  * @功	能	CAN2数据发送
  * @参	数	ID：CAN数据帧的ID
  *			msg：要发送的数据地址
  *			len：要发送的数据长度
  * @返	回	0：发送成功
  *			其他：发送失败
  */
uint8_t CAN2_Send_Msg(uint8_t ID, uint8_t* msg, uint8_t len)
{	
	uint8_t mbox;
	uint16_t i=0;
	CanTxMsg TxMessage;
	
	TxMessage.StdId=ID;	 	// 标准标识符为0
	TxMessage.ExtId=ID;	 	// 设置扩展标示符（29位）
	TxMessage.IDE=0;		// 使用扩展标识符
	TxMessage.RTR=0;		// 消息类型为数据帧，一帧8位
	TxMessage.DLC=len;		
	for(i=0;i<len;i++)
		TxMessage.Data[i]=msg[i];		
	
	i=0xFFF;
	do
		mbox= CAN_Transmit(CAN2, &TxMessage);   		//发送数据，返回使用的邮箱
	while((mbox == CAN_TxStatus_NoMailBox) && (i--));	//若邮箱已满，则超时重发
	
	if(mbox == CAN_TxStatus_NoMailBox)	return 2;		//邮箱始终满载，发送失败
	
	i=0;
	while((CAN_TransmitStatus(CAN2, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//等待发送结束
	if(i>=0XFFF)return 1;
	return 0;		
}

/**
  * @功	能	CAN2数据查询接收
  * @参	数	RxMessage：数据接收缓冲区地址
  * @返	回	接收数据的长度，无数据时返回0
  */
uint8_t CAN2_Receive_Msg(CanRxMsg* RxMessage)
{		   		   
    if(CAN_MessagePending(CAN2,CAN_FIFO1)==0)return 0;		//没有接收到数据,直接退出 
    CAN_Receive(CAN2, CAN_FIFO1,RxMessage);//读取数据	
	return RxMessage->DLC;	
}






/**
  ******************************************************************************
  * @文	件 ： SCA_APP.h
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.0
  * @日	期 ： 2019.8.20
  * @摘	要 ： SCA 测试程序
  ******************************************************************************/ 
  
#ifndef __SCA_APP_H
#define __SCA_APP_H
#include "sys.h"

void SCA_Init(void);	//控制器初始化
void SCA_Homing(void);	//执行器归零
void SCA_Exp1(void);  	//正反转Demo
void SCA_Exp2(void);	//高低速Demo
  
  
  
#endif



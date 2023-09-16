#ifndef __FUZZY_PID_H
#define __FUZZY_PID_H

//定义模糊PID结构体
typedef struct
{
 float setVaule;  //设定值
 float date_kp; //比例值增量
 float date_ki;  //积分值增量
 float date_kd;  //微分值增量
 float lasterror; //前一拍偏差
 float preerror;//前二拍偏差

 float maximum; //输入值的上限
 float minimum;  //输入值的下限
 
 float qKp;    //kp增量的修正系数
 float qKi;      //ki增量的修正系数
 float qKd;    //kd增量的修正系数
}FUZZYPID;

//外部文件可见
extern FUZZYPID FPID;
//模糊PID计算
void Fuzzytrans(float _Set_Vaule,float _Measure_Vaule,float pre_Measure_Vaule);

#endif


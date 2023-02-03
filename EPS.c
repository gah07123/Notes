#include "EPS.h"

static BCS_2_260 mBCS_2_260;
static BCS_9_26C mBCS_9_26C;
static BCS_10_26D mBCS_10_26D;

static OutputData Output;

static float Vehicle_Speed;

//转化物理值
u16 Physcis(float value, s16 offset, float factor)
{
	s16 temp = 0;
	temp = ceil((value - offset) / factor);
	return temp;
}
//转化实际值
float Raw(u16 value, s16 offset, float factor)
{
	float temp = 0;
	temp = ((float)value * factor + offset);
	return temp;
}

u16 speed_handle(u16 speed_raw)	//处理速度比例缩放，当前轮速 = 实际轮速 / 车速 * 车速限制最大值(9.9)
{
	float temp;
	u16 result;
	temp = Raw(speed_raw, 0, 0.05625); 		//转化为实际值
	temp = temp / Vehicle_Speed * 9.9;
	result = Physcis(temp, 0, 0.05625);		//转化为逻辑值
	return result;
}

void BCS_2_260_UnPacking(CanRxMsg* RxMsg)
{
	mBCS_2_260.byte0 = RxMsg->Data[0];
	mBCS_2_260.byte1 = RxMsg->Data[1];
	mBCS_2_260.byte2 = RxMsg->Data[2];
	mBCS_2_260.byte3 = RxMsg->Data[3];
	mBCS_2_260.byte4_5_7 = RxMsg->Data[4] & 0xE0;
	mBCS_2_260.BCS_VehSpd = Physcis(Vehicle_Speed, 0, 0.05625);
	mBCS_2_260.byte6 = RxMsg->Data[6];
}

void BCS_9_26C_UnPacking(CanRxMsg* RxMsg)
{
	mBCS_9_26C.byte1_5_7 = RxMsg->Data[1] & 0xE0;		//把第1个数据的5至7位提取出来
	/*左前轮速*/
	mBCS_9_26C.BCS_FLWheelSpd = ((u16)(RxMsg->Data[1] & 0x1f) << 8) | (RxMsg->Data[2]);

	mBCS_9_26C.byte3_5_7 = RxMsg->Data[3] & 0xE0;		//把第3个数据的5至7位提取出来
	/*右前轮速*/
	mBCS_9_26C.BCS_FRWheelSpd = ((u16)(RxMsg->Data[3] & 0x1f) << 8) | (RxMsg->Data[4]);
}

void BCS_10_26D_UnPacking(CanRxMsg* RxMsg)
{
	mBCS_10_26D.byte1_5_7 = RxMsg->Data[1] & 0xE0;		//把第1个数据的5至7位提取出来
	/*左后轮速*/
	mBCS_10_26D.BCS_RLWheelSpd = ((u16)(RxMsg->Data[1] & 0x1f) << 8) | (RxMsg->Data[2]);

	mBCS_10_26D.byte3_5_7 = RxMsg->Data[3] & 0xE0;		//把第3个数据的5至7位提取出来
	/*右后轮速*/
	mBCS_10_26D.BCS_RRWheelSpd = ((u16)(RxMsg->Data[3] & 0x1f) << 8) | (RxMsg->Data[4]);
}

void BCS_2_260_Packing(CanTxMsg* TxMsg)
{
	/*重新载入限制后的车速*/
	TxMsg->Data[4] = mBCS_2_260.byte4_5_7 | ((mBCS_2_260.BCS_VehSpd >> 8) & 0xff);
	TxMsg->Data[5] = mBCS_2_260.BCS_VehSpd & 0xff;
	/*checksum*/
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + 
					  TxMsg->Data[3] + TxMsg->Data[4] + 
					  TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

void BCS_9_26C_Packing(CanTxMsg* TxMsg)
{
	/*限制左前轮速*/
	mBCS_9_26C.BCS_FLWheelSpd = speed_handle(mBCS_9_26C.BCS_FLWheelSpd);
	TxMsg->Data[1] = ((mBCS_9_26C.BCS_FLWheelSpd >> 8) & 0x1f) | mBCS_9_26C.byte1_5_7;
	TxMsg->Data[2] = mBCS_9_26C.BCS_FLWheelSpd & 0xff;
	/*限制右前轮速*/
	mBCS_9_26C.BCS_FRWheelSpd = speed_handle(mBCS_9_26C.BCS_FRWheelSpd);
	TxMsg->Data[3] = ((mBCS_9_26C.BCS_FRWheelSpd >> 8) & 0x1f) | mBCS_9_26C.byte3_5_7;
	TxMsg->Data[4] = mBCS_9_26C.BCS_FRWheelSpd & 0xff;
	/*checksum*/
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] +
					  TxMsg->Data[3] + TxMsg->Data[4] +
					  TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

void BCS_10_26D_Packing(CanTxMsg* TxMsg)
{
	/*限制左后轮速*/
	mBCS_10_26D.BCS_RLWheelSpd = speed_handle(mBCS_10_26D.BCS_RLWheelSpd);		
	TxMsg->Data[1] = ((mBCS_10_26D.BCS_RLWheelSpd >> 8) & 0x1f) | mBCS_10_26D.byte1_5_7;
	TxMsg->Data[2] = mBCS_10_26D.BCS_RLWheelSpd & 0xff;
	/*限制右后轮速*/
	mBCS_10_26D.BCS_RRWheelSpd = speed_handle(mBCS_10_26D.BCS_RRWheelSpd);		
	TxMsg->Data[3] = ((mBCS_10_26D.BCS_RRWheelSpd >> 8) & 0x1f) | mBCS_10_26D.byte3_5_7;
	TxMsg->Data[4] = mBCS_10_26D.BCS_RRWheelSpd & 0xff;
	/*checksum*/
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] +
					  TxMsg->Data[3] + TxMsg->Data[4] +
					  TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

/*Msgs and Signals functions*/
void Msg_Unpacking_EPS(CanRxMsg* RxMsg)
{
	switch (RxMsg->StdId)
	{
	case ID_BCS_2_260:
		BCS_2_260_UnPacking(RxMsg);
		break;
	case ID_BCS_9_26C:
		BCS_9_26C_UnPacking(RxMsg);
		break;
	case ID_BCS_10_26D:
		BCS_10_26D_UnPacking(RxMsg);
		break;
	default:
		break;
	}
}

void Msg_Assignment_EPS(CanTxMsg* TxMsg, CanRxMsg* RxMsg)
{
	u8 i_dlc;
	TxMsg->StdId = RxMsg->StdId;
	TxMsg->DLC = RxMsg->DLC;
	for (i_dlc = 0; i_dlc < RxMsg->DLC; i_dlc++)
	{
		TxMsg->Data[i_dlc] = RxMsg->Data[i_dlc];
	}
}

void Msg_Packing_EPS(CanTxMsg* TxMsg)
{
	switch (TxMsg->StdId)
	{
	case ID_BCS_2_260:
		BCS_2_260_Packing(TxMsg);
		break;
	case ID_BCS_9_26C:
		BCS_9_26C_Packing(TxMsg);
		break;
	case ID_BCS_10_26D:
		BCS_10_26D_Packing(TxMsg);
		break;
	default:
		break;
	}
}

void Signal_Handle_EPS(const InputData* input)
{
	/*读取原始车速*/
	Vehicle_Speed = input->BCS_VehSpd;
	/*输入报文初始化*/
	Msg_Unpacking_EPS(&(input->RxMsg_260));
	Msg_Unpacking_EPS(&(input->RxMsg_26C));
	Msg_Unpacking_EPS(&(input->RxMsg_26D));	
	/*转发报文初始化*/
	Msg_Assignment_EPS(&(Output.TxMsg_260), &(input->RxMsg_260));
	Msg_Assignment_EPS(&(Output.TxMsg_26C), &(input->RxMsg_26C));
	Msg_Assignment_EPS(&(Output.TxMsg_26D), &(input->RxMsg_26D));
	/*发送标志位恒为1，无论何时都需要发送*/
	Output.TxMsg_260.CAN_Transmition = 1;
	Output.TxMsg_26C.CAN_Transmition = 1;
	Output.TxMsg_26D.CAN_Transmition = 1;
	
	if (Vehicle_Speed <= 9.9 || !(input->Auto_drive_flag))
	{
		//车速小于等于9.9，原封不动转发		
	}
	else
	{
		//车速高于9.9,限制车速在9.9
		mBCS_2_260.BCS_VehSpd = 0xB0;//相当于Physcis(9.9,0,0.05625);
		Msg_Packing_EPS(&(Output.TxMsg_260));
		Msg_Packing_EPS(&(Output.TxMsg_26C));
		Msg_Packing_EPS(&(Output.TxMsg_26D));
	}

}

void EPS_main(const InputData* input, OutputData* output)
{
	Signal_Handle_EPS(input);
	output = &Output;
}


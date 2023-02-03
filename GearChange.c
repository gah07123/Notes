#include "GearChange.h"

static u32 _10ms_counts = 0;

static GSM_1_190 mGSM_1_190;

static OutputData Output;

/*定时器*/
void TimeCounter_GC(void) //此处程序运行时不要置零，用于全局时钟
{
	_10ms_counts++;
//	if (_10ms_counts >= 100000)//1000
//		_10ms_counts = 0;
}

/*Msg Packing and Unpacking*/
void GSM_1_190_Packing(CanTxMsg* TxMsg)
{
	TxMsg->Data[1] = mGSM_1_190.GSM_GearShiftLeverPstReq & 0x0f;
	switch (mGSM_1_190.GSM_GearShiftLeverPstReq) 
	{
	case 4:  
	{
		mGSM_1_190.GSM_GearShiftLeverPstmech = 5;
		break;
	}
	case 3://N
	{
		mGSM_1_190.GSM_GearShiftLeverPstmech = 4;
		break;
	}
	case 2://R
	{
		mGSM_1_190.GSM_GearShiftLeverPstmech = 3;
		break;
	}
	default:
	{
		mGSM_1_190.GSM_GearShiftLeverPstmech = 1;
		break;
	}
	}
	TxMsg->Data[4] = (mGSM_1_190.GSM_GearShiftLeverPstmech << 3) & 0x38;

	if (mGSM_1_190.GSM_1_MsgCounter > 15)
	{
		mGSM_1_190.GSM_1_MsgCounter = 0;
	}
	else
	{
		mGSM_1_190.GSM_1_MsgCounter++;
	}
	TxMsg->Data[6] = mGSM_1_190.GSM_1_MsgCounter & 0x0f;
	mGSM_1_190.GSM_1_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] +
		TxMsg->Data[2] + TxMsg->Data[3] +
		TxMsg->Data[4] + TxMsg->Data[5] +
		TxMsg->Data[6]) ^ 0xff;
	TxMsg->Data[7] = mGSM_1_190.GSM_1_Checksum & 0xff;
}

/*Msgs and Signals functions*/
void Msg_Unpacking_GC(CanRxMsg* RxMsg)
{
	switch (RxMsg->StdId)
	{
	default:
		break;
	}
}

void Msg_Assignment_GC(CanTxMsg* TxMsg,CanRxMsg* RxMsg)
{
	u8 i_dlc;
	TxMsg->StdId = RxMsg->StdId;
	TxMsg->DLC = RxMsg->DLC;
	for(i_dlc = 0;i_dlc < RxMsg->DLC;i_dlc++)
	{
		TxMsg->Data[i_dlc] = RxMsg->Data[i_dlc];
	}
}

void Msg_Packing_GC(CanTxMsg* TxMsg)
{
	switch (TxMsg->StdId)
	{
	case ID_GSM_1_190:
		GSM_1_190_Packing(TxMsg);
		break;
	default:
		break;
	}
}

void Signal_Handle_GC(const InputData* input)
{
	/*转发报文初始化*/
	Msg_Assignment_GC(&(Output.TxMsg_190), &(input->RxMsg_190));

	if (input->MRR_PAS_Active == 2)//此处的判断条件正式版中应该以rxmode为判断条件
	{
		Output.TxMsg_190.CAN_Transmition = 1;
	}
	else
	{
		Output.TxMsg_190.CAN_Transmition = 0;
	}

	if (Output.TxMsg_190.CAN_Transmition == 1)//处于高速模式
	{
		if (input->SCU_GearLvlReqVD == 1 && input->SCU_GearLvlReq + input->VCU_CrntGearLvl != 5)//挡位使能，且当前挡位和目标挡位不同，需要发送挡位
			mGSM_1_190.GSM_GearShiftLeverPstReq = input->SCU_GearLvlReq;//目标位置
		else
		{
			mGSM_1_190.GSM_GearShiftLeverPstReq = 0x0b;//默认位置
		}
		Msg_Packing_GC(&(Output.TxMsg_190));//重新打包
	}
	else//不处于高速模式
	{
		mGSM_1_190.GSM_GearShiftLeverPstReq = 0x0b;
		Msg_Assignment_GC(&(Output.TxMsg_190), &(input->RxMsg_190));//原封不动转发
	}
}

void GearChange_main(const InputData* input,OutputData* output)
{
	TimeCounter_GC();
	Signal_Handle_GC(input);
	output = &Output;
}

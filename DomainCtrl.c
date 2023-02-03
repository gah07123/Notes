#include "DomainCtrl.h"

static OutputData Output;
CanTxMsg TxMsg;
bool BAN_CanTransmit;
bool VCU_VehRdySt_flag;
bool VCU_VehRdySt_last = 0;
u32 VCU_VehRdySt_time = 0;

u32 _10ms_counts = 0;
u32 global_time_100ms = 0;

/*Timer*/
void TimeCounter_DC(void) //10ms
{
	_10ms_counts++;
	if (_10ms_counts % 10 == 0)
	{
		global_time_100ms++;
	}
}

void MSG_Handling_1(InputData *input)
{
    if (!VCU_VehRdySt_last && input->VCU_VehRdySt) //检测到ready信号上升沿，不过滤用于初始化转向控制器
    {
        VCU_VehRdySt_flag = 1;
        VCU_VehRdySt_time = global_time_100ms;
    }
    if (global_time_100ms >= VCU_VehRdySt_time + 100 || !input->VCU_VehRdySt) //开机后10s左右后，启用过滤
    {
        VCU_VehRdySt_flag = 0;
    }
    
    VCU_VehRdySt_last = input->VCU_VehRdySt;
}

void MSG_Handling_2(InputData *input)
{
    if(!BAN_CanTransmit)
    {
        /*不启用过滤，正常转发*/
        Output.TxMsg_187.CAN_Transmition = 1;
        Output.TxMsg_186.CAN_Transmition = 1;
        Output.TxMsg_3A3.CAN_Transmition = 1;
        Output.TxMsg_19C.CAN_Transmition = 1;
        Output.TxMsg_3A6.CAN_Transmition = 1;
        Msg_Assignment_DC(&Output.TxMsg_186, &input->RxMsg_186);
        Msg_Assignment_DC(&Output.TxMsg_187, &input->RxMsg_187);
        Msg_Assignment_DC(&Output.TxMsg_3A3, &input->RxMsg_3A3);
        Msg_Assignment_DC(&Output.TxMsg_19C, &input->RxMsg_19C);
        Msg_Assignment_DC(&Output.TxMsg_3A6, &input->RxMsg_3A6);
    }
    else
    {
        /*启用过滤*/
        Output.TxMsg_187.CAN_Transmition = 0;
        Output.TxMsg_186.CAN_Transmition = 0;
        Output.TxMsg_3A3.CAN_Transmition = 0;
        Output.TxMsg_19C.CAN_Transmition = 0;
        Output.TxMsg_3A6.CAN_Transmition = 0;
    }
}

void MRR_Failed(InputData *input)
{
    if (!input->VCU_VehRdySt || VCU_VehRdySt_flag)
    {
        BAN_CanTransmit = 0; //无需过滤
    }
    else
    {
        BAN_CanTransmit = 1; //启用过滤
    }
}

void Msg_Assignment_DC(CanTxMsg *TxMsg, CanRxMsg *RxMsg)
{
    u8 i_dlc;
    TxMsg->StdId = RxMsg->StdId;
    TxMsg->DLC = RxMsg->DLC;
    for (i_dlc = 0; i_dlc < RxMsg->DLC; i_dlc++)
    {
        TxMsg->Data[i_dlc] = RxMsg->Data[i_dlc];
    }
}

void Signal_Handle_DC(InputData *input)
{
    TimeCounter_DC();
    MSG_Handling_1(input);
    MRR_Failed(input);
    MSG_Handling_2(input);

}

void DomainController_main(InputData *input, OutputData *output)
{
    Signal_Handle_DC(input);
    output = &Output;
}

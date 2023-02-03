#include "AEB_V2.5.h"

/*---------------------*/
/*2022.12.16 mLOW_PAS_1_140->input
			mEPS_2_16A->input
2022.12.19	add time control,adjust some variables
		*/

static OutputData Output;

static MRR_2_186 mMRR_2_186;
static MRR_1_3A6 mMRR_1_3A6;
static MRR_3_187 mMRR_3_187;
static MRR_IFC_2_19C mMRR_IFC_2_19C;
static PAS_3_162 mPAS_3_162;
static PAS_4_163 mPAS_4_163;

#define MODE_NONE 0
#define MODE_LOW_SPEED 1
#define MODE_HIGH_SPEED 2
#define DETECT_INTERVAL 10

/*global variable*/
static u8 Mode = 0;

static u8 MRR_Override_Remove_last = 0;
static u8 Fail_state_last = 0;
static u8 last_EBB_Brk_Applied = 0;
static u8 last_GearLvl = 0;
static u8 VCU_VehRdySt_last = 0;

static u8 PAS_TimeOut = 0;
static u8 PAS_Msg_count = 0;
static u8 MRR_TimeOut = 0;
static u8 MRR_Msg_count = 0;
static u8 ACAN_timeout = 0;
static u8 ACAN_count = 0;
static u8 brake_time_out = 0;
static u8 brake_count = 0;
static u8 Enter_Stop_count = 0;
static u8 mrrifc_count = 0;
static u8 PAS_MRR_count_last = 0;
static u8 EPS_2_MsgCounter_last = 0;

static u8 PAS_Gear_override = 0;
static u8 low_brake_flag = 0;
static u8 brake_flag = 0; // 2022.12.19
static u8 resume_flag = 0;
static u8 EPS_StrngWhlTorq_flag = 0;
static u8 fail_flag = 0;
static u8 MRR_BraRes_flag = 0;
static u8 MRR_Sound_Req;
static u8 PAS_BraRes_flag = 0;
static u8 PAS_Sound_Req;
static u8 handshake_flag = 0;
static u8 VCU_VehRdySt_flag = 0;
static u8 mrr_output_flag = 0;
static bool brake_appliy_result = false;
static bool brake_times_flag = 0;
static u8 brake_times_count = 0;
static u8 ebb_detect = 0;
static bool PAS_N_detect = 0;
static u8 PAS_APADrvReq_count;

static float EPS_StrngWhlTorq_value;
static float angle_now;
static float angle_last;
static float tar_angle;
static u8 PAS_Gear_Req;
static u8 PAS_SteeringCtrlVD;

/*Time control*/
static u32 _10ms_counts = 0;	  // 10msȫ��ʱ��
static u32 global_time_100ms = 0; // 100msȫ��ʱ��
static u32 ebb_detect_last_time;  // ebb�ϴμ��ʱ��
static u32 PAS_N_detect_time;
static u32 global_time_for_fail;
static u32 VCU_VehRdySt_time;

u16 Physcis(float value, s16 offset, float factor)
{
	s16 temp = 0;
	temp = ceil((value - offset) / factor);
	return temp;
}

float Raw(u16 value, s16 offset, float factor)
{
	float temp = 0;
	temp = ((float)value * factor + offset);
	return temp;
}

/*Timer*/
void TimeCounter(void) //10ms����һ�Σ�����ȫ��ʱ��
{
	_10ms_counts++;
	if (_10ms_counts % 10 == 0)
	{
		global_time_100ms++;
	}
}

void TimeOut_Handle(InputData *input)
{
	if (_10ms_counts % 2 == 0)//20ms
	{
		Enter_Stop_count++;
		brake_count++;
		Brake_detect(input);
	}
	if(MRR_Msg_count > 20)//MRR������س�ʱ��������20ms
	{
		MRR_Msg_count = 21;
		MRR_TimeOut = 1;
	}
	else
	{
		MRR_TimeOut = 0;
	}
	if(PAS_Msg_count > 20)//��ʱ
	{
		PAS_Msg_count = 21;
		PAS_TimeOut = 1;
	}
	else
	{
		PAS_TimeOut = 0;
	}
	if(ACAN_count > 20)//ACANδ���ճ�ʱ
	{
		ACAN_timeout = 1;
		ACAN_count =21;
	}
	else
	{
		ACAN_timeout = 0;
	}

	if(PAS_MRR_count_last == input->PAS_MRR_count)
	{
		MRR_Msg_count++;
		PAS_Msg_count++;
	}
	else
	{
		MRR_Msg_count = 0;
		PAS_Msg_count = 0;
	}
	PAS_MRR_count_last = input->PAS_MRR_count;

	if(EPS_2_MsgCounter_last == input->EPS_2_MsgCounter)
	{
		ACAN_count++;
	}
	else
	{
		ACAN_count = 0;
	}
	EPS_2_MsgCounter_last = input->EPS_2_MsgCounter;
}

/*MSG Packing*/
void Msg_MRR_1_3A6_Packing(CanTxMsg *TxMsg)
{
	if(_10ms_counts % 2 == 0 && mrr_output_flag)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = (mMRR_1_3A6.MRR_ACCMode & 0x07) | ((mMRR_1_3A6.MRR_ACCTextInfo << 3) & 0xF8);
	TxMsg->Data[1] = 0x00;
	TxMsg->Data[2] = (((u8)(mMRR_1_3A6.MRR_TargetSpd >> 8)) & 0x01) | ((mMRR_1_3A6.MRR_TargetDistanceSt << 1) & 0x0E) | ((mMRR_1_3A6.MRR_TauGapSet << 4) & 0x70);
	TxMsg->Data[3] = (u8)(mMRR_1_3A6.MRR_TargetSpd & 0xff);
	TxMsg->Data[4] = 0;
	TxMsg->Data[5] = 0;
	if (mMRR_1_3A6.MRR_1_MsgCounter < 15)
	{
		mMRR_1_3A6.MRR_1_MsgCounter++;
	}
	else
	{
		mMRR_1_3A6.MRR_1_MsgCounter = 0;
	}
	TxMsg->Data[6] = (mMRR_1_3A6.MRR_1_MsgCounter & 0x0f);
	mMRR_1_3A6.MRR_1_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
	TxMsg->Data[7] = mMRR_1_3A6.MRR_1_Checksum & 0xff;
}

void Msg_MRR_2_186_Packing(CanTxMsg *TxMsg)
{
	if(_10ms_counts % 2 == 0 && mrr_output_flag)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = mMRR_2_186.MRR_LongCtrlTargetAccel & 0xff;
	TxMsg->Data[1] = ((mMRR_2_186.ADAS_LongCtrlTypReqLong << 2) & 0x1C) | ((mMRR_2_186.ADAS_IntlCrusMode << 5) & 0xE0);
	TxMsg->Data[2] = mMRR_2_186.MRR_ACCRecpDecel & 0xff;
	TxMsg->Data[3] = mMRR_2_186.MRR_LongCtrlTargetAccelJerkMinVa & 0x7f;
	TxMsg->Data[4] = mMRR_2_186.MRR_LongCtrTargetAccelJerkMaxnVa & 0x7f;
	TxMsg->Data[5] = (mMRR_2_186.MRR_LongCtrlDecToStopReq & 0x01) | ((mMRR_2_186.MRR_LongCtrlDriveOff << 2) & 0x04);
	if (mMRR_2_186.MRR_2_MsgCounter < 15)
	{
		mMRR_2_186.MRR_2_MsgCounter++;
	}
	else
	{
		mMRR_2_186.MRR_2_MsgCounter = 0;
	}
	TxMsg->Data[6] = ((mMRR_2_186.MRR_LongCtrlModeForBrkSys << 4) & 0x70) | (mMRR_2_186.MRR_2_MsgCounter & 0x0f) | ((mMRR_2_186.MRR_LongCtrlAccelCtrlReq << 7) & 0x80);
	mMRR_2_186.MRR_2_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
	TxMsg->Data[7] = mMRR_2_186.MRR_2_Checksum & 0xff;
}

void Msg_MRR_3_187_Packing(CanTxMsg *TxMsg)
{
	if(_10ms_counts % 2 == 0 && mrr_output_flag)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = (u8)(mMRR_3_187.MRR_AEBTargetDecel >> 8);
	TxMsg->Data[1] = (u8)(mMRR_3_187.MRR_AEBTargetDecel);
	TxMsg->Data[2] = (mMRR_3_187.MRR_AEBDecelCtrlReq & 0x01) |
					 ((mMRR_3_187.MRR_AEBLVehHoldReq << 2) & 0x04) |
					 ((mMRR_3_187.MRR_ABAReq << 3) & 0x08) |
					 ((mMRR_3_187.MRR_ABALevel << 4) & 0x30) |
					 ((mMRR_3_187.MRR_AWBReq << 6) & 0x40) |
					 ((mMRR_3_187.MRR_ABPReq << 7) & 0x80);
	TxMsg->Data[3] = ((mMRR_3_187.SCU_GearLvlReqVD << 7) & 0x80) | ((mMRR_3_187.SCU_GearLvlReq << 4) & 0x70) | (mMRR_3_187.MRR_AWBLevel & 0x0f);
	TxMsg->Data[4] = ((mMRR_3_187.MRR_ActTrqReqVD << 2) & 0x04) | ((mMRR_3_187.MRR_ActTrqReq >> 12) & 0x03);
	TxMsg->Data[5] = ((u8)(mMRR_3_187.MRR_ActTrqReq >> 4)) & 0xff;
	if (mMRR_3_187.MRR_3_MsgCounter < 15)
	{
		mMRR_3_187.MRR_3_MsgCounter++;
	}
	else
	{
		mMRR_3_187.MRR_3_MsgCounter = 0;
	}
	TxMsg->Data[6] = (((u8)(mMRR_3_187.MRR_ActTrqReq << 4)) & 0xf0) | (mMRR_3_187.MRR_3_MsgCounter & 0x0f);
	mMRR_3_187.MRR_3_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
	TxMsg->Data[7] = mMRR_3_187.MRR_3_Checksum & 0xff;
}

void Msg_PAS_3_162_Packing(CanTxMsg *TxMsg)
{
	// angle_last = Raw(mPAS_3_162.PAS_SteeringAngleReqValue,SteeringAngleRequest_Offset,SteeringAngleRequest_Factor);
	// shakehand();
	if(_10ms_counts % 2 == 0)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = (mPAS_3_162.PAS_SteeringCtrlReq << 1) & 0x06;
	TxMsg->Data[1] = ((u8)(mPAS_3_162.PAS_SteeringAngleReqValue >> 8)) & 0xff;
	TxMsg->Data[2] = ((u8)(mPAS_3_162.PAS_SteeringAngleReqValue)) & 0xff;
	TxMsg->Data[3] = 0x00;
	TxMsg->Data[4] = ((mPAS_3_162.FAPA_LSMSubMTReq << 2) & 0x0c) | ((mPAS_3_162.FAPA_LSMSubMTLevel << 4) & 0x30) | ((mPAS_3_162.FAPA_LSMSubMTLong << 6) & 0xc0);
	TxMsg->Data[5] = (mPAS_3_162.FAPA_LSMVehDirRq << 4) & 0x30;
	if (mPAS_3_162.PAS_3_MsgCounter < 15)
	{
		mPAS_3_162.PAS_3_MsgCounter++;
	}
	else
	{
		mPAS_3_162.PAS_3_MsgCounter = 0;
	}
	TxMsg->Data[6] = mPAS_3_162.PAS_3_MsgCounter & 0x0f;
	mPAS_3_162.PAS_3_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2]) ^ 0xff;
	TxMsg->Data[7] = mPAS_3_162.PAS_3_Checksum & 0xff;
}

void Msg_PAS_4_163_Packing(CanTxMsg *TxMsg)
{
	if(_10ms_counts % 2 == 0)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = (u8)(mPAS_4_163.PAS_APA_ESPStopDist >> 8) & 0x0f;
	TxMsg->Data[1] = (u8)(mPAS_4_163.PAS_APA_ESPStopDist & 0xff);
	TxMsg->Data[2] = mPAS_4_163.PAS_APA_ESPSpdLimit;
	TxMsg->Data[3] = 0x00;
	if (mPAS_4_163.PAS_4_MsgCounter < 15)
	{
		mPAS_4_163.PAS_4_MsgCounter++;
	}
	else
	{
		mPAS_4_163.PAS_4_MsgCounter = 0;
	}
	TxMsg->Data[4] = ((mPAS_4_163.PAS_APASt << 4) & 0x70) | (mPAS_4_163.PAS_4_MsgCounter & 0x0f);
	TxMsg->Data[5] = 0x00;
	if (PAS_Sound_Req == 1 || MRR_Sound_Req == 1)
	{
		PAS_APADrvReq_count++;
		if (PAS_APADrvReq_count >= 3)
		{
			mPAS_4_163.PAS_APADrvReq = 0x07;
			PAS_APADrvReq_count = 4;
		}
		else //�ڱ�7֮ǰ�ȱ�����0
		{
			mPAS_4_163.PAS_APADrvReq = 0x00;
		}
	}
	else if (PAS_Sound_Req == 2 || MRR_Sound_Req == 2)
	{
		mPAS_4_163.PAS_APADrvReq = 0x14; //�ͷ�ɲ����ʾ��
		PAS_Sound_Req = 0;
		MRR_Sound_Req = 0;
		PAS_APADrvReq_count = 0;
	}
	else
	{
		mPAS_4_163.PAS_APADrvReq = 0x00;
		PAS_APADrvReq_count = 0;
	}
	TxMsg->Data[6] = mPAS_4_163.PAS_APADrvReq & 0x1f;
	mPAS_4_163.PAS_4_Checksum = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
	TxMsg->Data[7] = mPAS_4_163.PAS_4_Checksum & 0xff;
}

/* void Msg_feedback_Packing(CanTxMsg* TxMsg)
{
	TxMsg->Data[0] = (Output.MSG_Rec_Fail & 0x01) | ((mMSG_feedback.MSG_Fail_Feedback << 1) & 0x1E) | ((mMSG_feedback.MSG_GW_Statue << 5) & 0x60);
} */

void Msg_MRR_IFC_2_Packing(CanTxMsg *TxMsg)
{
	// MSG_MRR_IFC_2_Handle();
	static u8 ifc_mrr2_count = 0;
	if (mrr_output_flag)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->Data[0] = (mMRR_IFC_2_19C.IFC_TorqueReqSt << 3) | ((mMRR_IFC_2_19C.IFC_TorqueReq >> 8) & 0x07);
	TxMsg->Data[1] = mMRR_IFC_2_19C.IFC_TorqueReq & 0xff;
	TxMsg->Data[2] = 0x00;
	TxMsg->Data[3] = 0x1E;
	TxMsg->Data[4] = 0x78;
	TxMsg->Data[5] = 0x00;
	if (ifc_mrr2_count < 15)
	{
		ifc_mrr2_count++;
	}
	else
	{
		ifc_mrr2_count = 0;
	}
	TxMsg->Data[6] = (ifc_mrr2_count & 0x0f);
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

void Msg_MRR_IFC_1_Packing(CanTxMsg *TxMsg)
{
	static u8 fail_count = 0;
	static bool revers_flag = false;
	static u8 ifc_mrr1_count = 0;
	if (_10ms_counts % 10 == 0)//100ms
	{
		if (mrr_output_flag)
		{
			TxMsg->CAN_Transmition = 1;
		}
		else
		{
			TxMsg->CAN_Transmition = 0;
		}
	}
	if (fail_flag) // fail״̬������
	{
		if (fail_count < 2) //�ӹ���ʾ���������
		{
			if (global_time_100ms >= global_time_for_fail + 10) //����ʱ����10 * 100 ms = 1s
			{
				if (!revers_flag)
				{
					Output.IFC_LKS_TakeoverReq = 1;
					revers_flag = 1;
					fail_count++;
				}
				else
				{
					Output.IFC_LKS_TakeoverReq = 0;
					revers_flag = 0;
				}
				global_time_for_fail = global_time_100ms;
			}
			else
			{
			}
		}
		else
		{
			Output.IFC_LKS_TakeoverReq = 0x00;
			revers_flag = 0;
			fail_count = 3; //�ɼӿɲ��ӣ���Ϊglobal_time_for_failֻ���������س��ֵ�ʱ�����
		}
	}
	else
	{
		Output.IFC_LKS_TakeoverReq = 0x00;
		fail_count = 0;
		revers_flag = 0;
	}
	TxMsg->Data[0] = 0x0C;
	TxMsg->Data[1] = 0x4C;
	TxMsg->Data[3] = 0x01 | ((Output.IFC_LKS_TakeoverReq << 7) & 0x80);
	TxMsg->Data[4] = 0x00;
	TxMsg->Data[5] = 0x25;
	TxMsg->Data[6] = 0x06;

	if (ifc_mrr1_count < 15)
	{
		ifc_mrr1_count++;
	}
	else
	{
		ifc_mrr1_count = 0;
	}
	TxMsg->Data[2] = 0x90 | (ifc_mrr1_count & 0x0f);
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}


void Mode_handle(InputData *input)
{
	if (!Output.MSG_Rec_Fail) //���û��Fail����������
	{
		Mode = input->MRR_PAS_Active;
	}
	else // Fail�ˣ��ص������˹��ӹ�ģʽ
	{
		Mode = MODE_NONE;
	}
	// PAS_Active_last = Mode;

	if (MRR_TimeOut || PAS_TimeOut) //�����Ҫ��ֹ��û�յ����ٱ��ĵ�ʱ���������0��������LKA�ӿ�����һֱ��1
	{
		Mode = 0;
	}
}

void Fail_Handle(InputData *input)
{
	PAS_SteeringCtrlVD = input->PAS_SteeringCtrlVD;
	if ((!MRR_Override_Remove_last && input->MRR_Override_Remove) || resume_flag)
	{
		Output.MSG_Rec_Fail = 0;
		Output.MSG_Fail_Feedback = 0x00;
		PAS_Gear_override = 0;
		low_brake_flag = 0;
		resume_flag = 0;
		EPS_StrngWhlTorq_flag = 0; //�������������2Nm
	}
	else
	{
	}
	Fail_state_last = Output.MSG_Rec_Fail;
	// EPSFail || input->EPS_LKACtrlSt == 3 || input->EPS_LKACtrlSt == 4 ||
	if (input->EPS_PAS_EpasFailed || EPS_StrngWhlTorq_flag || input->EPS_LKACtrlSt == 3 || input->EPS_LKACtrlSt == 4)
	{
		Output.MSG_Rec_Fail = 1;
		if (input->EPS_PAS_AbortFeedback == 2 || EPS_StrngWhlTorq_flag)
		{
			Output.MSG_Fail_Feedback = 0x04;
		}
		else
		{
			Output.MSG_Fail_Feedback = 0x05;
		}
	}
	else if (PAS_TimeOut || MRR_TimeOut) //û���յ�ICP����
	{
		Output.MSG_Rec_Fail = 1;
		// 2022.12.16 mLOW_PAS_1_140->input
		input->PAS_SteeringCtrlVD = 0;//
		Output.MSG_Fail_Feedback = 0x01;
	}

	else if (ACAN_timeout) //û����ACAN��Ϣ
	{
		Output.MSG_Rec_Fail = 1;
		input->PAS_SteeringCtrlVD = 0;
		Output.MSG_Fail_Feedback = 0x02;
	}
	else if (Brake_fail(input) || low_brake_flag) //��ɲ������3��(�������1��)�����߳��ٳ���1km/h������ɲ��
	{
		Output.MSG_Rec_Fail = 1;
		input->PAS_SteeringCtrlVD = 0;
		Output.MSG_Fail_Feedback = 0x03;
	}
	else if (Mode == MODE_NONE)
	{
		Output.MSG_Rec_Fail = 1;
		input->PAS_SteeringCtrlVD = 0;
		Output.MSG_Fail_Feedback = 0x07; //�ߵ���ͬʱ����
	}
	else if (PAS_Gear_override)
	{
		Output.MSG_Rec_Fail = 1;
		input->PAS_SteeringCtrlVD = 0;
		Output.MSG_Fail_Feedback = 0x06; //����ģʽ�£���⵽N������ֹͣ���ƣ��ٻָ�
	}
	else
	{
	}

	if (!Fail_state_last && Output.MSG_Rec_Fail) //��⵽fail״̬�����أ�fail_flagȥ�����˹��ӹ���ʾ��
	{
		fail_flag = true;
		global_time_for_fail = global_time_100ms;
	}
	else if (!Output.MSG_Rec_Fail) //û�н���fail״̬���������˹��ӹ���ʾ��
	{
		fail_flag = false;
	}
	else
	{
	}
	MRR_Override_Remove_last = input->MRR_Override_Remove;
}

void MSG_MRR_1_3A6_Handle(InputData *input)
{
	mMRR_1_3A6.ADAS_ShifttoParkGearReq = 0;
	mMRR_1_3A6.MRR_ACCActiveSt = 1;
	mMRR_1_3A6.MRR_ACCFaultSt = 0;
	mMRR_1_3A6.MRR_ACCFuncCancel = 0;
	mMRR_1_3A6.MRR_ACCGoNotifier = 0;
	mMRR_1_3A6.MRR_ACCQuitSound = 0;
	mMRR_1_3A6.MRR_ACCTakeOverReq = 0;
	mMRR_1_3A6.MRR_ACCTextInfo = 0;
	mMRR_1_3A6.MRR_AEB_PEDPFaultSt = 0;
	mMRR_1_3A6.MRR_AEBFaultSt = 0;
	mMRR_1_3A6.MRR_AEBInterventionSt = 0;
	mMRR_1_3A6.MRR_AEBOffSt = 0;
	mMRR_1_3A6.MRR_FCWFaultSt = 0;
	mMRR_1_3A6.MRR_FCWInterventionSt = 0;
	mMRR_1_3A6.MRR_FCWLatentWarn = 0;
	mMRR_1_3A6.MRR_FCWOffSt = 0;
	mMRR_1_3A6.MRR_FCWPreWarn = 0;
	mMRR_1_3A6.MRR_FCWWarmDistance = 0;
	mMRR_1_3A6.MRR_ISLACC_St = 1;
	mMRR_1_3A6.MRR_ObjectCapturedSt = 0;
	mMRR_1_3A6.MRR_PEBSObjType = 0;

	if (Mode == MODE_HIGH_SPEED) //�������
	{
		mMRR_1_3A6.MRR_TargetSpd = input->MRR_TargetSpd;
		mMRR_1_3A6.MRR_ACCMode = 0x03;
		mMRR_1_3A6.MRR_TauGapSet = input->MRR_TauGapSet;
		mMRR_1_3A6.MRR_TargetDistanceSt = input->MRR_TargetDistanceSt;
	}
	else //�������
	{
		mMRR_1_3A6.MRR_TargetSpd = 0x1ff;
		mMRR_1_3A6.MRR_ACCMode = 0;
		mMRR_1_3A6.MRR_TauGapSet = 0;
		mMRR_1_3A6.MRR_TargetDistanceSt = 0;
	}
}

void MSG_MRR_2_186_Handle(InputData *input)
{
	mMRR_2_186.ADAS_LongCtrlTypReqMode = 0;
	mMRR_2_186.MRR_ACCMinBrakingReq = 0;
	mMRR_2_186.MRR_ACCRecpDecel = 0x64; // Physcis(0,-5,0.05);
	mMRR_2_186.MRR_ACCRecpDecelReq = 0;
	mMRR_2_186.MRR_IFC_PriCANAvailableSt = 1;
	mMRR_2_186.MRR_IFC_PriCANErrorSt = 0;
	mMRR_2_186.MRR_LongCtrlDriveOff = 0;
	mMRR_2_186.MRR_LongCtrlShutdownReq = 0;
	if (Mode == MODE_HIGH_SPEED) //�������
	{
		mMRR_2_186.ADAS_IntlCrusMode = input->ADAS_IntlCrusMode;
		mMRR_2_186.ADAS_LongCtrlTypReqLong = input->ADAS_LongCtrlTypReqLong;
		mMRR_2_186.MRR_LongCtrlAccelCtrlReq = input->MRR_LongCtrlAccelCtrlReq;
		mMRR_2_186.MRR_LongCtrlDecToStopReq = input->MRR_LongCtrlDecToStopReq;
		mMRR_2_186.MRR_LongCtrlModeForBrkSys = input->MRR_LongCtrlModeForBrkSys; //����������ֶ����ص�ʱ������3
		mMRR_2_186.MRR_LongCtrlTargetAccelJerkMinVa = input->MRR_LongCtrlTargetAccelJerkMinVa;
		mMRR_2_186.MRR_LongCtrTargetAccelJerkMaxnVa = input->MRR_LongCtrTargetAccelJerkMaxnVa;
		mMRR_2_186.MRR_LongCtrlDriveOff = input->MRR_LongCtrlDriveOff;
		mMRR_2_186.MRR_LongCtrlTargetAccel = input->MRR_LongCtrlTargetAccel;
	}
	else //�������
	{
		mMRR_2_186.MRR_LongCtrlAccelCtrlReq = 0;
		mMRR_2_186.MRR_LongCtrlDecToStopReq = 0;
		mMRR_2_186.MRR_LongCtrlModeForBrkSys = 0;
		mMRR_2_186.MRR_LongCtrlDriveOff = 0;
		mMRR_2_186.ADAS_IntlCrusMode = 0;
		mMRR_2_186.ADAS_LongCtrlTypReqLong = 0;
		mMRR_2_186.MRR_LongCtrlTargetAccelJerkMinVa = Physcis(0, -20, 0.2);
		mMRR_2_186.MRR_LongCtrTargetAccelJerkMaxnVa = Physcis(0, -5, 0.2);
		mMRR_2_186.MRR_LongCtrlTargetAccel = Physcis(0, -5, 0.05);
	}
}

void MSG_MRR_3_187_Handle(InputData *input)
{
	mMRR_3_187.MRR_ACCReqQua = 0;
	mMRR_3_187.MRR_ACCUnableActivated = 0;
	if (Mode == MODE_HIGH_SPEED) //�������
	{
		mMRR_3_187.MRR_ABALevel = input->MRR_ABALevel;
		mMRR_3_187.MRR_ABAReq = input->MRR_ABAReq;
		mMRR_3_187.MRR_ABPReq = input->MRR_ABPReq;
		mMRR_3_187.SCU_GearLvlReq = input->SCU_GearLvlReq;
		mMRR_3_187.SCU_GearLvlReqVD = input->SCU_GearLvlReqVD;
		mMRR_3_187.MRR_ActTrqReqVD = input->MRR_ActTrqReqVD;
		mMRR_3_187.MRR_ActTrqReq = input->MRR_ActTrqReq;
		mMRR_3_187.MRR_AEBDecelCtrlReq = input->MRR_AEBDecelCtrlReq;
		mMRR_3_187.MRR_AEBLVehHoldReq = input->MRR_AEBLVehHoldReq;
		mMRR_3_187.MRR_AEBTargetDecel = input->MRR_AEBTargetDecel;
		mMRR_3_187.MRR_AWBLevel = input->MRR_AWBLevel;
		mMRR_3_187.MRR_AWBReq = input->MRR_AWBReq;
		if (mMRR_3_187.SCU_GearLvlReqVD)
		{
			if (mMRR_3_187.SCU_GearLvlReq <= 1)
			{
				mMRR_3_187.SCU_GearLvlReq = 1;
			}
			else if (mMRR_3_187.SCU_GearLvlReq > 4)
			{
				mMRR_3_187.SCU_GearLvlReq = 4;
			}
			else
			{
			}
			if (mMRR_3_187.SCU_GearLvlReq != 1 && mMRR_3_187.SCU_GearLvlReq + input->VCU_CrntGearLvl != 5) //��ǰ��λ��Ŀ�굲λ��һ�£���������
			{
				MRR_Sound_Req = 1;	 // mPAS_4_163
				MRR_BraRes_flag = 1; //��ʾ��ͣ����־�������ͷ�ɲ����־
			}
			else if (mMRR_3_187.SCU_GearLvlReq != 1 && mMRR_3_187.SCU_GearLvlReq + input->VCU_CrntGearLvl == 5 && MRR_BraRes_flag)
			{
				MRR_Sound_Req = 2;
				MRR_BraRes_flag = 0;
			}
			else if (mMRR_3_187.SCU_GearLvlReq == 1)
			{
				MRR_Sound_Req = 0;
				MRR_BraRes_flag = 0;
			}
			else
			{
				MRR_BraRes_flag = 0;
			}
		}
		else
		{
			MRR_Sound_Req = 0;
			MRR_BraRes_flag = 0;
			mMRR_3_187.SCU_GearLvlReq = 1;
		}
	}
	else //�������
	{
		MRR_Sound_Req = 0;
		MRR_BraRes_flag = 0;
		mMRR_3_187.MRR_ABALevel = 0;
		mMRR_3_187.MRR_ABAReq = 0;
		mMRR_3_187.MRR_ABPReq = 0;
		mMRR_3_187.SCU_GearLvlReq = 0;
		mMRR_3_187.SCU_GearLvlReqVD = 0;
		mMRR_3_187.MRR_ActTrqReqVD = 0;
		mMRR_3_187.MRR_AEBDecelCtrlReq = 0;
		mMRR_3_187.MRR_AEBLVehHoldReq = 0;
		mMRR_3_187.MRR_AEBTargetDecel = 0;
		mMRR_3_187.MRR_ActTrqReq = Physcis(0, -5000, 1);
		mMRR_3_187.MRR_AWBLevel = 0;
		mMRR_3_187.MRR_AWBReq = 0;
	}
}

void MSG_MRR_IFC_2_19C_Handle(InputData *input)
{
	if (Mode == MODE_HIGH_SPEED && !input->PAS_SteeringCtrlVD)
	{
		if (input->IFC_TorqueReqSt == 1)
		{
			input->IFC_TorqueReqSt = 1;
			input->IFC_TorqueReq = 1024;
		}
		else if (input->IFC_TorqueReqSt == 2)
		{
			if (input->EPS_LKACtrlSt == 0)
			{
				mMRR_IFC_2_19C.IFC_TorqueReqSt = 1;
				mMRR_IFC_2_19C.IFC_TorqueReq = 1024;
			}
			else if (input->EPS_LKACtrlSt == 1)
			{
				mMRR_IFC_2_19C.IFC_TorqueReqSt = 2;
				mMRR_IFC_2_19C.IFC_TorqueReq = 1024;
			}
			else if (input->EPS_LKACtrlSt == 2)
			{
				mMRR_IFC_2_19C.IFC_TorqueReqSt = 2;
				mMRR_IFC_2_19C.IFC_TorqueReq = input->IFC_TorqueReq;
			}
			else if (input->EPS_LKACtrlSt == 3)
			{
				mMRR_IFC_2_19C.IFC_TorqueReqSt = 1;
				mMRR_IFC_2_19C.IFC_TorqueReq = 1024;
			}
			else
			{
				mMRR_IFC_2_19C.IFC_TorqueReqSt = 0;
				mMRR_IFC_2_19C.IFC_TorqueReq = 1024;
			}
		}
		else
		{
			mMRR_IFC_2_19C.IFC_TorqueReqSt = 0;
			mMRR_IFC_2_19C.IFC_TorqueReq = 1024; // 1024����0Nm,���ﵥ���Ǽ򻯼��㲻����Raw����
		}
	}
	else
	{
		if (Output.MSG_Rec_Fail && Mode == MODE_HIGH_SPEED && !input->PAS_SteeringCtrlVD && input->IFC_TorqueReq) //�������rxmode��������ΪҪ���и������������²Ž��������������Ȼ��Ӱ�������ʻ
		{
			mMRR_IFC_2_19C.IFC_TorqueReqSt = 1;
			mMRR_IFC_2_19C.IFC_TorqueReq = 1024; // 1024����0Nm,���ﵥ���Ǽ򻯼��㲻����Raw����
		}
		else
		{
			mMRR_IFC_2_19C.IFC_TorqueReqSt = 0;
			mMRR_IFC_2_19C.IFC_TorqueReq = 1024; // 1024����0Nm,���ﵥ���Ǽ򻯼��㲻����Raw����
		}
	}
}

void MSG_PAS_3_Handle(InputData *input)
{
	int PAS_GearReqVD = input->SCU_GearLvlReqVD;
	if (Mode == MODE_LOW_SPEED)
	{
		if (PAS_GearReqVD) //??2022.12.19,�Զ������
		{
			if (input->EBB_BrkPedalApplied && input->VCU_EMS_BrkPedalSt)
			{
				brake_flag = 1;
			}
			else
			{
			}
			if (input->VCU_CrntGearLvl == 4 && !brake_flag) //ͣ��������ʾ
			{
				PAS_Sound_Req = 1;
				PAS_BraRes_flag = 1;
			}
			else if (PAS_BraRes_flag && brake_flag)
			{
				PAS_Sound_Req = 2;
				PAS_BraRes_flag = 0;
			}
			else
			{
				PAS_BraRes_flag = 0;
			}
		}
		else
		{
			brake_flag = 0;
			PAS_Sound_Req = 0;
			PAS_BraRes_flag = 0;
		}
		if (brake_flag)
		{
			mPAS_3_162.FAPA_LSMSubMTReq = 0x02;
			mPAS_3_162.FAPA_LSMSubMTLevel = 0x01;
			mPAS_3_162.FAPA_LSMSubMTLong = 0x01;
			mPAS_3_162.FAPA_LSMVehDirRq = PAS_Gear_Req;
		}
		else
		{
			mPAS_3_162.FAPA_LSMSubMTReq = 0x00;
			mPAS_3_162.FAPA_LSMSubMTLevel = 0x00;
			mPAS_3_162.FAPA_LSMSubMTLong = 0x00;
			mPAS_3_162.FAPA_LSMVehDirRq = 0x00;
		}
	}
	else
	{
		brake_flag = 0;
		PAS_Sound_Req = 0;
		PAS_BraRes_flag = 0;
		mPAS_3_162.FAPA_LSMSubMTReq = 0x00;
		mPAS_3_162.FAPA_LSMSubMTLevel = 0x00;
		mPAS_3_162.FAPA_LSMSubMTLong = 0x00;
		mPAS_3_162.FAPA_LSMVehDirRq = 0x00;
	}
}

void VCU_VehRdySt_handle(InputData *input)
{
	if (!input->VCU_VehRdySt || VCU_VehRdySt_flag)
	{
		mrr_output_flag = 0;
	}
	else
	{
		mrr_output_flag = 1;
	}

	if (!VCU_VehRdySt_last && input->VCU_VehRdySt)
	{
		VCU_VehRdySt_flag = 1;
		VCU_VehRdySt_time = global_time_100ms;
	}
	if (global_time_100ms >= VCU_VehRdySt_time + 100 || !input->VCU_VehRdySt) //������10s
	{
		VCU_VehRdySt_flag = 0;
	}
	VCU_VehRdySt_last = input->VCU_VehRdySt;
}

void shakehand(InputData *input) //��������
{
	angle_now = -Raw(input->EPS_SteeringAngle, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
	// 2022.12.19
	angle_last = Raw(mPAS_3_162.PAS_SteeringAngleReqValue, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
	//
	tar_angle = Raw(input->PAS_SteeringAngleReqValue, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
	if (input->PAS_SteeringCtrlVD && !Output.MSG_Rec_Fail)
	{
		if (input->EPS_PAS_ReqFeedback == 0)
		{
			mPAS_3_162.PAS_SteeringCtrlReq = 1;
			mPAS_3_162.PAS_SteeringAngleReqValue = Physcis(angle_now, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
			handshake_flag = 0;
		}
		else if (input->EPS_PAS_ReqFeedback == 1)
		{
			if (!handshake_flag)
			{
				mPAS_3_162.PAS_SteeringCtrlReq = 2;
				if (tar_angle - angle_now >= 10)
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = Physcis((angle_now + 10), SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
				}
				else if (tar_angle - angle_now <= -10)
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = Physcis((angle_now - 10), SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
					;
				}
				else
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = input->PAS_SteeringAngleReqValue;
				}
				handshake_flag = 1;
			}
			else
			{
				mPAS_3_162.PAS_SteeringCtrlReq = 2;
				if (tar_angle - angle_last >= 20)
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = Physcis((angle_last + 20), SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
				}
				else if (tar_angle - angle_last <= -20)
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = Physcis((angle_last - 20), SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
					;
				}
				else
				{
					mPAS_3_162.PAS_SteeringAngleReqValue = input->PAS_SteeringAngleReqValue;
				}
			}
		}
		else
		{
			mPAS_3_162.PAS_SteeringCtrlReq = 0;
			mPAS_3_162.PAS_SteeringAngleReqValue = Physcis(angle_now, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
			handshake_flag = 0;
		}
	}
	else
	{
		handshake_flag = 0;
		mPAS_3_162.PAS_SteeringCtrlReq = 0;
		mPAS_3_162.PAS_SteeringAngleReqValue = Physcis(angle_now, SteeringAngleRequest_Offset, SteeringAngleRequest_Factor);
	}
}

void MSG_PAS_4_Handle(InputData *input)
{
	if (input->SCU_GearLvlReq == 0x02) //��λҪ���Ǹߵ��ٶ�ʹ��ͬһ�׵�λ����
	{
		PAS_Gear_Req = 0x02;
	}
	else if (input->SCU_GearLvlReq == 0x04)
	{
		PAS_Gear_Req = 0x01;
	}
	else
	{
		PAS_Gear_Req = 0;
	}
	if (Mode == MODE_LOW_SPEED)
	{
		if (PAS_Gear_Req) //��������ֵ��λλ�ò�Ϊ0,2022.12.19
		{
			mPAS_4_163.PAS_APASt = 0x03;
		}
		else
		{
			mPAS_4_163.PAS_APASt = 0x00;
		}
		mPAS_4_163.PAS_APA_ESPSpdLimit = input->PAS_APA_ESPSpdLimit;
		mPAS_4_163.PAS_APA_ESPStopDist = input->PAS_APA_ESPStopDist;
	}
	else
	{
		mPAS_4_163.PAS_APASt = 0x00;
		mPAS_4_163.PAS_APA_ESPSpdLimit = 0;
		mPAS_4_163.PAS_APA_ESPStopDist = 0;
	}
}

void Msg_TxMsg_03C_Packing(CanTxMsg *TxMsg, InputData *input)
{
	TxMsg->StdId = 0x3c;
	if(_10ms_counts % 4 == 0)
	{
		TxMsg->CAN_Transmition = 1;
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	if(input->PAS_MRR_Light == 3)
	{
		TxMsg->Data[2] = 0x30;
	}
	else
	{
		TxMsg->Data[2] = 0x00;
	}
	if(input->PAS_MRR_Horn)
	{
		TxMsg->Data[1] = 0xcf;
	}
	else
	{
		TxMsg->Data[1] = 0x00;
	}
	if(!input->PAS_MRR_Horn || input->PAS_MRR_Light != 3)
	{
		TxMsg->Data[5] = 0xff;
	}	
	else
	{
		TxMsg->Data[5] = 0x00;
	}
	TxMsg->Data[0] = 0x00;
	TxMsg->Data[3] = 0x00;
	TxMsg->Data[4] = 0x00;
	TxMsg->Data[6] = 0x00;
	TxMsg->Data[7] = 0x00;
}

void Msg_TxMsg_25D_Packing(CanTxMsg *TxMsg, InputData *input)
{
	if(input->PAS_MRR_Light == 1 || input->PAS_MRR_Light == 2)
	{
		if(_10ms_counts % 2 == 0)
		{
			TxMsg->CAN_Transmition = 1;
		}
		else
		{
			TxMsg->CAN_Transmition = 0;
		}
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}
	TxMsg->StdId = 0x25D;
	TxMsg->Data[0] = 0x80;
	TxMsg->Data[1] = 0x02;
	TxMsg->Data[2] = 0x10;
	if(input->PAS_MRR_Light == 2) 		//right turn light
	{
		TxMsg->Data[3] = 0x08;
		TxMsg->Data[4] = 0x09;
	}
	else if(input->PAS_MRR_Light == 1)	//left turn light
	{
		TxMsg->Data[3] = 0x04;
		TxMsg->Data[4] = 0x02;
	}
	else
	{
		TxMsg->Data[3] = 0x00;
		TxMsg->Data[4] = 0x01;
	}
	TxMsg->Data[5] = 0x02;
	TxMsg->Data[6] = 0x00;		//there should be a counter here, but it can work without counter
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

void Msg_TxMsg_318_Packing(CanTxMsg *TxMsg, InputData *input)
{
	TxMsg->StdId = 0x318;
	if(input->PAS_MRR_Light == 1 || input->PAS_MRR_Light == 2)
	{
		if(_10ms_counts % 2 == 0)
		{
			TxMsg->CAN_Transmition = 1;
		}
		else
		{
			TxMsg->CAN_Transmition = 0;
		}
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}

	TxMsg->Data[0] = 0x16;
	TxMsg->Data[1] = 0x80;
	if(input->PAS_MRR_Light == 2) 		//right turn light
	{
		TxMsg->Data[2] = 0x04;
	}
	else if(input->PAS_MRR_Light == 1)	// left turn light 
	{
		TxMsg->Data[2] = 0x01;
	}
	else
	{
		TxMsg->Data[2] = 0x00;
	}
	TxMsg->Data[3] = 0x00;
	TxMsg->Data[4] = 0x17;
	TxMsg->Data[5] = 0x00;
	TxMsg->Data[6] = 0x00;
	TxMsg->Data[7] = 0x12;
}

void Msg_TxMsg_306_Packing(CanTxMsg *TxMsg, InputData *input)
{
	TxMsg->StdId = 0x306;
	if(input->PAS_MRR_Light == 1 || input->PAS_MRR_Light == 2)
	{
		if(_10ms_counts % 2 == 0)
		{
			TxMsg->CAN_Transmition = 1;
		}
		else
		{
			TxMsg->CAN_Transmition = 0;
		}
	}
	else
	{
		TxMsg->CAN_Transmition = 0;
	}

	if(input->PAS_MRR_Light == 2) 		//right turn light
	{
		TxMsg->Data[0] = 0x30;
	}
	else if(input->PAS_MRR_Light == 1)	// left turn light 
	{
		TxMsg->Data[0] = 0x28;
	}
	else
	{
		TxMsg->Data[0] = 0x20;
	}
	TxMsg->Data[1] = 0x00;
	TxMsg->Data[2] = 0x00;
	TxMsg->Data[3] = 0x00;
	TxMsg->Data[4] = 0x00;
	TxMsg->Data[5] = 0x00;
	TxMsg->Data[6] = 0x00;		//there should be a counter here, but it can work without counter
	TxMsg->Data[7] = (TxMsg->Data[0] + TxMsg->Data[1] + TxMsg->Data[2] + TxMsg->Data[3] + TxMsg->Data[4] + TxMsg->Data[5] + TxMsg->Data[6]) ^ 0xff;
}

bool brake_appliy(InputData *input) //�ڳ��ٴ���1����ÿСʱ��ʱ�����̤����Ϊ�ӹ�
{
	if (input->BCS_VehSpd > 15 && input->EBB_BrkPedalApplied) //�ٶȸ���1m/s������ɲ���˳�����
	{
		brake_appliy_result = true;
	}
	else if (input->BCS_VehSpd <= 15 && input->EBB_BrkPedalApplied && input->VCU_EMS_BrkPedalSt) //�ٶȵ���1m/s������ɲ��10s
	{
		if (brake_time_out)
		{
			brake_appliy_result = true;
		}
		else
		{
			brake_appliy_result = false;
		}
	}
	else
	{
		brake_appliy_result = false;
	}
	return brake_appliy_result;
}

void Status_back(InputData *input) //����״̬λ
{
	if (Mode == MODE_HIGH_SPEED)
	{
		Output.MSG_GW_Statue = 0x02; //����ģʽ
	}
	else if (Mode == MODE_LOW_SPEED)
	{
		Output.MSG_GW_Statue = 0x01; //����ģʽ
	}
	else
	{
		Output.MSG_GW_Statue = 0x00; //�˹�ģʽ������ģʽ
	}
}

void Brake_detect(InputData *input)
{
	if (input->EBB_BrkPedalApplied && input->VCU_EMS_BrkPedalSt) //����Ƿ����̤��
	{
		if (brake_count > 500) //����̤�峬��10s�ӣ��˳�����,�˹��ӹ�
		{
			brake_count = 501;
			brake_time_out = 1;
		}
		else
		{
			brake_time_out = 0;
		}
	}
	else //û�в���̤�壬��������0
	{
		brake_count = 0;
		brake_time_out = 0;
	}
}

void EBB_detect(InputData *input) //���EBB�����أ���־λ��1
{
	if (input->EBB_BrkPedalApplied && !last_EBB_Brk_Applied)
	{
		if (global_time_100ms > ebb_detect_last_time + DETECT_INTERVAL) //��μ�¼��ʱ����ϴμ�¼��ʱ��������1.5s��¼Ϊ��Ч�ƶ�����ֹ�ƶ����ɴ���������
		{
			ebb_detect = true;
		}
		else
		{
			ebb_detect = false;
		}
		ebb_detect_last_time = global_time_100ms; //�˴���Ҫ��ͬ��ʱ��
	}
	last_EBB_Brk_Applied = input->EBB_BrkPedalApplied;
}

bool Brake_fail(InputData *input) //����Ƿ����̤�壬����ģʽ�²�3��̤���˳����ƣ�����ģʽ�²�һ���˳����ƣ��˹��ӹ�
{
	bool return_flag;
	if (Mode == MODE_LOW_SPEED) //����ģʽ��������
	{
		if (brake_times_count >= 3)
		{
			return_flag = true;
			low_brake_flag = 1;
		}
		else
		{
			return_flag = false;
		}
	}
	else if (Mode == MODE_HIGH_SPEED) //����ģʽ
	{
		return_flag = brake_appliy(input);
	}
	else //���������Ϊfalse
	{
		return_flag = false;
	}
	return return_flag;
}

void Func_brake_times_flag(InputData *input)
{
	if (input->PAS_Active) //ֻ�е���ģʽ�����ż���
	{
		if (input->VCU_CrntGearLvl != 4 && last_GearLvl == 4) //һ��P������P��������flag����
		{
			brake_times_flag = 1;
		}
		else if (input->VCU_CrntGearLvl == 4) // flag�رգ�����λ�ص�P��ʱ���������
		{
			brake_times_flag = 0;
		}
	}
	else //����ģʽ��������ģʽflag�ر�
	{
		brake_times_flag = 0;
	}
	last_GearLvl = input->VCU_CrntGearLvl;
}

void Brake_times_detect(InputData *input) //����P����̤����´���
{
	if (brake_times_flag)
	{
		if (ebb_detect)
		{
			brake_times_count++; //��������1
			ebb_detect = false;	 //�Ѽ��������ѱ�־λɾ��
		}
		else
		{
		}
	}
	else
	{
		brake_times_count = 0;
		ebb_detect = false;
	}
}

void PAS_detect_handle(InputData *input)
{
	if (input->PAS_Active)
	{
		if (input->VCU_CrntGearLvl == 2)
		{
			if (PAS_N_detect == 0)
			{
				PAS_N_detect_time = global_time_100ms;
				PAS_N_detect = 1; //������ȫ��ʱ��
			}
			else
			{
			}
			if (global_time_100ms > PAS_N_detect_time + 20 && PAS_N_detect)
			{
				PAS_Gear_override = 1;
			}
			else
			{
			}
		}
		else
		{
			PAS_N_detect = 0;
		}
	}
	else
	{
		PAS_N_detect = 0;
	}
}

void EPS_StrngWhlTorq_detect(InputData *input)
{
	//ֻ�ڸ��ٵ������ʹ�ã����ٽӹܻ��Զ�Fail
	if (Mode == MODE_HIGH_SPEED)
	{
		EPS_StrngWhlTorq_value = Raw(input->EPS_StrngWhlTorq, -127, 1) / 10; //�����offset,factor������10��
		if (EPS_StrngWhlTorq_value < -2 || EPS_StrngWhlTorq_value > 2)
		{
			if (!mPAS_3_162.PAS_SteeringCtrlReq) //���������Ҫ�Ƿ���ת�ǿ��Ƶķ�����ת�ÿ��˷����������غ����״��2Nm
				EPS_StrngWhlTorq_flag = 1;
			else
			{
			}
		}
	}
	else
	{
	}
}

void Msg_Unpacking_AEB25(CanRxMsg *RxMsg)
{
	switch (RxMsg->StdId)
	{
	case ID_MRR_2_186:
		break;
	case ID_MRR_3_187:
		break;
	case ID_MRR_IFC_1_3A3:
		break;
	case ID_MRR_IFC_2_19C:
		break;

	default:
		break;
	}
}

void Msg_Assignment_AEB25(CanTxMsg *TxMsg, CanRxMsg *RxMsg)
{
	u8 i_dlc;
	TxMsg->StdId = RxMsg->StdId;
	TxMsg->DLC = RxMsg->DLC;
	for (i_dlc = 0; i_dlc < RxMsg->DLC; i_dlc++)
	{
		TxMsg->Data[i_dlc] = RxMsg->Data[i_dlc];
	}
}

void Msg_Packing_AEB25(CanTxMsg *TxMsg)
{
	switch (TxMsg->StdId)
	{
	case ID_MRR_1_3A6:
		Msg_MRR_1_3A6_Packing(TxMsg);
		break;
	case ID_MRR_2_186:
		Msg_MRR_2_186_Packing(TxMsg);
		break;
	case ID_MRR_3_187:
		Msg_MRR_3_187_Packing(TxMsg);
		break;
	case ID_MRR_IFC_1_3A3:
		Msg_MRR_IFC_1_Packing(TxMsg);
		break;
	case ID_MRR_IFC_2_19C:
		Msg_MRR_IFC_2_Packing(TxMsg);
		break;
	case ID_PAS_3_162:
		Msg_PAS_3_162_Packing(TxMsg);
		break;
	case ID_PAS_4_163:
		Msg_PAS_4_163_Packing(TxMsg);
		break;
	default:
		break;
	}
}

void Signal_Handle_AEB25(InputData *input)
{
	/*msg initializing*/
	
	/*msg handling*/
	TimeOut_Handle(input);
	Mode_handle(input);
	shakehand(input);
	MSG_MRR_1_3A6_Handle(input);
	MSG_MRR_2_186_Handle(input);
	MSG_MRR_3_187_Handle(input);
	MSG_PAS_3_Handle(input);
	MSG_PAS_4_Handle(input);
	Status_back(input);
	Brake_times_detect(input);
	EPS_StrngWhlTorq_detect(input);
	VCU_VehRdySt_handle(input);
	/*msg packing*/
	Msg_Packing_AEB25(&Output.TxMsg_186);
	Msg_Packing_AEB25(&Output.TxMsg_187);
	Msg_Packing_AEB25(&Output.TxMsg_19C);
	Msg_Packing_AEB25(&Output.TxMsg_162);
	Msg_Packing_AEB25(&Output.TxMsg_163);
	Msg_Packing_AEB25(&Output.TxMsg_3A6);
	Msg_Packing_AEB25(&Output.TxMsg_3A3);
	Msg_TxMsg_318_Packing(&Output.TxMsg_318,input);	//以下做法主要是没有dbc解析
	Msg_TxMsg_306_Packing(&Output.TxMsg_306,input);
	Msg_TxMsg_03C_Packing(&Output.TxMsg_03C,input);
	Msg_TxMsg_25D_Packing(&Output.TxMsg_25D,input);
}

void AEB_V25_main(InputData *input, OutputData *output)
{
	Signal_Handle_AEB25(input);
	output = &Output;
}
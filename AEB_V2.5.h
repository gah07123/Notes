#ifndef _AEB_V25_H
#define _AEB_V25_H

#include "Common.h"

#define ID_MRR_1_3A6 0x3A6
#define ID_MRR_2_186 0x186
#define ID_MRR_3_187 0x187
#define ID_PAS_4_163 0x163
#define ID_PAS_3_162 0x162
#define ID_EPS_2_16A 0x16A
#define ID_BCS_2_A 0x260
#define ID_VCU_9_A_2AB 0x2AB
#define ID_VCU_13_A_2B1 0x2B1
#define ID_MRR_IFC_2_19C 0x19C
#define ID_MRR_IFC_1_3A3 0x3A3
#define ID_VCU_21_A_176 0x176
#define ID_EBB_2_A_283 0x283
#define ID_VCU_14_370 0x370

//�Զ�����Ϣ
#define ID_HIGH_MRR_1_120 0x120
#define ID_LOW_PAS_1_140 0x140
#define ID_MSG_feedback 0x1ff
#define ID_MSG_100 0x100
#define ID_HIGH_MRR_AEB 0x121
#define ID_EPS_1_366 0x366
#define ID_VCU_2_360 0x360

#define SteeringAngleRequest_Offset -780
#define SteeringAngleRequest_Factor 0.1
#define Acc_Brk_TrqReq_Offset -5000
#define Acc_Brk_TrqReq_Factor 1
#define Brk_Offset -5
#define Brk_Factor 0.05

typedef struct MRR_1_3A6
{
	u8 ADAS_ShifttoParkGearReq;
	u8 MRR_ACCActiveSt;
	u8 MRR_ACCFaultSt;
	u8 MRR_ACCFuncCancel;
	u8 MRR_ACCGoNotifier;
	u8 MRR_ACCMode;
	u8 MRR_ACCQuitSound;
	u8 MRR_ACCTakeOverReq;
	u8 MRR_ACCTextInfo;
	u8 MRR_AEB_PEDPFaultSt;
	u8 MRR_AEBFaultSt;
	u8 MRR_AEBInterventionSt;
	u8 MRR_AEBOffSt;
	u8 MRR_FCWFaultSt;
	u8 MRR_FCWInterventionSt;
	u8 MRR_FCWLatentWarn;
	u8 MRR_FCWOffSt;
	u8 MRR_FCWPreWarn;
	u8 MRR_FCWWarmDistance;
	u8 MRR_ISLACC_St;
	u8 MRR_ObjectCapturedSt;
	u8 MRR_PEBSObjType;
	u8 MRR_TargetDistanceSt;
	u16 MRR_TargetSpd;
	u8 MRR_TauGapSet;
	u8 MRR_1_MsgCounter;
	u8 MRR_1_Checksum;
} MRR_1_3A6;

typedef struct MRR_2_186
{
	u8 ADAS_IntlCrusMode;		// Intelligent Cruise Mode For HMI
	u8 ADAS_LongCtrlTypReqLong; // motion service type from LongitudinalControl(from driving)
	u8 ADAS_LongCtrlTypReqMode; // IntelligentCruise from LongitudinalControl
	u8 MRR_ACCMinBrakingReq;
	u8 MRR_ACCRecpDecel;
	u8 MRR_ACCRecpDecelReq;
	u8 MRR_IFC_PriCANAvailableSt;
	u8 MRR_IFC_PriCANErrorSt;
	u8 MRR_LongCtrlAccelCtrlReq;
	u8 MRR_LongCtrlDecToStopReq;
	u8 MRR_LongCtrlDriveOff;
	u8 MRR_LongCtrlModeForBrkSys;
	u8 MRR_LongCtrlShutdownReq;
	u8 MRR_LongCtrlTargetAccel;
	u8 MRR_LongCtrlTargetAccelJerkMinVa;
	u8 MRR_LongCtrTargetAccelJerkMaxnVa;
	u8 MRR_2_MsgCounter;
	u8 MRR_2_Checksum;
} MRR_2_186;

typedef struct MRR_3_187
{
	u8 SCU_GearLvlReq;
	u8 SCU_GearLvlReqVD;
	u8 MRR_ABALevel;
	u8 MRR_ABAReq;
	u8 MRR_ABPReq;
	u8 MRR_ACCReqQua;
	u8 MRR_ACCUnableActivated;
	u16 MRR_ActTrqReq;
	u8 MRR_ActTrqReqVD;
	u8 MRR_AEBDecelCtrlReq;
	u8 MRR_AEBLVehHoldReq;
	u16 MRR_AEBTargetDecel;
	u8 MRR_AWBLevel;
	u8 MRR_AWBReq;
	u8 MRR_3_MsgCounter;
	u8 MRR_3_Checksum;
} MRR_3_187;

typedef struct PAS_4_163
{
	u8 PAS_APASt;
	u16 PAS_APA_ESPStopDist;
	u8 PAS_APA_ESPSpdLimit;
	u8 PAS_4_MsgCounter;
	u8 PAS_4_Checksum;
	u8 PAS_APADrvReq;
} PAS_4_163;

typedef struct PAS_3_162
{
	u8 FAPA_LSMSubMTReq; // 0x02
	u8 FAPA_LSMSubMTLevel;
	u8 FAPA_LSMSubMTLong;
	u8 FAPA_LSMVehDirRq;
	u8 PAS_SteeringCtrlReq;
	u16 PAS_SteeringAngleReqValue;
	u8 PAS_3_MsgCounter;
	u8 PAS_3_Checksum;
} PAS_3_162;

typedef struct HIGH_MRR_1_120
{
	u8 MRR_ACCMode;
	u8 MRR_Active;
	u16 MRR_ActTrqReq;
	u8 MRR_ActTrqReqVD;
	u8 MRR_LongCtrlAccelCtrlReq;
	u8 MRR_LongCtrlDecToStopReq;
	u8 MRR_LongCtrlDriveOff;
	u8 MRR_LongCtrlModeForBrkSys;
	u8 MRR_LongCtrlTargetAccel;
	u8 MRR_TargetSpd;
	u8 SCU_GearLvlReqVD;
	u8 SCU_GearLvlReq;
	u8 MRR_LongCtrlTargetAccelJerkMinVa;
	u8 MRR_LongCtrTargetAccelJerkMaxnVa;
	u8 MRR_Override_Remove;
} HIGH_MRR_1_120;

typedef struct HIGH_MRR_AEB
{
	u8 MRR_ABALevel;
	u8 MRR_ABAReq;
	u8 MRR_ABPReq;
	u8 MRR_AEBDecelCtrlReq;
	u8 MRR_AEBLVehHoldReq;
	u16 MRR_AEBTargetDecel;
	u8 MRR_AWBLevel;
	u8 MRR_AWBReq;
	u8 IFC_TorqueReqSt;
	u16 IFC_TorqueReq;
	u8 ADAS_LongCtrlTypReqLong; // motion service type from LongitudinalControl(from driving)
	u8 ADAS_IntlCrusMode;
	u8 MRR_TargetDistanceSt;
	u8 MRR_TauGapSet;
} HIGH_MRR_AEB;

typedef struct LOW_PAS_1_140
{
	u8 PAS_Active;
	u8 PAS_APA_ESPSpdLimit;
	u16 PAS_APA_ESPStopDist;
	u8 PAS_Gear_Req;
	u8 PAS_GearReqVD;
	u16 PAS_SteeringAngleReqValue;
	u8 PAS_SteeringCtrlVD;
} LOW_PAS_1_140;

typedef struct MSG_feedback
{
	u8 MSG_Fail_Feedback;
	u8 MSG_Rec_Fail;
	u8 MSG_GW_Statue;
} MSG_feedback;

typedef struct EPS_2_16A
{
	u8 EPS_PAS_AbortFeedback; // 2.BIT
	bool EPS_PAS_EpasFailed;  // 1.BIT
	bool EPS_PAS_ReqFeedback;
	u8 EPS_LKACtrlSt;
	u8 EPS_LKACtrlAbortFeedback;
} EPS_2_16A;

typedef struct BCS_2_A_260
{
	bool BCS_VehSpdVD; // 1.BIT
	u16 BCS_VehSpd;
} BCS_2_A_260;

typedef struct EBB_2_A_283
{
	u8 EBB_BrkPedalApplied;
} EBB_2_A_283;

typedef struct VCU_21_A_176
{
	u8 VCU_EMS_BrkPedalSt;
} VCU_21_A_176;

typedef struct VCU_14_370
{
	u8 EMS_ACCButtInfo;
} VCU_14_370;

typedef struct MRR_IFC_2_19C
{
	u8 IFC_TorqueReqSt;
	u16 IFC_TorqueReq;
} MRR_IFC_2_19C;

typedef struct EPS_1_366
{
	u16 EPS_SteeringAngle;
	u8 EPS_StrngWhlTorq;
} EPS_1_366;

typedef struct VCU_2_360
{
	bool VCU_VehRdySt;
} VCU_2_360;

typedef struct MRR_IFC_1
{
	u8 IFC_TJA_ICA_St;
	bool IFC_LKS_TakeoverReq;
	u8 IFC_CruiseMode;
} MRR_IFC_1;

extern void Msg_Unpacking_AEB25(CanRxMsg *RxMsg);
extern void Msg_Assignment_AEB25(CanTxMsg *TxMsg, CanRxMsg *RxMsg);
extern void Msg_Packing_AEB25(CanTxMsg *TxMsg);
extern void Signal_Handle_AEB25(InputData *input);
extern void AEB_V25_main(InputData *input, OutputData *output);
/*msg handling*/
extern void TimeOut_Handle(InputData* input);
extern void Mode_handle(InputData* input);
extern void shakehand(InputData* input);
extern void MSG_MRR_1_3A6_Handle(InputData* input);
extern void MSG_MRR_2_186_Handle(InputData* input);
extern void MSG_MRR_3_187_Handle(InputData* input);
extern void MSG_PAS_3_Handle(InputData* input);
extern void MSG_PAS_4_Handle(InputData* input);
extern void Status_back(InputData* input);
extern bool Brake_fail(InputData* input);
extern void Brake_times_detect(InputData* input);
extern void EPS_StrngWhlTorq_detect(InputData* input);
extern void VCU_VehRdySt_handle(InputData* input);


#endif // !_AEB_V25_H

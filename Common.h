#ifndef _COMMON_
#define _COMMON_

#include "stdbool.h"           
#include "string.h"
#include "stdlib.h"          
#include "math.h"
#include "stdio.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef char s8;
typedef short s16;
typedef int s32;

typedef struct
{
	u8 CAN_Transmition;

	u32 StdId;  /*!< Specifies the standard identifier.
						  This parameter can be a value between 0 to 0x7FF. */

	u8 DLC;     /*!< Specifies the length of the frame that will be
						  transmitted. This parameter can be a value between
						  0 to 8 */

	u8 Data[8]; /*!< Contains the data to be transmitted. It ranges from 0
						  to 0xFF. */
} CanTxMsg;

typedef struct
{
	u32 StdId;  /*!< Specifies the standard identifier.
						  This parameter can be a value between 0 to 0x7FF. */

	u8 DLC;     /*!< Specifies the length of the frame that will be received.
						  This parameter can be a value between 0 to 8 */

	u8 Data[8]; /*!< Contains the data to be received. It ranges from 0 to
						  0xFF. */
} CanRxMsg;

typedef struct
{
	//HIGH_MRR_1_120 from ICP
	u8 MRR_PAS_Active;//rxmode
	u8 MRR_ACCMode;
	//u8 MRR_Active;
	u16 MRR_ActTrqReq;
	u8 MRR_ActTrqReqVD;
	u8 MRR_LongCtrlAccelCtrlReq;
	u8 MRR_LongCtrlDecToStopReq;
	u8 MRR_LongCtrlDriveOff;
	u8 MRR_LongCtrlModeForBrkSys;
	float MRR_LongCtrlTargetAccel;
	float MRR_TargetSpd;
	u8 SCU_GearLvlReqVD;
	u8 SCU_GearLvlReq;
	float MRR_LongCtrlTargetAccelJerkMinVa;
	float MRR_LongCtrTargetAccelJerkMaxnVa;
	u8 MRR_Override_Remove;

	//HIGH_MRR_AEB from ICP
	u8 MRR_ABALevel;
	u8 MRR_ABAReq;
	u8 MRR_ABPReq;
	u8 MRR_AEBDecelCtrlReq;
	u8 MRR_AEBLVehHoldReq;
	float MRR_AEBTargetDecel;
	u8 MRR_AWBLevel;
	u8 MRR_AWBReq;
	u8 IFC_TorqueReqSt;
	float IFC_TorqueReq;
	u8 ADAS_LongCtrlTypReqLong;   //motion service type from LongitudinalControl(from driving)
	u8 ADAS_IntlCrusMode;
	u8 MRR_TargetDistanceSt;
	u8 MRR_TauGapSet;

	//LOW_PAS_1_140 from ICP
	u8 PAS_Active;
	float PAS_APA_ESPSpdLimit;
	u16 PAS_APA_ESPStopDist;
	float PAS_SteeringAngleReqValue;
	u8 PAS_SteeringCtrlVD;
	u8 PAS_MRR_count;
	u8 PAS_MRR_Light;
	u8 PAS_MRR_Horn;
	//VCU_9_2AB
	u8 VCU_CrntGearLvl;	//form channel ACAN

	CanRxMsg RxMsg_190;	//from channel 

	/*20221208*/
	float BCS_VehSpd;	//from ACAN 20221208
	CanRxMsg RxMsg_260;	//from ACAN 20221208
	CanRxMsg RxMsg_26C;	//from ACAN 20221208
	CanRxMsg RxMsg_26D;	//from ACAN 20221208
	u8 Auto_drive_flag;	//�Զ����Ƿ�ƭ���ٱ�־λ 20221208 from ICP

	/*20221209*/
	CanRxMsg RxMsg_186;
	CanRxMsg RxMsg_187;
	CanRxMsg RxMsg_3A3;
	CanRxMsg RxMsg_19C;

	/*20221215*/
	u8 EPS_LKACtrlSt;	//from ACAN
	u8 EPS_PAS_EpasFailed; //from ACAN
	u8 EPS_2_MsgCounter;	//from ACAN

	/*20221219*/
	u8 EPS_PAS_AbortFeedback; 	//from ACAN
	u8 EPS_PAS_ReqFeedback;		//from ACAN
	u8 EBB_BrkPedalApplied; 	//from ACAN
	u8 VCU_EMS_BrkPedalSt; 		//from ACAN
	float EPS_StrngWhlTorq; 	//from ACAN
	u8 VCU_VehRdySt;			//from ACAN
	float EPS_SteeringAngle;	//from ACAN

	/*20230103*/
	CanRxMsg RxMsg_3A6;

}InputData;

typedef struct
{
	//to channel ICP
	u8 MSG_Fail_Feedback;
	u8 MSG_Rec_Fail;
	u8 MSG_GW_Statue;

	CanTxMsg TxMsg_190; //to channel PCAN

	CanTxMsg TxMsg_260;	//to ACAN 20221208
	CanTxMsg TxMsg_26C;	//to ACAN 20221208
	CanTxMsg TxMsg_26D;	//to ACAN 20221208

	/*20221209*/
	CanTxMsg TxMsg_186;
	CanTxMsg TxMsg_187;
	CanTxMsg TxMsg_3A3;
	CanTxMsg TxMsg_19C;

	/*20221219*/
	u8 IFC_LKS_TakeoverReq;  //to ACAN
	CanTxMsg TxMsg_3A6;
	CanTxMsg TxMsg_162;
	CanTxMsg TxMsg_163;

	/*20221220*/
	CanTxMsg TxMsg_03C;	//to BCAN horn and Emergency flashers
	CanTxMsg TxMsg_25D;	//to BCAN
	CanTxMsg TxMsg_318;	//to BCAN
	CanTxMsg TxMsg_306;	//to BCAN
}OutputData;



#endif // !_COMMON_

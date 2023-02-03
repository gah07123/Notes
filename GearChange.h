#ifndef _GEARCHANGE_
#define _GEARCHANGE_

#include "Common.h"

#define ID_GSM_1_190 0x190

typedef struct GSM_1_190
{
	u8 GSM_DriveMode;		
	u8 GSM_GearShiftLeverPstReq;
	u8 GSM_DriveModeRespFail;
	bool GSM_PGearErrSt;
	u8 GSM_GearShiftLeverPstmech;
	bool GSM_ECOErrSt;
	u8 GSM_1_MsgCounter;
	u8 GSM_WarnInfo;
	u8 GSM_1_Checksum;
}GSM_1_190;

extern void TimeCounter_GC(void);

extern void GSM_1_190_Packing(CanTxMsg* TxMsg);

extern void Msg_Unpacking_GC(CanRxMsg* RxMsg);
extern void Msg_Assignment_GC(CanTxMsg* TxMsg, CanRxMsg* RxMsg);
extern void Msg_Packing_GC(CanTxMsg* TxMsg);
extern void Signal_Handle_GC(const InputData* input);

extern void GearChange_main(const InputData* input,OutputData* output);





#endif // !_GEARCHANGE_

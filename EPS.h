#ifndef _EPS_H_
#define _EPS_H_

#include "Common.h"

#define ID_BCS_2_260  0x260
#define ID_BCS_9_26C  0x26C
#define ID_BCS_10_26D 0x26D

typedef struct BCS_2_260
{
	u8 byte0;
	u8 byte1;
	u8 byte2;
	u8 byte3;
	u8 byte4_5_7;
	u16 BCS_VehSpd;
	u8 byte6;
	u8 BCS_2_A_Checksum;
}BCS_2_260;

typedef struct BCS_9_26C
{
	u8 byte0;
	u8 byte1_5_7;//byte1的5至7位
	u16 BCS_FLWheelSpd;
	u16 BCS_FRWheelSpd;
	u8 byte3_5_7;//byte3的5至7位
	u8 byte5;
	u8 byte6;
	u8 BCS_9_A_Checksum;

}BCS_9_26C;

typedef struct BCS_10_26D
{
	u8 byte0;
	u8 byte1_5_7;
	u16 BCS_RLWheelSpd;
	u16 BCS_RRWheelSpd;
	u8 byte3_5_7;//byte3的5至7位
	u8 byte5;
	u8 byte6;
	u8 BCS_10_A_Checksum;
}BCS_10_26D;

extern float Raw(u16 value, s16 offset, float factor);
extern u16 Physcis(float value, s16 offset, float factor);
extern u16 speed_handle(u16 speed_raw);

extern void BCS_2_260_UnPacking(CanRxMsg* RxMsg);
extern void BCS_9_26C_UnPacking(CanRxMsg* RxMsg);
extern void BCS_10_26D_UnPacking(CanRxMsg* RxMsg);

extern void BCS_2_260_Packing(CanTxMsg* TxMsg);
extern void BCS_9_26C_Packing(CanTxMsg* TxMsg);
extern void BCS_10_26D_Packing(CanTxMsg* TxMsg);

extern void Msg_Unpacking_EPS(CanRxMsg* RxMsg);
extern void Msg_Assignment_EPS(CanTxMsg* TxMsg, CanRxMsg* RxMsg);
extern void Msg_Packing_EPS(CanTxMsg* TxMsg);
extern void Signal_Handle_EPS(const InputData* input);

extern void EPS_main(const InputData* input, OutputData* output);

#endif // !_EPS_H_

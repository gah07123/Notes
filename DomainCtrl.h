#ifndef _DCTRL_H_
#define _DCTRL_H_

#include "Common.h"


extern void Msg_Unpacking_DC(CanRxMsg *RxMsg);
extern void Msg_Assignment_DC(CanTxMsg *TxMsg, CanRxMsg *RxMsg);
extern void Msg_Packing_DC(CanTxMsg *TxMsg);
extern void Signal_Handle_DC(InputData *input);
extern void DomainController_main(InputData *input, OutputData *output);


#endif
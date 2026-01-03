

#ifndef RC522_H
#define RC522_H

#include "xil_types.h"
#include "xspi.h"
#include "xgpio.h"
#include "xparameters.h"

#define uchar   unsigned char
#define uint    unsigned int
#define u8      uint8_t
#define u16     uint16_t
#define u32     uint32_t
#define MAX_LEN 16


#define TxControlReg    0x14
#define CommandReg      0x01
#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReload_low     0x2D
#define TReload_high    0x2C
#define TxASKReg        0x15
#define ModeReg         0x11
#define CollReg         0x0E
#define ControlReg      0x0C
#define ComlEnReg       0x02
#define ComIrqReg       0x04
#define FIFOLevelReg    0x0A
#define FIFODataReg     0x09
#define BitFramingReg   0x0D
#define CommIrqReg      0x04
#define ErrorReg        0x06
#define PICC_ANTICOLL   0x93
#define PICC_REQIDL     0x26


#define MI_OK           0
#define MI_NOTAGERR     1
#define MI_ERR          2


extern uint8_t state_ment;
extern uint8_t str_1[MAX_LEN];
extern uint8_t card_id[5];
extern uint8_t card_id2[3];


u8 rc522_trans_recv(u8 data);
void trans_rc522(u8 address, u8 value);
u8 read_rc522(u8 address);

void set_bit_mask(u8 reg, u8 mask);
void clear_bit_mask(u8 reg, u8 mask);
void on_aten(void);
void off_aten(void);
void soft_reset(void);
void rc522_init(void);

u8 rc522_to_card(u8 command, u8* data_trans, u8 size_data_trans, u8* data_recv, u16* size_data_recv);
u8 rc522_anticol(u8 *serNum);
u8 rc522_request(uint8_t reqMode, uint8_t *TagType);
void get_card_id(void);
void bus_card(void);

#endif

#include "rc522.h"
#include <string.h>

extern XSpi  SpiInstance;  
extern XGpio GpioInstance; 


#define CS_CHANNEL  1
#define RST_CHANNEL 2


u8 rc522_trans_recv(u8 data){
    u8 data_receive;
    XSpi_Transfer(&SpiInstance, &data, &data_receive, 1);
    return data_receive;
}

void trans_rc522(u8 address, u8 value){
    XGpio_DiscreteWrite(&GpioInstance, CS_CHANNEL, 0);
    rc522_trans_recv((address << 1) & 0x7E);
    rc522_trans_recv(value);
    XGpio_DiscreteWrite(&GpioInstance, CS_CHANNEL, 1);
}

u8 read_rc522(u8 address){
    u8 val;
    XGpio_DiscreteWrite(&GpioInstance, CS_CHANNEL, 0);
    rc522_trans_recv(((address << 1) & 0x7E) | 0x80);
    val = rc522_trans_recv(0x00);
    XGpio_DiscreteWrite(&GpioInstance, CS_CHANNEL, 1);
    return val;
}


void set_bit_mask(u8 reg, u8 mask){
    u8 save = read_rc522(reg);
    trans_rc522(reg, save | mask);
}

void clear_bit_mask(u8 reg, u8 mask){
    u8 save = read_rc522(reg);
    trans_rc522(reg, save & (~mask));
}

void on_aten(void){
    u8 temp = read_rc522(TxControlReg);
    if(!(temp & 0x03)){
        set_bit_mask(TxControlReg, 0x03);
    }
}

void off_aten(void){
    clear_bit_mask(TxControlReg, 0x03);
}

void soft_reset(void){
    trans_rc522(CommandReg, 0x0f);
}

void rc522_init(void){
    XGpio_DiscreteWrite(&GpioInstance, RST_CHANNEL, 1);
    XGpio_DiscreteWrite(&GpioInstance, CS_CHANNEL, 1);
    soft_reset();
    trans_rc522(TModeReg, 0x8D);
    trans_rc522(TPrescalerReg, 0x3E);
    trans_rc522(TReload_high, 0x1E);
    trans_rc522(TReload_low, 0x00);
    trans_rc522(TxASKReg, 0x40);
    trans_rc522(ModeReg, 0x3D);
    on_aten();
}


u8 rc522_to_card(u8 command, u8* data_trans, u8 size_data_trans, u8* data_recv, u16* size_data_recv){
    u8 save_state = 2; u8 IRQ_E = 0x00; u8 wait_IRQ_E = 0x00;
    u8 last_bit; u8 n; u16 i;

    if(command == CollReg){ IRQ_E = 0x12; wait_IRQ_E = 0x10; }
    else if(command == ControlReg){ IRQ_E = 0x77; wait_IRQ_E = 0x30; }

    trans_rc522(ComlEnReg, IRQ_E|0x80);
    clear_bit_mask(ComIrqReg, 0x80);
    set_bit_mask(FIFOLevelReg, 0x80);
    trans_rc522(CommandReg, 0x00);

    for(i=0; i<size_data_trans; i++){ trans_rc522(FIFODataReg, data_trans[i]); }
    trans_rc522(CommandReg, command);
    if(command == ControlReg){ set_bit_mask(BitFramingReg, 0x80); }

    i = 2000;
    do {
        n = read_rc522(CommIrqReg);
        i--;
    } while(i != 0 && !(n & 0x01) && !(n & wait_IRQ_E));

    clear_bit_mask(BitFramingReg, 0x80);

    if(i == 0) return 2;
    if(!(read_rc522(ErrorReg) & 0x1B)){
        save_state = 0;
        if(n & IRQ_E & 0x01) save_state = 1;
        if (command == ControlReg){
            n = read_rc522(FIFOLevelReg);
            last_bit = read_rc522(ControlReg) & 0x07;
            if (last_bit) *size_data_recv = (n-1)*8 + last_bit;
            else *size_data_recv = n*8;
            if (n == 0) n = 1;
            if (n > 16) n = 16;
            for (i=0; i<n; i++) data_recv[i] = read_rc522(FIFODataReg);
        }
    }
    return save_state;
}

u8 rc522_anticol(u8 *serNum){
    u8 status; u8 i; u8 serNumCheck=0; u16 unLen;
    trans_rc522(BitFramingReg, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = rc522_to_card(ControlReg, serNum, 2, serNum, &unLen);
    if (status == 0){
        for (i=0; i<4; i++) serNumCheck ^= serNum[i];
        if (serNumCheck != serNum[i]) status = 2;
    }
    return status;
}

u8 rc522_request(uint8_t reqMode, uint8_t *TagType){
    uint8_t status; u16 backBits;
    trans_rc522(BitFramingReg, 0x07);
    TagType[0] = reqMode;
    status = rc522_to_card(ControlReg, TagType, 1, TagType, &backBits);
    if ((status != 0) || (backBits != 0x10)) status = 2;
    return status;
}

void get_card_id(void){
    state_ment = rc522_request(0x26, str_1);
    if (state_ment != 0) return;
    state_ment = rc522_anticol(str_1);
    if (state_ment == 0){
        memcpy(card_id, str_1, 5);
        card_id2[0] = card_id[2];
        card_id2[1] = card_id[3];
        card_id2[2] = card_id[4];
    }
}

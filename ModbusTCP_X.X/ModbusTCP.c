#include "ModbusTCP.h"
#include <string.h>
#include <math.h>
#include <stdio.h>


#define ULL unsigned long long

//#define DebugFloat
//#define DebugLong
//#define DebugReg
////////////////////////////////////////////////////////////////////////////////
//structs and enums

static ModbusTCP modbus;
static ModbusREG regs = {{0},{0},{0}};


////////////////////////////////////////////////////////////////////////////////
//local scope variables and constants
const char SERIAL_NUMBER[] = "1111" ;

////////////////////////////////////////////////////////////////////////////////
//global variable
volatile GLOBAL_VARS gvars;
 
////////////////////////////////////////////////////////////////////////////////
//local functions
//returns the value of the required buffer
unsigned int get_ResponseValue(ModbusTCP *pdu);
unsigned long long lookup_mask(int val);

void Modbus_Configure(){
int i=0,j=0,k=0;
CONV_INT ui2c;
  for(i = 0;i < strlen(SERIAL_NUMBER)/2;i++){
     ui2c.valC[j++] = SERIAL_NUMBER[k++];
     ui2c.valC[j] = SERIAL_NUMBER[k++];
     regs.rd_reg[i] = ui2c.valUI;
     j = 0;
  }

}

////////////////////////////////////////////////////////////////////////////////
//test the mbap for correctness
ModbusTCP* test_MBAP(uint8_t* mbap){
ModbusTCP *pdu;
  pdu = &modbus;

   pdu->error = MB_OK;
   pdu->mbap.TransID = reverse(mbap[0],mbap[1]);
   pdu->mbap.ProtocolID = reverse(mbap[2],mbap[3]);
   
   if(pdu->mbap.ProtocolID != 0){
        pdu->error = ServerFAIL;
        goto end;
   }
   
   pdu->mbap.Length = reverse(mbap[4],mbap[5]);
   if(pdu->mbap.Length > MAX_DATA_LENGTH){
       pdu->error = IllegalDATA;
       goto end;
   }
   
   pdu->mbap.UnitID = mbap[6];
   pdu->mbap.FC = mbap[7];
   
   if(pdu->mbap.FC != O1 & pdu->mbap.FC != O2 & pdu->mbap.FC != O3 & 
      pdu->mbap.FC != O4 & pdu->mbap.FC != O5 & pdu->mbap.FC != O6 &
      pdu->mbap.FC != O15 & pdu->mbap.FC != O16)
   {
      pdu->error = IllegalFC;
   }
   pdu->error;
   pdu->mbap.FC;

end:
  return pdu;
}


////////////////////////////////////////////////////////////////////////////////
//test the pdu for correctness
ModbusTCP* test_PDU(ModbusTCP *_pdu,uint8_t *reqs){
ModbusTCP *pdu;
int temp;
int len = MBAP_LEN;
    pdu = _pdu;
    pdu->pdu.ByteCount = 0;
    pdu->pdu.StartingAddr = reverse(reqs[len],reqs[len+1]);
    pdu->pdu.RegQuantity =  reverse(reqs[len+2],reqs[len+3]);
    
   //this is a modbus fc16 final byte count
   if(pdu->mbap.FC == O16 | pdu->mbap.FC == O15){
       pdu->pdu.ByteCount = reqs[len+4];
       
       if (pdu->mbap.FC == O16 & (pdu->pdu.ByteCount != (pdu->pdu.RegQuantity << 1))) {   //(byteLen - 7)
         pdu->error = IllegalDATA;
       }
    }
    return pdu;
}


uint16_t set_Data(ModbusTCP *_pdu,uint8_t *resp){
static unsigned long long mask;
uint16_t bit_bytequantity,
             bit_startaddress,
             bit_startbyte,
             bit_modulo;
int16_t i,j,temp,len;
uint8_t tempA[20] = {0};
CONV_INT ui2c;
CONV_LONG l2c;
ModbusTCP *pdu = _pdu;
   j = 0;
   switch(pdu->mbap.FC){
     case O1:
         //a maximum of 64 bits can be sampled at ny given time
         bit_startaddress = pdu->pdu.StartingAddr;
         mask = 0x0UL;
         mask = lookup_mask(pdu->pdu.RegQuantity);
         l2c.valLL = 0;

         //get byte start offset [stil to be used]
         bit_startbyte = (pdu->pdu.StartingAddr / 8);
         bit_modulo    =  pdu->pdu.StartingAddr % 8;
         if(bit_modulo > 0 )
            bit_startbyte += 1;
            
         //get byte count from offset
         bit_bytequantity = pdu->pdu.RegQuantity / 8;
         bit_modulo = pdu->pdu.RegQuantity % 8;
         if(bit_modulo > 0)
            bit_bytequantity += 1;
            
         //O1 index 8 of responxe holds byte count not address
         pdu->pdu.StartingAddr = bit_bytequantity;
         //this area needs rework in the futureas it is a fixed
         //offset and quantity of 0-63 bits
         memcpy(l2c.valS,regs.wr_coils,sizeof(l2c));
         
         l2c.valLL >>= bit_startaddress;
         l2c.valLL &= mask;
         memcpy(resp,l2c.valS,sizeof(l2c));
         len = bit_bytequantity;
         break;
     case O3:case O4:
         for(i=pdu->pdu.StartingAddr;i<(pdu->pdu.StartingAddr+pdu->pdu.RegQuantity);i++){
            ui2c.valUI = (pdu->mbap.FC == O3)? regs.wr_reg[i]:regs.rd_reg[i];
            *(resp+(j++)) = ui2c.valC[1];
            *(resp+(j++)) = ui2c.valC[0];
         }
         len = j;
         break;
     case O5:
         bit_startaddress = pdu->pdu.StartingAddr / 8;
         bit_modulo = pdu->pdu.StartingAddr % 8;
         //0xFF is coils on 0x00 = coil off
         if(pdu->pdu.RegQuantity == 0xFF00)
            regs.wr_coils[bit_startaddress] |= (1<<bit_modulo);
         else
            regs.wr_coils[bit_startaddress] &= ~(1<<bit_modulo);
         len = 0;
         break;
      case O6:
         regs.wr_reg[pdu->pdu.StartingAddr] = pdu->pdu.RegQuantity;
         len = 0;
         break;
      case O15:
         bit_startaddress = pdu->pdu.StartingAddr / 8;
         j = 0;
         for(i = bit_startaddress;i<(bit_startaddress+pdu->pdu.ByteCount);i++){
             regs.wr_coils[i] = *(resp+(j++));
         }
         len = 0;
         break;
     case O16:
       j=pdu->pdu.StartingAddr;
       
       for(i=0;i<(pdu->pdu.RegQuantity*2);i++,j++){
           temp = resp[i] & 0xFF;
           temp <<= 8;
           temp |= resp[++i];
           regs.wr_reg[j] = temp;
       }
       len = 0;
       break;
       
   }
   return len;
}


uint16_t get_Response(ModbusTCP* pdu_,uint8_t* resp){
int i;
unsigned int temp_byte_count;
 CONV_INT ui2c;
 ModbusTCP* pdu = pdu_;
 ui2c.valUI = get_ResponseValue(pdu);

 switch(pdu->mbap.FC){

    case O5:case O6:case O15:case O16:
      // response mbap
      resp[4] = 0;
      resp[5] = 6;
      resp[9] = (pdu->pdu.StartingAddr & 0xFF);
      resp[8] = (pdu->pdu.StartingAddr >> 8)& 0xFF;
      resp[11] = ui2c.valC[0] & 0xFF;
      resp[10] = ui2c.valC[1] & 0xFF;
      i = 12;
       break;
    case O1:case O2:
       resp[8] = ui2c.valC[0] & 0xFF;
       temp_byte_count = ui2c.valUI + MBAP_UID_FC;
       ui2c.valUI = temp_byte_count;
       resp[4] = ui2c.valC[1];
       resp[5] = ui2c.valC[0];
       i = 9;
       break;
    case O3:case O4:
       ui2c.valUI *= 2;
       temp_byte_count = ui2c.valUI + MBAP_UID_FC;
       //O3 response pdu byte count
       resp[8] = ui2c.valC[0] & 0xFF;
       //03 response mbap length
       ui2c.valUI = temp_byte_count;
       resp[4] = ui2c.valC[1];
       resp[5] = ui2c.valC[0];
       i = 9;

       break;
  }
  return  i;
}


////////////////////////////////////////////////////////////////////////////////
//local functions
unsigned int get_ResponseValue(ModbusTCP *pdu){
unsigned int ret_val = 0;
  switch(pdu->mbap.FC){
    case O6:  ret_val = regs.wr_reg[pdu->pdu.StartingAddr];
         break;
    case O1: ret_val = pdu->pdu.RegQuantity/8;
             if((pdu->pdu.RegQuantity%8)>0)
                ret_val++;
         break;
    case O3:case O4:case O5:case O15:case O16: ret_val = pdu->pdu.RegQuantity;
         break;
  }
  return ret_val;
}

////////////////////////////////////////////////////////////////////////////////
//lookup mask for bit request, at present this can only handle a 64bit request
//over modbus
unsigned long long lookup_mask(int val){
ULL temp = 0UL;
int i = 0;
  if(val < 16){
    for(i=0;i<val;i++){
      temp |= (1<<i);
    }
  }
  else{
    switch(val){
      case 16:temp = 0x1ffff;break;
      case 17:temp = 0x3ffff;break;
      case 18:temp = 0x7ffff;break;
      case 19:temp = 0xfffff;break;
      case 20:temp = 0x1fffff;break;
      case 21:temp = 0x3fffff;break;
      case 22:temp = 0x7fffff;break;
      case 23:temp = 0xffffff;break;
      case 24:temp = 0x1ffffff;break;
      case 25:temp = 0x3ffffff;break;
      case 26:temp = 0x7ffffff;break;
      case 27:temp = 0xfffffff;break;
      case 28:temp = 0x1fffffff;break;
      case 29:temp = 0x3fffffff;break;
      case 30:temp = 0x7fffffff;break;
      case 31:temp = 0xffffffff;break;
      case 32:temp = 0x1ffffffff;break;
      case 33:temp = 0x3ffffffff;break;
      case 34:temp = 0x7ffffffff;break;
      case 35:temp = 0xfffffffff;break;
      case 36:temp = 0x1fffffffff;break;
      case 37:temp = 0x3fffffffff;break;
      case 38:temp = 0x7fffffffff;break;
      case 39:temp = 0xffffffffff;break;
      case 40:temp = 0x1ffffffffff;break;
      case 41:temp = 0x3ffffffffff;break;
      case 42:temp = 0x7ffffffffff;break;
      case 43:temp = 0xfffffffffff;break;
      case 44:temp = 0x1fffffffffff;break;
      case 45:temp = 0x3fffffffffff;break;
      case 46:temp = 0x7fffffffffff;break;
      case 47:temp = 0xffffffffffff;break;
      case 48:temp = 0x1fffffffffff;break;
      case 49:temp = 0x3fffffffffff;break;
      case 50:temp = 0x7fffffffffff;break;
      case 51:temp = 0xffffffffffff;break;
      case 52:temp = 0x1fffffffffff;break;
      case 53:temp = 0x3fffffffffff;break;
      case 54:temp = 0x7fffffffffff;break;
      case 55:temp = 0xffffffffffff;break;
      case 56:temp = 0x1ffffffffffff;break;
      case 57:temp = 0x3ffffffffffff;break;
      case 58:temp = 0x7ffffffffffff;break;
      case 59:temp = 0xfffffffffffff;break;
      case 60:temp = 0x1fffffffffffff;break;
      case 61:temp = 0x3fffffffffffff;break;
      case 62:temp = 0x7fffffffffffff;break;
      case 63:temp = 0xffffffffffffff;break;
    }
  }
  return temp;
}


///////////////////////////////////////////////////////////////////////////////
//                        RETRIEVE DATA                                      //
///////////////////////////////////////////////////////////////////////////////
//utility function to long rotat a 1 << indx, mikroc useless at crossing an int
//even though the devce is 32bit :(
unsigned long add1toindex(unsigned long val,unsigned long indx){
unsigned long temp = 1;
   temp  <<= indx;
   return val += temp;
}

//lower value but greater precision after the decimal place, max val +/-65534.9
float ConvX2Float(unsigned int x, unsigned y){
#ifdef DebugFloat
char txt[15];
#endif

unsigned short sign=0;
long exp = 0;
volatile unsigned long rotmant = 0,rotexp=0,i=0,mask = 0;
unsigned long val23 = 0,val32 = 0;
float valFA=0.0,valFB = 0.0,valFC = 0.0,j=0.0;

   val32 = ((unsigned long)x << 16) | y;
   mask = MANTISSA_MASK;
   
   if(val32 == 0)return 0.0;
   //test the sign bit
   sign = (val32 & SIGN_MASK)? 1:0;
   //extract the exponent
   exp  = (val32 >> MANTISSA) & 0x00FF;
   //subtract bias from exponent
   exp  = exp - BIAS;
   //if value is going to greater than 65534.000 then
   //clamp it to 65534 by value and exponent as plc send error
   //value of low number with exp == 16
   if(exp > 15){
     exp = 15;
     val32 = 0x477FFE3C;
   }
   if(exp >= 0){
     rotexp  = MANTISSA - exp;
     rotmant = rotexp - 1;
     mask >>= rotmant;
     val23   = ( val32 & MANTISSA_MASK ) >> rotexp;
     val23 = add1toindex(val23,exp);
     //val23  |= 1<<exp;
   }
   else {
     rotexp  = MANTISSA - -exp ;
     rotmant = MANTISSA_SUB1;
     val32   = ( val32 & MANTISSA_MASK ) >> -exp ;
     val32 = add1toindex(val32,rotexp);
   }

   j=1.0;
   for((i=(rotmant));i>0;i--){
     if((val32 >> i)& 1){//val32 & (1<<i)){
       valFA += 1.0/pow(BASE2,j);
     }
     j+=1.0;
   }
   valFB += (float)(val23 & mask);
   valFB += valFA;
   if(sign)
      valFB = -valFB;

   return valFB;
}

//standard implimentation of IEEE 745 float max value ? 3.4028235 × 10^38.
//~340282380.0  at this length the decimal place is lost and thus precision
//after decimal place no longer applies. the precision after the deicmal
//place is rounded and succeptible to loss in precision.
float _ConvX2Float(unsigned int x, unsigned y){

unsigned short sign=0;
long exp = 0;
unsigned long rotmant = 0,rotexp=0,i=0,mask = 0;
unsigned long val23 = 0,val32 = 0;
float valFA=0.0,valFB = 0.0,valFC = 0.0,j=0.0;

   val32 = ((unsigned long)x << 16) | y;
   mask = MANTISSA_MASK;

   if(val32 == 0)return 0.0;
   //test the sign bit
   sign = (val32 & SIGN_MASK)? 1:0;
   //extract the exponent
   exp  = (val32 >> MANTISSA) & 0x00FF;
   //subtract bias from exponent
   exp  = exp - BIAS;
   
   rotexp  = MANTISSA - exp;
   rotmant = rotexp - 1;
   mask >>= rotmant;
   val23   = ( val32 & MANTISSA_MASK ) >> rotexp;

   j=1.0;
   for(i=22;i>0;i--){
     if((val32 >> i)& 1){
       valFA += 1.0/pow(BASE2,j);
     }
     j+=1.0;
   }
   valFB = 1.0;
   valFB += valFA;
   if(sign)
      valFB = -valFB;

   valFB *= pow(2,(float)exp);

  return valFB;
}

unsigned long ConvFloat2Bin(float val){

unsigned long temp1 = 0,temp2 = 0,j=0;
unsigned long integral=0,remainder=0;
unsigned long binary_num=0,exp=0;
double fractional=0.0,tempintegral;
long k=0;
  fractional = fabs(val);
  fractional = modf(fractional,&tempintegral);
  
  integral  = (unsigned long)tempintegral;

  j=0;
  while (integral > 0) {
     remainder = integral % 2;
     integral /= 2;
     if(integral == 0)break;
     if(remainder != 0){
          binary_num |= 1<<j;
     }
     j++;
  }
  exp = 127+j;
  temp1 |= exp;
  temp1 <<= MANTISSA;
  temp2 = binary_num;
  temp2 <<=  MANTISSA - j;
  temp1 |= temp2;

  k = j + 9;
  k -= 32;
  tempintegral = 0;
  while(k++ < 0){
     if(k >= 0)break;
     fractional *= 2;
     fractional = modf(fractional,&tempintegral);
     temp2 |= (tempintegral >= 1.0)? 1:0;
     temp2 <<= 1;
  }
  temp1 |= temp2 & 0xFFFF;
  if(val < 0.0)temp1 |= 0x80000000;
  return temp1;
}

////////////////////////////////////////////////////////////////////////////////
//parse char data as offsets from a modbus recieve / request instruction
void get_Data(uint16_t offset,void *args){
CONV_FLOAT l2f;
volatile char str[30];
int i,j,k;
static unsigned long tempL = 0;
static float flt = 0.0;

  switch(offset){
     case TEXT:
          //test {18533,27756,28416}
          for(i=0,j=0;i<15;i++){
              *((char*)args++) = (regs.wr_reg[offset+i]>>8);
              *((char*)args++) = regs.wr_reg[offset+i];
              //args+=2;
          }
          break;
     case IVAR:    case(IVAR+1):case (IVAR+2):case (IVAR+3):case (IVAR+4):
     case (IVAR+5):case(IVAR+6):case (IVAR+7):case (IVAR+8):case (IVAR+9):
          sprintf((char*)args,"%d",regs.wr_reg[offset]);
          break;
     case WFVAR:case (WFVAR+2):case (WFVAR+4):case (WFVAR+8):
          //3f48 f5c3
          //regs.wr_reg[offset] = 39322;regs.wr_reg[offset+1] = 17138;
          flt = _ConvX2Float(regs.wr_reg[offset+1],regs.wr_reg[offset]);
          gvars.wr_floats[offset - WFVAR] = flt;
          sprintf((char*)args,"%08.3f",flt);
          break;
  }
}

uint16_t get_Var(uint16_t offset){
  return regs.wr_reg[offset];
}

uint8_t get_Bits(uint16_t offset){
  return regs.wr_coils[offset];
}

////////////////////////////////////////////////////////////////////////////////
//sets modbus registers with a float value to return on request
void set_Float(uint16_t offset,float flt){
unsigned long tempL = ConvFloat2Bin(flt);
  regs.wr_reg[offset] = LoWord(tempL);
  regs.wr_reg[offset+1] = HiWord(tempL);

}


uint16_t modbus_DataConditioning(uint8_t *mbArr,uint16_t data_len){
 ModbusTCP *pdu = &modbus;
 uint8_t prefix[15] = {0}; 
 uint8_t reg[256] = {0};
 uint8_t err = 0;
 uint16_t i;
 static uint16_t last_len = 0,len = 0; 
      //something wrong with socket
      if (data_len < 8) {
        return 0;
      }
 
      // get the request prefix
      if(pdu == NULL)
           pdu = &modbus;
 
      len =  sizeof(MBAP);
      //extract MBAP header [ID,xx,xx,x,FC]
      memcpy(prefix,mbArr,len);
    /*  for(i = 0 ; i < len ; i++) {
        prefix[i] = mbArr[i];
      }*/
      //configure and sort out FC and check for errors
      pdu = test_MBAP(prefix);
      
      if (pdu->error == MB_OK) {
      //FC to get the next number of bytes
        if(pdu->mbap.FC == O15 | pdu->mbap.FC == O16){
            memcpy(prefix+len,mbArr+len,5);
            len += 5;
          /*for(i=len ; i < (len+5) ; i++) {
             prefix[i] = mbArr[i];
          }*/
        }
        else{
            memcpy(prefix+len,mbArr+len,4);
            len += 4;
          /* for(i=len ; i < (len+4) ; i++) {
             prefix[i] = mbArr[i] ;
          }*/
        }
        //index at end of MBAP + SA + DL +/- BL

        //sort out start add and length etc
        pdu = test_PDU(pdu,prefix);
      }

     //  i = 8 + 4/5 => 12/13  [MBAP  + StartAdd + DataQuantity + ByteNo]
     //  12/13 - 6/7 => 6 bytes
      if (pdu->error == MB_OK) {
          
        if(pdu->mbap.FC != O5 & pdu->mbap.FC != O6){
            memcpy(reg,mbArr+len,(pdu->pdu.RegQuantity*2));
          /*for(i=0 ; i < pdu->pdu.RegQuantity*2; i++) {
            reg[i] = mbArr[len+i] ;
          }*/
        }
        
        last_len = set_Data(pdu,reg);
        
        //response to client
        len = get_Response(pdu,prefix);
        memcpy(mbArr,prefix,len);
        
        if(pdu->mbap.FC == O1 | pdu->mbap.FC == O3 | pdu->mbap.FC == O4){
             memcpy(mbArr+len,reg,last_len);
        }
   
        return  len+last_len; // return to the library with the number of bytes to transmit
      }else {                // Return error
        prefix[4] = 0;
        prefix[5] = 3;      // Update to number of bytes in reply
        prefix[7] += 0x80;  // Error
        prefix[8] = err;
        memcpy(mbArr,prefix, 9);

        return 9; // return to the library with the number of bytes to transmit
      }
    
   return 0; // return to the library with the number of bytes to transmit
}


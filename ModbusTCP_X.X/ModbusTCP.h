/* ************************************************************************** */
/** Descriptive File Name

  @Pvt
 David Coetzee

  @File Name
 ModbusTCP.h

  @Summary
 *Exposes ModbusTCP data and functions needed to implement 
 * ModbusTCP into any MPLABX application. 

  @Description
 * ModbusTCP is a Industrial protocol which communicates on an 
 * OT application layer to PLC's that implement Modbus, most PLC's
 * implement Modbus as it is the only free standard that shares all
 * documentation.
 */
/* ************************************************************************** */

#ifndef _MODBUS_TCP_H    /* Guard against multiple inclusion */
#define _MODBUS_TCP_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdint.h>
#include <stdarg.h>




/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

#define reverse(a,b) ((a<<8)|b)
#define Lo(a) ((a)& (0xFF)
#define Hi(a) ((a>>8)&0xFF)
#define LoWord(a) ((a)&0xFFFF)
#define HiWord(a) ((a>>16)&0xFFFF)
    /* ************************************************************************** */
    /** Descriptive Constant Name

      @Summary
        Brief one-line summary of the constant.
    
      @Description
        Full description, explaining the purpose and usage of the constant.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>

    
      @Remarks
        Any additional remarks
     */

    




////////////////////////////////////////////////////
//types and defines
/*defines how long the data packet should be*/
#define MAX_DATA_LENGTH 249//18
#define MBAP_LEN        8
#define PDU_LEN         4
#define MBAP_UID_FC     3


////////////////////////////////////////////////////
//calculations for floats
#define BASE2 2.0
#define BIAS  127
#define MANTISSA 23
#define MANTISSA_SUB1 22
#define MANTISSA_MASK 0x7FFFFF
#define SIGN_MASK 0x80000000
    
///////////////////////////////////////////////////
//error codes
#define MB_OK        0                     //No error
#define IllegalFC    1                     //FC is unknown to server
#define IllegalADD   2                     //Illegal data address
#define IllegalDATA  3                     //Illegal data value
#define ServerFAIL   4                     //Server failed during execution of request
#define ServerACK    5                     //Server requires a long time to respond
#define ServerBUSY   6                     //Server unable to respond
#define GatewayUnAvailable 0x0A,           //Gateway paths not available
#define GatewayTargetResponse 0x0B        //targeted device failed to respond
 
/*************************************************************/    
/*
   Summary:
     Application specific defines need for RGB matric, need to make this dynamic
 
    Description
        This is very specific to RGB screen data, would like to
        to make this a lot more portable in the future
 */
//offsets for various data types

/*
//setup features
#define POSX          0
#define POSY          1
#define ENDX          2
#define ENDY          3
#define ROTDIR        4
#define ROTSPEED      5
#define FONT_TYPE     6
#define BITMAP        7
#define BITMAP_Q      8
#define SHAPE         9
#define SHAPE_Q       10
#define SHAPE_WEIGHT  11
#define BORDER_WEIGHT 12
#define COLOUR        13 //unsigned long value
#define SHAPE_COLOUR  14
*/

//offsets
#define SETUP        0
#define TEXT         23    //40 characters to display on screen     1byte
#define IVAR         63    //10 Holding registers                   2bytes
#define WFVAR        83    // 2 write floating point regesters      4bytes
#define RFVAR        91    // 2 read only floating point registers  4bytes
    
    
    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************


////////////////////////////////////////////////////
//globals
    typedef struct{
    uint8_t mb_bits[4];
    float wr_floats[10];
    int16_t xyz_vals[19];
    }GLOBAL_VARS;
    extern volatile GLOBAL_VARS gvars;
    
    
    // *****************************************************************************

    /** Descriptive Data Type Name

      Summary
        MBAP Header of Modbus Protocol.
    
      Description
        ModbusTCP implements MPAB header for data targeting
        ? Transaction Identifier - It is used for transaction pairing, the MODBUS server copies
            in the response the transaction identifier of the request.
        ? Protocol Identifier ? It is used for intra-system multiplexing. The MODBUS protocol
            is identified by the value 0.
        ? Length - The length field is a byte count of the following fields, including the Unit
            Identifier and data fields. 
        ? Unit Identifier ? This field is used for intra-system routing purpose. It is typically
            used to communicate to a MODBUS+ or a MODBUS serial line slave through a
            gateway between an Ethernet TCP-IP network and a MODBUS serial line. This field is
            set by the MODBUS Client in the request and must be returned with the same value in
            the response by the server.

     */
    typedef struct{
      uint16_t TransID;
      uint16_t ProtocolID;
      uint16_t Length; //from unit ID to data field
      uint8_t UnitID;
      uint8_t FC;
    }__attribute__((packed))MBAP;

    
    /** Descriptive Data Type Name

      Summary
        PDU The function code must be verified and the MODBUS response format analyzed
        according to the MODBUS Application Protocol: 
    
      Description
        ModbusTCP PDU
     *  The Function Code (read Analog Output Holding Registers)
     *  Data Address of the first register requested. (40108-40001 = 107 =6B hex)
     *  The total number of registers requested. (read 3 registers 40108 to 40110)

     */ 
    typedef struct{
     uint16_t StartingAddr;
     uint16_t RegQuantity;
     uint8_t ByteCount;
    }__attribute__((packed))PDU;


    typedef struct{
     MBAP mbap;
     PDU pdu;
     unsigned short error;
    }__attribute__((packed))ModbusTCP;

 
  /** Descriptive Data Type Name

  Summary
   Registers used in Modbus application

  Description
   * Modbus Registers , this is not a common implimentation
   * of these registers, adjust as needed 

 */ 
    typedef struct{
     uint8_t      wr_coils[200];
     uint16_t     rd_reg[100];
     uint16_t     wr_reg[100];
    }ModbusREG;


    /* Descriptive Data Type Name

      Summary
     Various converters used to sort out byte order etx
    
      Description
     Byte alignment and int from char etc
     */ 
    typedef union{
     uint8_t valC[2];
     uint16_t valUI;
     int16_t valI;
    }CONV_INT;

    typedef union{
     uint8_t valS[8];
     uint16_t   valI[4];
     uint32_t  valL[2];
     uint64_t valLL;
    }CONV_LONG;

    typedef union{
    uint32_t valL;
    float valF;
    }CONV_FLOAT;

    /** Descriptive Data Type Name

      Summary
        Modbus response Function codes
    
      Description
        * The function code must be verified and the MODBUS response format analyzed
            according to the MODBUS Application Protocol:
        ? if the function code is the same as the one used in the request, and if the
            response format is correct, then the MODBUS response is given to the user
            application as a Positive Confirmation.
        ? If the function code is a MODBUS exception code (Function code + 80H), the
            MODBUS exception response is given to the user application as a Positive
            Confirmation.
        ? If the function code is different from the one used in the request (=non
            expected function code), or if the format of the response is incorrect, then an
            error is signaled to the user application using a Negative Confirmation.
     */ 
    typedef enum{
    O1=1,   //Read Output coils
    O2,     //Read Inputs
    O3,     //Read Holding Reg
    O4,     //Read Input Reg
    O5,     //Force single Output coil
    O6,     //Preset single reg
    O7,     //Read Exception Status
    O8,     //Diagnosis
    O9,     //
    O10,    //
    O11,    //Fetch comm event counter
    O12,    //Fetch comm event log
    O13,    //Program controller
    O14,    //Poll controller
    O15,    //Force multiple Output coils
    O16,    //Preset multiple reg
    O17,    //Report Slave ID
    O18,    //
    O19,    //Reset comm linl
    O20,    //Read General reference
    O21,    //Write General Reference
    O80=0x80     //ERROR
    } FunctionCodes;
    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
void Modbus_Configure();

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
ModbusTCP* test_MBAP(uint8_t* mbap);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
ModbusTCP* test_PDU(ModbusTCP *_tcp,uint8_t *reqs);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint16_t set_Data(ModbusTCP *_pdu,uint8_t *resp);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint16_t get_Response(ModbusTCP* pdu_,uint8_t* resp);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
void get_Data(uint16_t offset,void* args);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint16_t get_Var(uint16_t offset);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint8_t set_Bits(uint8_t bits,uint16_t offset);

    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint8_t get_Bits(uint16_t offset);
    // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
void set_Float(uint16_t offset,float flt);

   // *****************************************************************************
    /**
      @Function
       void Modbus_Configure(void)

      @Summary
        Initialize common data used by Modbus.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
uint16_t modbus_DataConditioning(uint8_t *mbArr,uint16_t data_len);
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */

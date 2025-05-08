#include <stdint.h>
#include <string.h>  
#include <stdbool.h>
#include <math.h>
#include "bms.h"
#include <cross_studio_io.h>

BMSData_t bmsData;
FailureStatus alarm;


void bmsinit(void) {
   memset(&bmsData, 0, sizeof(bmsData)); // Fill bytes with 0s
// Set up the transmit buffer with constant values
   bmsData.txBuffer[0] = 0xA5; // Start byte
   bmsData.txBuffer[1] = 0x40; // Host address
   bmsData.txBuffer[3] = 0x08; // Length
}

uint8_t bmsGetCommandString(int cmdId) {
    switch (cmdId) {
        case 1:  return 0x90;
        case 2:  return 0x91;
        case 3:  return 0x92;
        case 4:  return 0x93;
        case 5:  return 0x94;
        case 6:  return 0x95;
        case 7:  return 0x96;
        case 8:  return 0x97;
        case 9:  return 0x98;
        default: return 0x00;  // Invalid command ID
    }
}

void bmsCreateCommand(int cmdID) {
    bmsData.txBuffer[2] = cmdID;
    uint8_t checksum = 0;
    // Calculate checksum
    for (int i = 0; i < XFER_BUFFER_LENGTH - 1; i++) {
        checksum += bmsData.txBuffer[i];
    }
    bmsData.txBuffer[XFER_BUFFER_LENGTH - 1] = checksum;
   
}

void bmsCreateTxPacket(void) {
    static int cmdID = 1;
    bmsCreateCommand(bmsGetCommandString(cmdID));
    cmdID++;
    if(cmdID == maxID) {
    cmdID = 1;
    }
}
// SOC of voltage and current
void bmsSOCofvoltageandcurrent(void) {

    // Extract data from response buffer
    
    bmsData.cumulativevoltage = ((float)(((uint16_t)bmsData.rxBuffer[4] << 8) | bmsData.rxBuffer[5]) / 10.0f);
    bmsData.gathervoltage = ((float)(((uint16_t)bmsData.rxBuffer[6] << 8) | bmsData.rxBuffer[7]) / 10.0f);
    // Current has an offset of 30000 (per Daly BMS specification)
  
    bmsData.Current = ((float)((((uint16_t)bmsData.rxBuffer[8] << 8) | bmsData.rxBuffer[9]) - 30000) / 10.0f);
    bmsData.SOC = ((float)(((uint16_t)bmsData.rxBuffer[4] << 8) | bmsData.rxBuffer[11]) / 10.0f);
   // debug_printf("cumu vol : %.2f\n gather vol : %.2f\n current : %.2f\n soc : %.2f\n", bmsData.cumulativevoltage,bmsData.gathervoltage, bmsData.Current,bmsData.SOC);
}

// maximum and minimum voltage

void bmsMinMaxCellVoltage(void) {
    // Extract values from response buffer
    
    bmsData.maxCellmV = (float)(((uint16_t)bmsData.rxBuffer[4] << 8) | bmsData.rxBuffer[5]);
    bmsData.maxCellVNum = bmsData.rxBuffer[6];
    bmsData.minCellmV = (float)(((uint16_t)bmsData.rxBuffer[7] << 8) | bmsData.rxBuffer[8]);
    bmsData.minCellVNum = bmsData.rxBuffer[9]; 
  //  debug_printf("max cell v :%.2f\n max cell num :%d\n min cel v :%.2f\n min cell num : %d\n",bmsData.maxCellmV,bmsData.maxCellVNum, bmsData.minCellmV,bmsData.minCellVNum); 
}
void bmsMinmaxtemp(void) {
    // an offset of 40 is added to BMS to avoid negative values
    bmsData.tempMax = (bmsData.rxBuffer[4]-40);
    bmsData.tempMin = (bmsData.rxBuffer[6]-40);
    bmsData.maxcelltempnum = bmsData.rxBuffer[5]; 
    bmsData.mincelltempnum = bmsData.rxBuffer[7];
   //debug_printf("max temp : %d\n min temp :%d\n max cell num : %d\n min cell num :%d\n",bmsData.tempMax,bmsData.tempMin,bmsData.maxcelltempnum, bmsData.mincelltempnum);

}

void bmsChargedischargeMOSstatus(void) {
    switch(bmsData.rxBuffer[4]) {
        case 0:
         strcpy(bmsData.ChargeDischargeState,"Stationary");
         break;
        case 1:
        strcpy(bmsData.ChargeDischargeState,"Charging");
         break;
        case 2:
        strcpy(bmsData.ChargeDischargeState,"Discharging");

    }
    bmsData.chargemosState = bmsData.rxBuffer[5];
    bmsData.disChargemosState = bmsData.rxBuffer[6];
    bmsData.bmslife = bmsData.rxBuffer[7];
    bmsData.resCapacitymAh = (((uint32_t)bmsData.rxBuffer[8]<< 24) | ((uint32_t)bmsData.rxBuffer[9]<<16) | ((uint32_t)bmsData.rxBuffer[10]<<8) | ((uint32_t)bmsData.rxBuffer[11]));
    //debug_printf("Charge/Discharge State: %s\n", bmsData.ChargeDischargeState);
    //debug_printf("Charge MOS State: %s\n", bmsData.chargemosState ? "true" : "false");
    //debug_printf("Discharge MOS State: %s\n", bmsData.disChargemosState ? "true" : "false");
    //debug_printf("remain capacity : %d\n" ,bmsData.resCapacitymAh );

}
void bmsStatusinfo(void) {
    bmsData.numberOfCells =  bmsData.rxBuffer[4];
    bmsData.numOfTemp = bmsData.rxBuffer[5]; 
    bmsData.chargeState = bmsData.rxBuffer[6];
    bmsData.loadState = bmsData.rxBuffer[7];
    for(size_t i=0;i<8;i++){
        bmsData.dIO[i] = (bmsData.rxBuffer[8]>>1) & 0x01;

    }
    //debug_printf("num of cell : %d\n" ,bmsData.numberOfCells );
    //debug_printf("num of temp : %d\n" , bmsData.numOfTemp );
    //debug_printf("Charge State: %s\n", bmsData.chargeState ? "true" : "false");
    //debug_printf("Discharge State: %s\n", bmsData.loadState ? "true" : "false");
}

void bmsCellVoltages(void) {
    int cellNo = 1;
    bmsData.cellVmV[0] = bmsData.rxBuffer[4];
    // Check valid number of cells
    if ((bmsData.numberOfCells < MIN_NUMBER_CELLS) && (bmsData.numberOfCells >= MAX_NUMBER_CELLS)) {
        return;
    }
     for (size_t i = 0; i <= ceil(bmsData.numberOfCells)/3.0; i++) {
        for (size_t j = 0; j < 3; j++) {
            bmsData.cellVmV[cellNo] = (((uint16_t)bmsData.rxBuffer[5 + i * 2] << 8) | bmsData.rxBuffer[6 + i * 2]);
            
            cellNo++;
           // debug_printf("cell voltage : %f\t, CELL NO : %d\n",bmsData.cellVmV[cellNo], cellNo);
            if (cellNo >= bmsData.numberOfCells) {
                break;
            }
        }   
    }
}

void bmsCelltemperature(void) {
    int tempno = 0;
    if((bmsData.numOfTemp<MIN_NUMBER_TEMP) && (bmsData.numOfTemp>=MAX_NUMBER_TEMP)) {
        return;
    }

    for (size_t i = 1; i <= ceil(bmsData.numOfTemp / 7); i++) {
        for (size_t j = 0; j < 7; j++) {
            bmsData.cellTemperature[tempno] = (bmsData.rxBuffer[5 + j] - 40);
            tempno++;
        //    debug_printf("cell temperature : %f\t,bmsData.cellTemperature[tempno]);
            if (tempno >= bmsData.numOfTemp) {
                break;
            }
        }
    }

}

void bmsCellBalanceState(void) {
   int cellbit = 0;
   if((bmsData.numberOfCells < MIN_NUMBER_CELLS) && (bmsData.numberOfCells >= MAX_NUMBER_CELLS)) {
        return;
   }
        //6 byte response in this command
        for(size_t i=0;i<6;i++) {
            //for each bit in byte
            for(size_t j=0;j<8;j++) {
                bmsData.cellBalanceState[cellbit] = ((bmsData.rxBuffer[i+4]>>j)&0x01);
                cellbit++;
                //debug_printf("cell balance : %f\t",bmsData.cellBalanceState[cellbit]);
                if(cellbit>47) {
                    break;
                }

            }
        }
}

void bmsBatteyrFailureStatus(void) {
    alarm.levelOneCellVoltageTooHigh = (bmsData.rxBuffer[4]>>0)&0x01;
    alarm.levelTwoCellVoltageTooHigh = (bmsData.rxBuffer[4]>>1)&0x01;
    alarm.levelOneCellVoltageTooLow = (bmsData.rxBuffer[4]>>2)&0x01;
    alarm.levelTwoCellVoltageTooLow = (bmsData.rxBuffer[4]>>3)&0x01;
    alarm.levelOneSumVoltageTooHigh = (bmsData.rxBuffer[4]>>4)&0x01;
    alarm.levelTwoSumkVoltageTooHigh = (bmsData.rxBuffer[4]>>5)&0x01;
    alarm.levelOneSumVoltageTooLow = (bmsData.rxBuffer[4]>>6)&0x01;
    alarm.levelTwoSumVoltageTooLow = (bmsData.rxBuffer[4]>>7)&0x01;

    alarm.levelOneChargeTempTooHigh = (bmsData.rxBuffer[5]>>0)&0x01;
    alarm.levelTwoChargeTempTooHigh = (bmsData.rxBuffer[5]>>1)&0x01;
    alarm.levelOneChargeTempTooLow = (bmsData.rxBuffer[5]>>2)&0x01;
    alarm.levelTwoChargeTempTooLow = (bmsData.rxBuffer[5]>>3)&0x01;
    alarm.levelOneDischargeTempTooHigh = (bmsData.rxBuffer[5]>>4)&0x01;
    alarm.levelTwoDischargeTempTooHigh = (bmsData.rxBuffer[5]>>5)&0x01;
    alarm.levelOneDischargeTempTooLow = (bmsData.rxBuffer[5]>>6)&0x01;
    alarm.levelTwoDischargeTempTooLow = (bmsData.rxBuffer[5]>>7)&0x01;

    alarm.levelOneChargeCurrent = (bmsData.rxBuffer[6]>>0)&0x01;
    alarm.levelTwoChargeCurrent = (bmsData.rxBuffer[6]>>1)&0x01;
    alarm.levelOneDischargeCurrent = (bmsData.rxBuffer[6]>>2)&0x01;
    alarm.levelTwoDischargeCurrent = (bmsData.rxBuffer[6]>>3)&0x01;
    alarm.levelOneSOCHigh = (bmsData.rxBuffer[6]>>4)&0x01;
    alarm.levelTwoSOCHigh = (bmsData.rxBuffer[6]>>5)&0x01;
    alarm.levelOneSOCLow = (bmsData.rxBuffer[6]>>6)&0x01;
    alarm.levelTwoSOCLow = (bmsData.rxBuffer[6]>>7)&0x01;

    alarm.levelOneCellVoltageDifference = (bmsData.rxBuffer[7]>>0)&0x01;
    alarm.levelTwoCellVoltageDifference = (bmsData.rxBuffer[7]>>1)&0x01;
    alarm.levelOneTempSensorDifference = (bmsData.rxBuffer[7]>>2)&0x01;
    alarm.levelTwoTempSensorDifference = (bmsData.rxBuffer[7]>>3)&0x01;

    alarm.chargeMOSTemperatureHigh = (bmsData.rxBuffer[8]>>0)&0x01;
    alarm.dischargeMOSTemperatureHigh = (bmsData.rxBuffer[8]>>1)&0x01;
    alarm.ChargeMOSTemperatureSensor = (bmsData.rxBuffer[8]>>2)&0x01;
    alarm.DischargeMOSTemperatureSensor = (bmsData.rxBuffer[8]>>3)&0x01;
    alarm.ChargeMOSAdhesion = (bmsData.rxBuffer[8]>>4)&0x01;
    alarm.DischargeMOSAdhesion = (bmsData.rxBuffer[8]>>5)&0x01;
    alarm.ChargeMOSOpencircuit = (bmsData.rxBuffer[8]>>6)&0x01;
    alarm.DischargeMOSOpencircuit = (bmsData.rxBuffer[8]>>7)&0x01;

    alarm.AFECollectChip = (bmsData.rxBuffer[9]>>0)&0x01;
    alarm.VoltageCollectDropped = (bmsData.rxBuffer[9]>>1)&0x01;
    alarm.failureOfCellTemperatureSensor = (bmsData.rxBuffer[9]>>2)&0x01;
    alarm.failureOfEEPROM = (bmsData.rxBuffer[9]>>3)&0x01;
    alarm.failureOfRTC = (bmsData.rxBuffer[9]>>4)&0x01;
    alarm.failureOfPrecharge = (bmsData.rxBuffer[9]>>5)&0x01;
    alarm.failureOfCommunicationModule = (bmsData.rxBuffer[9]>>6)&0x01;
    alarm.failureOfIntranetCommunicationModule = (bmsData.rxBuffer[9]>>7)&0x01;

    alarm.CurrentModuleFault = (bmsData.rxBuffer[10]>>0)&0x01;
    alarm.SumVoltageDetectfault = (bmsData.rxBuffer[10]>>1)&0x01;
    alarm.ShortCircuitProtectFault = (bmsData.rxBuffer[10]>>2)&0x01;
    alarm.LowVoltageForbiddenChargeFault = (bmsData.rxBuffer[10]>>3)&0x01;

  

}

// Function to receive response from BMS
uint8_t bmsProcessResponse(uint8_t* data, uint8_t size) {
       uint8_t checksum = 0;
       for(int i = 0; i < 12; i++)
        checksum += data[i];
       if (checksum != data[12])
          return 0;
       else{
       int cmdID = data[2]; 
       switch(cmdID){
            case 0x90 :
                bmsSOCofvoltageandcurrent();
                break;
            case 0x91:
                bmsMinMaxCellVoltage();
                break;
            case 0x92:
                bmsMinmaxtemp();
                break;
            case 0x93:
                bmsChargedischargeMOSstatus();
                break;
            case 0x94:
                bmsStatusinfo();
                break;
            case 0x95:
                bmsCellVoltages();
                break;
            case 0x96:
                bmsCelltemperature();
                break;
            case 0x97:
                bmsCellBalanceState();
                break;
            case 0x98:
                bmsBatteyrFailureStatus();
                break;
            default:
                break;
          }

       }
}

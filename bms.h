#ifndef _BMS_H
#define _BMS_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "uart3.h"
#define XFER_BUFFER_LENGTH 13
#define MIN_NUMBER_CELLS 1
#define MAX_NUMBER_CELLS 6
#define MIN_NUMBER_TEMP 1
#define MAX_NUMBER_TEMP 16
#define maxID 10




//DALYBMS
// Data structure for storing BMS data
typedef struct {
    // data from 0x90 SOC of total voltage current
    float cumulativevoltage; // cumulative total voltage
    float gathervoltage; // gather total voltage
    float Current; // current
    float SOC; //soc

    // data from 0x91 maximum and minimum voltage
    float maxCellmV; //maximum cell voltage value(mV)
    int maxCellVNum; // no. of cell with maximum voltage
    float minCellmV; // minimum cell voltage value(mV)
    int minCellVNum; //no of cell with minimum voltage

    // data from 0x92 maximum and minimum temperature
    int tempMax; //maximum temperature value
    int tempMin; //minimum temperature value
    int maxcelltempnum; //maximum temperature cell no
    int mincelltempnum; //minimum temperture cell no

    // data from 0x93 charge and discharge MOS status
    char ChargeDischargeState[15]; //state
    bool chargemosState; //charge MOS state 
    bool disChargemosState; //discharge MOS status
    int bmslife; //bms life
    int resCapacitymAh; //remain capacity

    // data from 0x94 status information
    int numberOfCells; //no. of battery string
    int numOfTemp; //no. of temperature
    bool chargeState; //chargeer status
    bool loadState; //load status
    bool dIO[8]; //dio

    // data from 0x95 cell voltage
    float cellVmV[48]; //cell voltage

    // data from 0x96 cell temperature
    int cellTemperature[16]; //cell temperature

    // data from 0x97 cell balance state
    bool cellBalanceState[48]; //cell balance state
    uint8_t txBuffer[XFER_BUFFER_LENGTH];//Holds data sent to bms
    uint8_t rxBuffer[XFER_BUFFER_LENGTH];//Holds data rcvd from bms
} BMSData_t;


 typedef struct
    {
        // data from 0x98 battery failure status
        /* 0x00 */
        bool levelOneCellVoltageTooHigh;
        bool levelTwoCellVoltageTooHigh;
        bool levelOneCellVoltageTooLow;
        bool levelTwoCellVoltageTooLow;
        bool levelOneSumVoltageTooHigh;
        bool levelTwoSumkVoltageTooHigh;
        bool levelOneSumVoltageTooLow;
        bool levelTwoSumVoltageTooLow;

        /* 0x01 */
        bool levelOneChargeTempTooHigh;
        bool levelTwoChargeTempTooHigh;
        bool levelOneChargeTempTooLow;
        bool levelTwoChargeTempTooLow;
        bool levelOneDischargeTempTooHigh;
        bool levelTwoDischargeTempTooHigh;
        bool levelOneDischargeTempTooLow;
        bool levelTwoDischargeTempTooLow;

        /* 0x02 */
        bool levelOneChargeCurrent;
        bool levelTwoChargeCurrent;
        bool levelOneDischargeCurrent;
        bool levelTwoDischargeCurrent;
        bool levelOneSOCHigh;
        bool levelTwoSOCHigh;
        bool levelOneSOCLow;
        bool levelTwoSOCLow;

        /* 0x03 */
        bool levelOneCellVoltageDifference;
        bool levelTwoCellVoltageDifference;
        bool levelOneTempSensorDifference;
        bool levelTwoTempSensorDifference;

        /* 0x04 */
        bool chargeMOSTemperatureHigh;
        bool dischargeMOSTemperatureHigh;
        bool ChargeMOSTemperatureSensor;
        bool DischargeMOSTemperatureSensor;
        bool ChargeMOSAdhesion;
        bool DischargeMOSAdhesion;
        bool ChargeMOSOpencircuit;
        bool DischargeMOSOpencircuit;

        /* 0x05 */
        bool AFECollectChip;
        bool VoltageCollectDropped;
        bool failureOfCellTemperatureSensor;
        bool failureOfEEPROM;
        bool failureOfRTC;
        bool failureOfPrecharge;
        bool failureOfCommunicationModule;
        bool failureOfIntranetCommunicationModule;

        /* 0x06 */
        bool CurrentModuleFault;
        bool SumVoltageDetectfault;
        bool ShortCircuitProtectFault;
        bool LowVoltageForbiddenChargeFault;
    } FailureStatus;

extern BMSData_t bmsData;

extern void bmsinit(void);
extern uint8_t bmsProcessResponse(uint8_t* data, uint8_t size);
extern void bmsCreateTxPacket(void);
extern uint8_t bmsProcessResponse(uint8_t* data, uint8_t size);

#endif
#pragma once

#include "Definitions.h"
#include <StructWithData.h>
#include <RealtimeObject.h>

class HidDevice;
class HidDeviceDescr;

#define WM_NFC_COMMAND_DONE WM_USER+8
#define WM_NFC_DEVICE_DETECTION_STATUS WM_USER+9
#define WM_NFC_DEVICE_LOG WM_USER+10
#define WM_NFC_OPERATION_DONE WM_USER+11
#define WM_NFC_DEVICE_ADDED WM_USER+13
#define WM_SWITCHTOMANUFACT WM_USER+14
#define WM_NFC_MARKERROR WM_USER+15
#define WM_TABSELECTED WM_USER+16
#define WM_SELECTNEXTTAB WM_USER+17
#define WM_SELECTNEXTDEVICE WM_USER+18
#define WM_HOTKEYCLICKED WM_USER+19
#define WM_CAMERASTATUSCHANGED WM_USER+20
#define WM_CAMERALOG WM_USER+21
#define WM_CAMERAERROR WM_USER+22

enum
{
	STARTALL_CALIBRATION,
	STARTALL_FACTORYRESET,
	STARTALL_FIRMWARE,
	STARTALL_MIPEX,
	STARTALL_UNITS,
	STARTALL_GETLOG,
	STARTALL_MONITORING,
	STARTALL_LIMITTEST,
	STARTALL_COEFF_O2,
	STARTALL_COEFF_CO,
	STARTALL_COEFF_CH4,
	STARTALL_COEFF_H2S,
	STARTALL_SENSOR_ACCEL,
	STARTALL_SENSOR_STATUS,
	STARTALL_SENSOR_GASRANGE,
	STARTALL_SENSOR_VRANGE,
	STARTALL_LOGTIMEOUT,
	STARTALL_DEVICEMODE,
	STARTALL_ALARMS,
	STARTALL_DISABLEALARM,
	STARTALL_CLEARLOADSTATE,
	STARTALL_MULTIPLEUNITS,
	STARTALL_GETCOEFF,
	STARTALL_COEFF,
	STARTALL_PRECISIONS,
	STARTALL_MIPEXCOMMAND,
	STARTALL_AUTOZERO,
	STARTALL_DCA,
	STARTALL_GASPOS
};

#define HW_LORA 0x01
#define HW_SUBSEC_TIM 0x02
#define HW_RADIO_RAK 0x04
#define HW_RADIO_ZIGBEE 0x08
#define HW_RTC_CLOCK_QUARZ 0x10
#define HW_RTC_CLOCK_LSI 0x20

#pragma pack(push, 1)

struct DEVICE_BASE_SETTINGS
{
	bool bDaylight;
	bool bBeep;
	bool bVibro;
	bool bAlarm;
	BYTE nLogTimeout;
	BYTE btVibroPower;
	BYTE btReserved[256];
	DEVICE_BASE_SETTINGS()
	{
		bDaylight = bBeep = bVibro = bAlarm = 0;
		nLogTimeout = 0;
		btVibroPower = 1;
		memset(btReserved, 0, sizeof(btReserved));
	}
};

#pragma pack(pop)

struct DEVICE_INFO
{
public:
	DEVICE_BASE_SETTINGS base;
	double dExchangeQuality;
	CString strSoftwareVer;
	CString strHardwareVer;
	CString strCPUId;
	CString strSensorId;
	UINT nTime;
	UINT nSerialNo;
	UINT nSWVer;
	bool bInCalibration;
	bool bLoaded;
	BYTE btReplaceCount;
	BYTE btVerControl;
	BYTE btDeviceMode;
	BYTE sw_min;
	BYTE sw_maj;
	BYTE hw_min;
	BYTE hw_maj;
	BYTE btEnabledHW;
	bool bRegVibroLoaded;
	DEVICE_INFO()
	{
		nTime = nSerialNo= 0;
		btReplaceCount = 0;
		btVerControl = 0;
		nSWVer = 0;
		bInCalibration = 0;
		sw_min = sw_maj = hw_min = hw_maj = 0;
		bLoaded = 0;
		btEnabledHW = 0;
		bRegVibroLoaded = 0;
		dExchangeQuality = -1;
	};
	void FillSWVer();
};

#pragma pack(push, 1)

struct GAS_RANGE
{
	WORD wFrom;
	WORD wTo;
	bool bInited;
	BYTE btReserved[32];
	GAS_RANGE()
	{
		wFrom = wTo = 0;
		bInited = 0;
		memset(btReserved, 0, sizeof(btReserved));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)

struct ALARM_INFO
{
	GAS_RANGE ch4Alarm;
	GAS_RANGE coAlarm;
	GAS_RANGE h2sAlarm;
	GAS_RANGE o2Alarm;
	UINT nReserved1;
	UINT nReserved2;
	UINT nReserved3;
	UINT nReserved4;
	bool bLoaded;
	BYTE btReserved[256];
	ALARM_INFO()
	{
		nReserved1 = nReserved2 = nReserved3 = nReserved4 = 0;
		bLoaded = 0;
		memset(btReserved, 0, sizeof(btReserved));
	}
};

#pragma pack(pop)

#pragma pack(push, 1)

struct SENSORS_INFO
{
public:
	float fO2Coeff[19];
	float fCOCoeff[14];
	float fH2SCoeff[14];
	float fAccO2;
	float fAccCO;
	float fAccH2S;
	float fAccMIPEX;
	UINT fCH4Threshold;
	float fCH4K1;
	float fCH4K2;
	float fCH4K3;
	float fCH4K4;
	float fCH4K5;
	float fCH4K6;
	GAS_RANGE ch4Range;
	GAS_RANGE coRange;
	GAS_RANGE h2sRange;
	GAS_RANGE o2Range;
	GAS_RANGE o2VRange;
	GAS_RANGE coVRange;
	GAS_RANGE h2sVRange;
	ALARM_INFO alarms;
	BYTE bO2En;
	BYTE bCOEn;
	BYTE bH2SEn;
	BYTE bCH4En;
	bool bLoaded;
	bool bStatusLoaded;
	bool bGasRangeLoaded;
	float fCoefCH4_Mult[6];
	float fCoefCH4_Mult2[6];
	int nSwitchConc;
	BYTE btReserved[203];
	SENSORS_INFO()
	{
		fCH4Threshold = 0;
		fCH4K1 = fCH4K2 = fCH4K3 = fCH4K4 = fCH4K5 = fCH4K6 = 0;
		bO2En = bCOEn = bH2SEn = bCH4En = 0;
		bLoaded = bStatusLoaded= bGasRangeLoaded=0;
		nSwitchConc = 0;
		for (int i = 0; i < 6; i++)
		{
			fCoefCH4_Mult[i] = 0.0;
			fCoefCH4_Mult2[i] = 0.0;
		}
		memset(btReserved, 0, sizeof(btReserved));
	}
};

#pragma pack(pop)

#define CH4_UNITS_PDK 0x10
#define CO_UNITS_H2 0x20
#define H2S_UNITS_H2 0x40
#define O2_UNITS_H2 0x80

#define GAS_CO 0x01
#define GAS_H2S 0x02
#define GAS_MIPEX 0x04
#define GAS_O2 0x08

#define MIPEX_CH4 1
#define MIPEX_C3H8 2
#define MIPEX_CO2 3

#define CO_COH2S 1
#define CO_CO 2
#define CO_MPC 3
#define CO_H2 0x82

#define SO2_H2S 0x11

#define H2S_H2S 1
#define H2S_CO 2
#define H2S_MPC 3
#define H2S_H2 0x82

#define O2_O2 1
#define O2_H2 8

#pragma pack(push, 1)

struct BASE_COEFF
{
	WORD CoefH2SppmToMg;
	WORD CoefCOppmToMg;
	WORD CoefVolToLEL;
	BYTE O2Chem;
	WORD CoefCHEMppmToMg;
	BYTE btReserved[32];
	BASE_COEFF()
	{
		CoefH2SppmToMg = 0;
		CoefCOppmToMg = 0;
		CoefVolToLEL = 0;
		O2Chem = 0;
		CoefCHEMppmToMg = 0;
		memset(btReserved, 0, sizeof(btReserved));
	}
};

#pragma pack(pop)

#pragma pack(push, 1)

#define DEVOPT_CH4PRESSURE 0x01
#define DEVOPT_SHORTLOG 0x02
#define DEVOPT_MIPEXLOG 0x04
#define DEVOPT_ACCUMULATOR 0x40

#define SKIPSELFTEST_STANDART 0x01
#define SKIPSELFTEST_TRANSPORT 0x02

struct DEVICE_SETTINGS
{
	BASE_COEFF base;
	WORD Led_PWM;
	WORD Vibro_PWM;
	WORD Led_Time;
	WORD Led_Slow_Time;
	WORD BeepOff_TimeOut;
	WORD Wait_TimeOut;
	WORD LedRedSCR_PWM;
	WORD FreezeDeltaTemper;
	WORD FreezeDeltaTime;
	WORD Vref_WarmUp_Time;
	BYTE CH4_Buffer_Term;
	BYTE CH4_Buffer_Time;
	//////////////////////////////////////для 28-й

	BYTE NFCTimeOutDetectSeconds;
	BYTE NFCTimeOutWaitMinutes;
	BYTE BattLow;
	BYTE Log_State;
	BYTE Pressure_State;
	BYTE Life_Time_W;
	WORD FreezeStаtusMask;
	WORD FreezeLimit;
	BYTE LogTimeOut;
	BYTE LogAlarmTimeOut;
	BYTE SensorsUnits;
	BYTE bSensorUnitsResult;
	////////////////////////////////////////для 19-й
	WORD Flash_WriteTime;
	WORD Task_Latency;
	WORD Stop_Time;
	BYTE Mipex_State;

/////////////////////////////// для платы с новой антеной
	WORD SNSRef_ADC;
////////////////////////////// для O2/Chem

// v. 1.95
	BYTE SensorsPrecisions;
// v. 1.99
	BYTE SkipSelfTest;
	BYTE SensorsAutoZero; //XXXXnnnn-O2-CxHx-H2S-CO bits; 0-нет / 1-да
	BYTE AltScreenTime; //0/xx - Нет автовыхода / автовыхода в секундах (<256)
	bool bLoaded;
	int16_t RssiLow; //Порог уровня сигнала Лора для сигнализации
	int8_t SnrLow; //Порог качества сигнала Лора для сигнализации
	BYTE AlarmType;
	WORD LostSec;
	BYTE LostPackets;
	int16_t O2Sim;
	int16_t COSim;
	int16_t H2SSim;
	int16_t CH4Sim;
	BYTE O2ChemScrPos;
	struct
	{
		uint16_t O2;
		uint16_t CO;
		uint16_t H2S;
		uint16_t CH4;
	} ScalePoint;
	BYTE Options;
	BYTE WeekToScale;
	BYTE TransportAlarmOffMin;
	BYTE Unfreeze;
	BYTE btReserved[225];
	DEVICE_SETTINGS()
	{
		memset(this, 0, sizeof(DEVICE_SETTINGS));
	};
};

#pragma pack(pop)

#pragma pack(push, 1)
struct LORA_FREQ
{
	UINT nFreq;
	BYTE nSF;
	BYTE btPower;
	bool bEn;
	BYTE btReserved[32];
	LORA_FREQ()
	{
		bEn = 0;
		btPower = 2;
		nFreq = 867700000;
		nSF = 12;
		memset(btReserved, 0, sizeof(btReserved));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)

struct LORA_RX
{
	uint16_t RX1Delay;
	uint16_t RX2Delay;
	uint32_t RX2Frequency;
	uint8_t RX2SpreadingFactor;
	uint8_t Window; // RX1(0x01) | RX2(0x02)
	BYTE btReserved[32];
	LORA_RX()
	{
		Window = 0;
		RX1Delay = 1000;
		RX2Delay = 2000;
		memset(btReserved, 0, sizeof(btReserved));
	};
	UINT GetSize()
	{
		return sizeof(LORA_RX) - sizeof(btReserved);
	}
};
#pragma pack(pop)

#define OTAA_ENABLED 0x01
#define LORA_CONFIRM 0x02
#define LORA_P2P_ENABLED 0x04

#pragma pack(push, 1)
struct LORA_OTAASettings
{
	uint8_t Joined; // Activation->Status
	uint8_t Options; // Activation->OTAA
	struct {
		uint16_t NumberOfJoins;
		uint16_t AcceptDelay1;
		uint16_t AcceptDelay2;
	} Join;
	uint8_t JoinEUI[8];
	uint8_t DevEUI[8];
	uint8_t Appkey[16];
	BYTE btReserved[32];
	LORA_OTAASettings()
	{
		Options = 0;
		Joined = 0;
		Join.NumberOfJoins = 5;
		Join.AcceptDelay1 = 1000;
		Join.AcceptDelay2 = 2000;
		memset(btReserved, 0, sizeof(btReserved));
	};
	UINT GetSize()
	{
		return sizeof(LORA_OTAASettings) - sizeof(btReserved);
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct LORA_SETTINGS
{
	char strAppKey[64];
	char strNetworkKey[64];
	LORA_FREQ f1;
	LORA_FREQ f2;
	LORA_FREQ f3;
	LORA_RX rx;
	LORA_OTAASettings otaa;
	UINT nAddress;
	WORD nDataPeriod;
	BYTE btBW;
	BYTE btCR;
	bool bLoaded;
	bool bOTAALoaded;
	BYTE btReserved[31];
	LORA_SETTINGS()
	{
		strAppKey[0] = 0;
		strNetworkKey[0] = 0;
		nDataPeriod = 60;
		btBW = btCR = 0;
		nAddress = 0;
		bLoaded = 0;
		bOTAALoaded = 0;
		memset(btReserved, 0, sizeof(btReserved));
	}
};

#pragma pack(pop)

#pragma pack(push, 1)
struct LORA_Info {
	uint32_t FCntUp; // Количество переданных пакетов
	uint32_t FCntDown; // Количество полученных пакетов
	int16_t Rssi; // RSSI
	int16_t Snr; // SNR
};
#pragma pack(pop)

struct WORKSTAT
{
	UINT AlarmTime;
	UINT ScrLEDTime;
	UINT WorkTime;
	UINT StopTime;
	UINT SleepTime;
	UINT RunTime;
	UINT StandByTime;
	UINT TurnOffTime;
	UINT WorkFrozen20Time;
	UINT WorkFrozen10Time;
	UINT WorkFrozen0Time;
	UINT WorkHeat10Time;
	UINT WorkHeat20Time;
	UINT WorkHeat30Time;
	UINT WorkHeat40Time;
	UINT WorkHeatTime;
	BYTE VoltBattPerWeek[104];
	BYTE Reserved[86];
	WORD CRCWorkStat;
	bool bLoaded;
	WORKSTAT()
	{
		bLoaded = 0;
	};
};

struct SENSORS_DATA
{
	WORD O2;
	WORD CO;
	WORD H2S;
	WORD CH4VOL;
	DWORD CH4LEL;
	DWORD Press;
	WORD O2Volt;
	WORD COVolt;
	WORD H2SVolt;
	WORD TempVolt;
	WORD Batt;
	short int SensorTemp;
	short int PressTemp;
	short int CPUTemp;
	CString strCH4F;
	CDateTime time;
	SENSORS_DATA()
	{
		memset(this, 0, sizeof(SENSORS_DATA)-sizeof(CString)-sizeof(CDateTime));
	};
	static double GetVolt(WORD v)
	{
		return 2.5 / 4095 * v * 1000;
	}
};

struct EXCHANGE_SETTINGS
{
	int nGetRetryCount;
	int nSendRetryCount;
	int nRetryWaitMS;
	int nDataWaitMS;
	int nSwitchOffWait;
	int nSwitchOnWait;
	int nBootloaderExitWait;
	EXCHANGE_SETTINGS();
};

struct CALIBRATION
{
	double GasCalbInitOfs[4];
	double GasCalbInitAmp[4];
	double GasCalbVals[4][10];
	double dParamWaitTime;
	double GasCalbRefVal[5];//в исходниках был индекс массива с единицы, поэтому 5, а не 4
	UINT nResult;
	double dCoeffMin[4];
	double dCoeffMax[4];
	double dAmpGasFlowError;
	bool GasCalbSens[5];
	bool GasCalbSensFinished[5];
	bool bCalibZero;
	bool bAll;
	bool bInCalibration;
	CALIBRATION()
	{
		nResult = 0;
		bAll = 0;
		dParamWaitTime = 0;
		dAmpGasFlowError = 7;
		bInCalibration = 0;
	};
};

struct DEVICE_LOG_RECORD
{
	UINT nDateTime;
	UINT nFlag;
	double nTemp;
	double nO2;
	double nCO;
	double nH2S;
	double nCH4;
	double nO2Volt;
	double nCOVolt;
	double nH2SVolt;
	double nPressure;
	double nBattVolt;
	int16_t nRSSI;
	int8_t nSNR;
	BYTE nExt;
	BYTE nFreeze;
	void SetFromMSB(double* to, BYTE* from, int nB);
	CString GetFreezeStatus();
	DEVICE_LOG_RECORD()
	{
		nDateTime = 0;
		nBattVolt = 0;
		nRSSI = nSNR = 0;
		nO2Volt = 0;
		nExt = 0;
		nFreeze = 0;
	};
};

class CNFC_Device;

struct DEVICE_LOG
{
	CNFC_Device* dev;
	UINT LastLogAddr;
	UINT nPageLog;
	int nVersion;
	UINT nResult;
	CArray< DEVICE_LOG_RECORD, DEVICE_LOG_RECORD&> records;
	bool bFromPast;
	bool bShort;
	bool bShort2;
	bool bMIPEXLog;
	bool bShowBadData;
	DEVICE_LOG()
	{
		LastLogAddr = -1;
		bFromPast = 1;
		nPageLog = PageLogNorm;
		bShort = 0;
		nVersion = 1;
		nResult = 0;
		dev = 0;
		bShort2 = 0;
		bMIPEXLog = 0;
		bShowBadData = 0;
	};
	DEVICE_LOG& operator=(const DEVICE_LOG& l)
	{
		LastLogAddr=l.LastLogAddr;
		nPageLog=l.nPageLog;
		records.Copy(l.records);
		bFromPast=l.bFromPast;
		bShort=l.bShort;
		nResult = l.nResult;
		bShort2 = l.bShort2;
		bMIPEXLog = l.bMIPEXLog;
		bShowBadData = l.bShowBadData;
		return *this;
	}
};

struct FIRMWARE_SETTINGS
{
	CString strPath;
	int nResult;
	FIRMWARE_SETTINGS()
	{
		nResult = 0;
	};
};
#define DEFAULT_COMMAND_TRY_COUNT 5

enum
{
	DEST_MIPEX=0,
	DEST_UART,
	DEST_SPI
};

struct ADDITIONAL_PARAMETERS
{
	UINT nFactoryResetGasSel;
	UINT nFactoryResetResult;
	CString strMIPEXCommand;
	CString strLastMIPEXAnswer;
	CString strCommandDescription;
	CString strMipexEnd;
	UINT nSendMIPEXCommandTry;
	UINT nSendMIPEXResult;
	UINT nCommandTryCount;
	UINT nMIPEXCommandId;
	UINT nDelayedMIPEXResult;
	BYTE nMipexDest;
	bool bMonitoringSendF;
	bool bGetOnlySensorCoeff;
	bool bReportNextCommandStatus;
	bool bCheckMIPEXFAnswer;
	bool bRetryMIPEXCommand;
	bool bInMIPEXSend;
	bool bInfoOnly;
	bool bInDelayedMIPEX;
	ADDITIONAL_PARAMETERS()
	{
		nMipexDest = DEST_MIPEX;
		nFactoryResetGasSel = 0;
		nFactoryResetResult = 0;
		bMonitoringSendF = 0;
		bGetOnlySensorCoeff = 0;
		bReportNextCommandStatus = 0;
		bCheckMIPEXFAnswer = 0;
		nSendMIPEXCommandTry = 0;
		bRetryMIPEXCommand = 0;
		nSendMIPEXResult = 0;
		nCommandTryCount = DEFAULT_COMMAND_TRY_COUNT;
		strMipexEnd = "\r";
		nMIPEXCommandId = -1;
		bInMIPEXSend = 0;
		bInfoOnly = 0;
		bInDelayedMIPEX = 0;
		nDelayedMIPEXResult = 0;
	};
};

struct OPERATION_STATUS
{
	UINT nOperation;
	UINT nStatus;
	CString strResult;
};

//struct MIPEX_QUERY
//{
//	CTime tm;
//	CString strCommand;
//	CString strReply;
//};

struct GAS_LIMITS
{
	UINT nGas;
	CString strLimit1;
	CString strLimit2;
};

struct GAS_LIMIT_TEST
{
	UINT nGas;
	UINT nResult;
	bool bTest2;
};

// Type
#define RT_NONE 0 // def
#define RT_LORA 1
#define RT_WIFI 2
#define RT_BLE 3
#define RT_ZIGBEE 4

#pragma pack(push, 1)
struct INTERFACE_SETTINGS {
	uint8_t Standard; // Тип
	uint8_t Module; // Модуль
	uint8_t Protocol; // Протокол
	uint8_t Other; // на будущее
	bool bLoaded;
	INTERFACE_SETTINGS()
	{
		bLoaded = 0;
		Standard = RT_NONE;
		Module = 0;
		Protocol = 0;
		Other = 0;
	}
	UINT GetSize()
	{
		return sizeof(INTERFACE_SETTINGS) - 1;
	}
};
#pragma pack(pop)



// Modul LoRa
#define RM_LORA_INTERNAL 0
#define RM_LORA_SX1272 1 // def
#define RM_LORA_RAK811 2
#define RM_LORA_RAK3172 3
// Modul Wifi
#define RM_WIFI_INTERNAL 0
#define RM_WIFI_ESP32WROOM 1 // def
#define RM_WIFI_ESP8266 2
// Modul BLE
#define RM_BLE_INTERNAL 0
#define RM_BLE_ESP32WROOM 1 // def
// Modul ZigBee
#define RM_ZB_INTERNAL 0
#define RM_ZB_ETRX357 1

// Protocol Lora
#define RP_LORA_IGM_PACK 0 /* def - реализован */
#define RP_LORA_IGM_ASCII 1

// Protocol Wifi
#define RP_WIFI_IGM_PACK 0 /* def - реализован */
#define RP_WIFI_IGM_ASCII 1
#define RP_WIFI_BEACON_SYSTEM 2
// Protocol BLE
#define RP_BLE_IGM_PACK 0 /* def - реализован */
#define RP_BLE_IGM_ASCII 1
// Protocol ZigBee
#define RP_ZB_IGM_PACK 0
#define RP_ZB_IGM_ASCII 1
#define RP_ZB_MODBUS_SPKKPU 2 /* def - реализован MODBUS RTU, Транснефть СПККПУ */

#define WIFI_STATIC_IP (1U<<1) // DHCP / Static IP
#define WIFI_AUTO_CONNECT (1U<<2) // Not / Yes Station connect to the AP on power-up
#define WIFI_TCP_SERVER (1UL<<3)

#pragma pack(push, 1)
struct WIFI_SETTINGS {
	struct {
		uint8_t Channel; // Устанавливается при SoftAP / Читается при подсоединении как хост (на данную реализацию)
		uint8_t OutputPower; // 10.0-19.5 dBm (редактируется с шагом 0.5), пересылка как 195=19.5dBm
		uint16_t nSendDataPeriod;
	} RF;
	struct {
		uint8_t SSID[16];
		uint8_t Options;
		uint8_t Authentication; // None / ...
		uint8_t Password[16]; // "”
		uint8_t reserved[2];
	} AccPoint;
	struct {
		uint8_t Local[16]; // DHCP - читается в виде строки xxx.xxx.xxx.xxx
		uint8_t Remote[16]; // указывается - передавать в виде строки xxx.xxx.xxx.xxx
		uint16_t LPort; // 1112 к примеру (ICF)
		uint16_t RPort; // 1112 к примеру (ICF)
	} IP;
	uint8_t LocalMAC[18]; // local MAC: "94:b5:55:01:69:80" к примеру (r)
	uint16_t TCPServerLPort; // local (r/w)
	int8_t RSSI[6][2];
	bool bLoaded;
	WIFI_SETTINGS()
	{
		memset(this, 0, sizeof(WIFI_SETTINGS));
	}
	UINT GetSize(UINT nSWVer);
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MIPEX_COEFF
{
	uint8_t Backup;
	uint8_t RX;
	uint16_t RT;
	char Serial[8]; //ascii
	uint16_t T[2];
	uint16_t Smin[2];
	uint16_t Kscale[2];
	float Ktgd[2];
	double GPoly0[6]; //Полином LittleEndian (x^0 первый)
	double GPoly1[6]; //Полином LittleEndian (x^0 первый)
	double TPoly[6]; //Полином LittleEndian (x^0 первый)
	uint16_t TABZ[20][2];
	bool bLoaded;
	MIPEX_COEFF()
	{
		Backup=0;
		bLoaded = 0;
	};
	bool IsValid() { return Backup == 0xaa; };
	CString GetSerial()
	{
		char buf[9];
		buf[8] = 0;
		memcpy(buf, Serial, 8);
		return buf;
	}
	UINT GetSize() { return sizeof(MIPEX_COEFF) - 1; }
}; //256b
#pragma pack(pop)


struct DEVICE_SAVED_SETTINGS:public RM_VERSIONEDFILE
{
	CString strDevNoFrom;
	CString strDevVersion;
	DEVICE_BASE_SETTINGS base;
	DEVICE_SETTINGS dev_settings;
	SENSORS_INFO sensors;
	LORA_SETTINGS lora;
	MIPEX_COEFF mipex;
	INTERFACE_SETTINGS interfaces;
	WIFI_SETTINGS wifi;
	CDateTime tmCreate;
	UINT nSWVer;
	bool bUserOnly;
	bool bLoaded;
	bool Save(CFile* f);
	bool Load(CFile* f);
	CString GetDescr();
	DEVICE_SAVED_SETTINGS()
	{
		bLoaded = 0;
		nSWVer = 0;
		bUserOnly = 0;
	}
};

class CNFC_Command;

#define NFC_STATUS_NONE 0
#define NFC_STATUS_HASREADER 0x1
#define NFC_STATUS_DEVICEINSTALLED 0x2

enum
{
	RESULT_ERROR=0,
	RESULT_GOOD,
	RESULT_TERMINATED,
	RESULT_ATTENTION
};

enum device_property
{
	CH4_COEFF,
	PRECISION,
	VBATINLOG,
	LORAOTAA,
	SKIPSELFTEST,
	AUTOZERO,
	ALTSCREENTIME,
	CRC,
	LORACONFIRM,
	LORALOW,
	GASSIM,
	O2CHEMPOS,
	SCALEPOINT,
	RAK811,
	LORAP2P,
	CHPRESSURE,
	EXTENDEDCO,
	WEEKTOSCALE,
	GASPOS,
	CH4_MULT,
	BITLEN16,
	SHORTLOG,
	INTERFACES,
	CH4_MULT_NEW,
	RSSI_PROPERTY,
	CH4_PDK_UNITS,
	MIPEXCOEFF,
	MIPEX_16BIT,
	MIPEX_LASTREAD,
	CH4_BUFFER,
	SKIPSELFTTESTTRANSPORT,
	FREEZE_MASK,
	TRANSPORTALARM,
	FREEZE_IN_LOG,
	ACCUMULATOR
};

enum
{
	DCAS_UNKNOWN,
	DCAS_NOTDONE,
	DCAS_WAIT,
	DCAS_DONE,
	DCAS_EXECUTE
};

struct DELAYED_COMMAND
{
	CString strCommand;
	double dTimeDelay;
	double dTimeDelayFromStart;
	double dT;
	double dTTime;
	double dGasTime;
	double dGasConc;
	UINT nValve;
	UINT nCommandId;
	UINT nStatus;
	bool bT;
	bool bGas;
	bool bTSet;
	CDateTime tmStarted;
	CDateTime tmPlateStarted;
	CDateTime tmDone;
	CDateTime tmGasStarted;
	DELAYED_COMMAND()
	{
		nCommandId = -1;
		dTimeDelay = dTimeDelayFromStart = 0;
		nStatus = DCAS_UNKNOWN;
		bT = 0;
		bGas = 0;
		dT = 25;
		dTTime = 30;
		bTSet = 0;
		nValve = 1;
		dGasConc = 1;
		dGasTime = 1;
	}
};

struct DELAYED_COMMAND_ARRAY :public RM_VERSIONEDFILE
{
	CString strName;
	CArray< DELAYED_COMMAND, DELAYED_COMMAND&> arr;
	UINT nMaxId;
	CDateTime tmStarted;
	DELAYED_COMMAND_ARRAY()
	{
		nMaxId = 1;
	}
	DELAYED_COMMAND_ARRAY& operator=(const DELAYED_COMMAND_ARRAY& a)
	{
		nMaxId = a.nMaxId;
		strName = a.strName;
		arr.Copy(a.arr);
		tmStarted = a.tmStarted;
		return *this;
	}
	bool Save();
	bool Load();
	void RecalcTime();
	CString GetProfileFolder();
};

struct DELAYED_COMMAND_RESULT
{
	CString strCommand;
	CString strResult;
	UINT nCommandId;
	CString strDeviceId;
	CDateTime tm;
};

struct DELAYED_COMMAND_RESULT_ARRAY
{
	CArray< DELAYED_COMMAND_RESULT, DELAYED_COMMAND_RESULT&> arr;
	DELAYED_COMMAND_RESULT_ARRAY& operator=(const DELAYED_COMMAND_RESULT_ARRAY& a)
	{
		arr.Copy(a.arr);
		return *this;
	}
};

class CNFC_Device:public CRealtimeObject
{
	friend class CLongGasMainDlg;
public:
	CNFC_Device(HidDeviceDescr* dev, CWnd* wndNotify, CProgressCtrl* progr);
	virtual ~CNFC_Device();
	bool SendCommand(UINT nCommand, CByteArray* send=0, CByteArray* get=0,bool bForce=0);
	DEVICE_INFO dev_info;
	SENSORS_INFO sensors_info;
	SENSORS_DATA sensors_data;
	DEVICE_SETTINGS dev_settings;
	LORA_SETTINGS lora_settings;
	WORKSTAT work_stat;
	EXCHANGE_SETTINGS exchange_settings;
	CALIBRATION calibration_settings;
	FIRMWARE_SETTINGS firmware_settings;
	DEVICE_LOG log;
	ADDITIONAL_PARAMETERS additional_pars;
	LORA_Info loraInfo;
	CArray< SENSORS_DATA, SENSORS_DATA&> monitoring;
//	CArray< MIPEX_QUERY, MIPEX_QUERY&> MipexData;
	CArray< GAS_LIMITS, GAS_LIMITS&> limits;
	GAS_LIMIT_TEST limitTest;
	INTERFACE_SETTINGS interfaces;
	INTERFACE_SETTINGS interfacesToSet;
	WIFI_SETTINGS wifi;
	MIPEX_COEFF mipex;
	CString strAttention;
	UINT nDeviceNum;
	bool IsHaveProperty(device_property nProperty);
	static bool IsHaveProperty(device_property nProperty,UINT nSWVer);
	bool bInMonitoring;
	bool bPauseMonitoring;
	bool bStopMonitoring;
	bool bMonitorOnce;
	bool IsHasReader()
	{
		return (nDeviceStatus & NFC_STATUS_HASREADER) != 0;
	};
	bool IsDeviceInstalled()
	{
		return (nDeviceStatus & NFC_STATUS_DEVICEINSTALLED) != 0;
	};
	bool StartDeviceDetection();
	bool bManufact;
	bool IsVirtualDevice() { return dev_info.nSerialNo == -1; };
	CString GetSerialNumber();
protected:
	UINT nDeviceStatus;
	UINT nCurrentCommand;
	CDateTime tmLastCommand;
	HidDevice* dev;
	bool SwitchOff();
	bool GetAnswer(bool bSleepAfter=1, UINT nTimeout=2000);
	bool SendBuffer(BYTE* data,BYTE nBlock, bool bSleepAfter=1);
	bool GetBuffer(BYTE nBlock,bool bGetLog=0, UINT nTimeout = 2000);
	bool Write(BYTE* data, UINT nLen);
	BYTE arrRead[64];
	bool SendCountByte(UINT nCount);
	bool SendE140FF01();
	CWnd* wndNotify;
	CProgressCtrl* progressCtrl;
	static DWORD WINAPI CommandThread(void* ptr);
	static DWORD WINAPI DeviceDetectionThread(void* ptr);
	static DWORD WINAPI CalibrationThread(void* ptr);
	static DWORD WINAPI LogThread(void* ptr);
	static DWORD WINAPI FirmwareThread(void* ptr);
	static DWORD WINAPI FactoryResetThread(void* ptr);
	static DWORD WINAPI SetSensorUnitsThread(void* ptr);
	static DWORD WINAPI TestLimitThread(void* ptr);
	static DWORD WINAPI StartMonitoringThread(void* ptr);
	CEvent evRunning;
	CEvent evService;
	CEvent evBreak;
	HANDLE hCommandThread;
	HANDLE hMutex;
	bool IsBreak();
	CArray< CNFC_Command*, CNFC_Command*> commandQueue;
	static bool CompareArrays(float* f1, float* f2, int nCount);
	UINT ConvertFromBin(BYTE* bin, int& nFrom, int nCount);
	bool IsInBootLoader(BYTE& nBootloaderVersion);
	bool ParseDeviceInfo(CByteArray* arr);
	bool ParseSensorsInfo(CByteArray* arr);
	bool ParseGasRange(CByteArray* arr);
	bool ParseSensorVRange(CByteArray* arr);
	bool ParseLastLogAddr(CByteArray* arr);
	bool ParseSensorStatus(CByteArray* arr);
	bool ParseVibroPower(CByteArray* arr);
	bool ParseAlarms(CByteArray* arr);
	bool ParseSettings(CByteArray* arr);
	bool ParseLoraSettings(CByteArray* arr);
	bool ParseLoraKey(CByteArray* arr);
	bool ParseLoraInfo(CByteArray* arr);
	bool ParseSensorsData(CByteArray* arr);
	bool ParseWorkStat(CByteArray* arr);
	bool ParseInterfaces(CByteArray* arr);
	bool ParseCH4Poly(CByteArray* arr);
	bool ParseWifiSettings(CByteArray* arr);
	bool ParseMipexCoeff(CByteArray* arr);
	bool ParseLog(CByteArray* arr, UINT nBlock);
	bool ParseData(CNFC_Command* c);
	bool WriteLog(CString str, UINT nStatus = 0);
	bool SendSwitchBootloader();
	bool ReadOneByte();
	bool bLogEnabled;
	bool SwitchToBootloader(BYTE& nBootloaderVersion,bool bCheckFirst = 0);
	bool SendExitBootloader();
	bool SendBlock0ForBoot(BYTE* buf);
	bool SendBeforeStartFirmware(int n);
	bool WriteC0D0();
	bool GetAnswerFlash();
	bool CheckCRC(CNFC_Command* c);
	bool bLastCommandStatus;
	bool ExitBootloader(int nWaitTime=0);
	int ReaderSequence1();
	int ReaderSequence2();
	int ReaderSequence3();
	void ExchangeSuccess(bool bSuccess);
	CUIntArray arrExchanges;
	void SendFirmwareStatus(int nBlock,int nTotal);
public:
	void OperationStatusChange(UINT nOperation, UINT nStatus);
	HidDevice* GetHidDevice() { return dev; };
	bool SwitchOn();
	bool Blink();
	bool Beep();
	bool IsInCommand();
	bool IsInService();
	bool SetSerialNumber(UINT nNum);
	bool SetLogTimeout(BYTE nTimeout);
	bool Reboot();
	bool ReplaceBattery();
	bool EnableBeep(bool bEnable);
	bool EnableVibro(bool bEnable);
	bool EnableAlarm(bool bEnable);
	bool EnableNFCPowerDown(bool bEnable);
	bool SetDeviceMode(UINT nMode);
	bool SetDateTime(UINT nTime, bool bDaylightSave);
	bool GetDeviceInfo(bool bClearVer=1,bool bOnlyInfo=0,bool bForce=0);
	bool GetAllCoeffs();
	bool GetCoeffsOnly();
	bool GetVibroPower();
	bool GetGasRange();
	bool GetSensorVoltRange();
	bool SetO2Coeff();
	bool SetCOCoeff();
	bool SetH2SCoeff();
	bool SetCH4Coeff();
	bool SetCH4CoeffMult();
	bool SetSensorAccel();
	bool SetGasRange();
	bool SetSensorVRange();
	bool GetSensorStatus();
	bool SetSensorStatus();
	bool SetVibroPower();
	bool GetAlarms();
	bool SetAlarms();
	bool GetSettings();
	bool SetSettings(bool bGet=0);
	bool TestAlarm(UINT nAlarm);
	bool GetLoraSettings();
	bool SetLoraSettings();
	bool GetLoraKey();
	bool SetLoraKey();
	bool GetLoraInfo();
	bool SendLoraPacket();
	bool GetWorkStat();
	bool ResetWorkStat();
	bool SendCommandMIPEX();
	CString GetMIPEXAnswer(CByteArray* arr);
	bool GetSensorData();
	bool WaitComplete(bool bAll=0, bool bIgnoreInputs=1);
	bool BreakExecution();
	bool Calibrate();
	static CString GetCommandString(UINT nCommand);
	void Sleep(UINT nMS);
	bool GetLastLogAddr();
	bool GetLog();
	bool FactoryReset();
	bool ResetLog();
	bool StartFirmware();
	bool SetSensorUnits(bool bGetSettings=1);
	bool SetWeekToScale(BYTE nWeeks);
	bool GetInterfaces();
	bool SetInterfaces();
	bool GetWifiSettings();
	bool SetWifiSettings();
	void CopyUnits(CNFC_Device* from);
	int GetPrecision(UINT nGas);
	bool MipexBackup();
	bool MipexRestore();
	bool MipexGetBackup();
	bool MipexSetBackup(MIPEX_COEFF * coeff);
	CString LoadString(UINT nStringId, bool bUnicode);
	bool IsExtendedUnits(UINT nGas);
	UINT GetExtendedUnitsMask(UINT nGas);
	bool MipexGetLastCommand();
	bool ParseLastMIPEXCommand(CByteArray* arr, DELAYED_COMMAND_RESULT& r);
protected:
	static int InitThread();
public:
	bool GetLoraOTAASettings();
	bool SetLoraOTAASettings();
	bool ParseLoraOTAA(CByteArray* arr);
	bool LoraOTAARejoin();
	CString GetAlarmString(UINT nGas, SENSORS_DATA& data);
	void GetAvailableUnits(UINT nGas, RM_DATABASEIDLIST& l, bool bUnicode = 0);
	void GetAvailableGas(UINT nGas, RM_DATABASEIDLIST& l, bool bShort = 0, bool bUnicode = 0);
	CString GetUnitsString(UINT nGas, bool bUnicode = 0);
	UINT GetUnits(UINT nGas);
	static time_t SystemTimeToTime(SYSTEMTIME& st);
	static void TimeToSystemTIME(UINT nTime, SYSTEMTIME& st);
	static bool IsCorrectDate(UINT nDate);
	bool IsMIPEX_FAnswerCorrect(CString str);
	float GetGasDevider(UINT nGas,bool bForScalePoints);
	bool IsGasSelected(UINT nGas);
private:
	int GetMIPEXCommandBlocksNum();
	UINT GetMIPEXCommandTimeout();
	bool IsBigMIPEXReply(CString strCommand);
public:
	bool IsHasLora();
	bool SetSensorUnitsAll();
	CString GetGasString(UINT nGas,bool bShort=1,bool bUnicode=0);
	CString GetGasStringByVal(UINT nGas, UINT nVal,bool bShort = 1, bool bUnicode = 0);
	BYTE GetSelectedGas(UINT nGas);
	bool TestLimit(UINT nGas, bool bLimit2);
	bool TestIfDeviceChanged();
	void ClearLoadState();
	double RecalcForCalibration(UINT nGas, double dVal);
	bool IsPDK();
	bool UpdateMIPEXCommand(CString& m_Command);
private:

public:
	bool UpdateSensorsData(bool bSendF, CString strFCommand = "");
	bool StartMonitoring(bool bSendF, CString strFCommand = "");
	CDateTime dtLastRepeat;
	DELAYED_COMMAND_ARRAY dca;
	DELAYED_COMMAND_RESULT_ARRAY dcar;
	DELAYED_COMMAND_RESULT_ARRAY dcarMIPEX;
	UINT nCurrentCommandId;
	bool bInMIPEXRepeat;
	CString lastF;
	CDateTime tmMonitoringStart;
	UINT nCurrentOperation;
	UINT nFirmwareProgress;
	UINT nRecordsFrom;
};

class CNFC_Command
{
public:
	CNFC_Device* dev;
	UINT nCommand;
	CByteArray* send;
	CByteArray* get;
	UINT nParam;
	UINT nRet;
	CString strCommandDescription;
	bool bReportCommandStatus;
	CNFC_Command()
	{
		send = 0;
		get = 0;
		nRet = 0;
		nParam = 0;
		bReportCommandStatus = 0;
	};
	~CNFC_Command()
	{
		if (send)delete send;
		if (get)delete get;
	}
};

struct NFC_LOG
{
	CString strLog;
	COLORREF clr;
	CNFC_Device* from;
};

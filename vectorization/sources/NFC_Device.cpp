#include "pch.h"
#include "NFC_Device.h"
#include "HidApi.h"
#include <StructWithData.h>
#include <MainAppTemplate.h>
#include "resource.h"
#include <CRC.h>

CRC_32 g_crc(0xEDB88320);

extern CString g_szCurrentDir;

enum
{
	LOG_OK = 0,
	LOG_ERROR,
	LOG_PROCESSCOMPLETE,
	LOG_MEASURED,
	LOG_ATTENTION,
	LOG_CRCERROR
};

EXCHANGE_SETTINGS::EXCHANGE_SETTINGS()
{
	nGetRetryCount = 5;
	nSendRetryCount = 5;
	nRetryWaitMS = 1000;
	nDataWaitMS = 45;
	nSwitchOffWait =300;
	nSwitchOnWait = 400;
	nBootloaderExitWait = 15;
};

CNFC_Device::CNFC_Device(HidDeviceDescr* dev,CWnd * wndNotify,CProgressCtrl * progr):evRunning(0,1),evBreak(0,1), evService(0,1)
{
	log.dev = this;
	nDeviceStatus = NFC_STATUS_NONE;
	this->dev = new HidDevice;
	this->dev->descr = *dev;
	//arrRead = new BYTE[dev->getReadBufferSize()];
	this->wndNotify = wndNotify;
	hCommandThread = INVALID_HANDLE_VALUE;
	this->dev->evBreak = &evBreak;
	hMutex=CreateMutex(NULL, FALSE, NULL);
	bLogEnabled = 1;
	bManufact = 1;
	progressCtrl = progr;
	nCurrentCommand = -1;
	for (int i = 0; i < 4; i++)
	{
		calibration_settings.dCoeffMin[i] = 0;
		calibration_settings.dCoeffMax[i] = 0;
	}
	bInMonitoring = 0;
	bPauseMonitoring = 0;
	bStopMonitoring = 0;
	bMonitorOnce = 0;
	GAS_LIMITS gl;
	UINT nGas[] = { GAS_MIPEX,GAS_O2,GAS_CO,GAS_H2S };
	for (int i = 0; i < 4; i++)
	{
		gl.nGas = nGas[i];
		limits.Add(gl);
	}
	bLastCommandStatus = 1;
	bInMIPEXRepeat = 0;
	nDeviceNum = 0;
	nCurrentOperation = -1;
	nFirmwareProgress = 0;
	nRecordsFrom = 0;
}

CNFC_Device::~CNFC_Device()//destructor
{
	WaitForSingleObject(hMutex, INFINITE);
	if (IsInCommand() || IsInService())
	{
		evBreak.SetEvent();
		ReleaseMutex(hMutex);
		WaitForSingleObject(hCommandThread, INFINITE);
	}
	else ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	delete this->dev;
	//delete [] arrRead;
}

bool CNFC_Device::CheckCRC(CNFC_Command* c)
{
	if (dev_info.nSWVer && !IsHaveProperty(CRC))return 1;
	DEVICE_INFO prev;
	if (c->nCommand == GetInfDeviceByte)
	{
		prev = dev_info;
		if (!ParseDeviceInfo(c->get))
		{
			dev_info = prev;
			return 0;
		}
		if (!IsHaveProperty(CRC))return 1;
	}
	if (!dev_info.nSWVer)return 1;
	bool bWriteCommand = 0,bReturnArg=0,bReturnOne=0;
	switch (c->nCommand)
	{
	case NFCPowerDownByte:
	case UnlockSignByte:
	case SwitchBeepByte:
	case SwitchVibrByte:
	case SetLogTimeOutByte:

		bWriteCommand = 1;
		bReturnArg = 1;
		break;
	case RebootByte:
	case ModeDevByte:
		bWriteCommand = 1;
		bReturnOne = 1;
		break;
	case SetScaleTime:
	case SetSettingsByte:
	case SendCoefO2Byte:
	case SendCoefCOByte:
	case SendCoefH2SByte:
	case SetCH4Mult:
	case SwitchOFFByte:
	case SetRegVibroByte:
	case TestAlarm1Byte:
	case TestAlarm2Byte:
	case BlinkByte:
	case OneBeepByte:
	case SetGasRangeByte:
	case SetSensStatusByte:
	case SetSensUnitsByte:
	case SetSerialNumberByte:
	case ClearLogByte:
	case SetTimeByte:
	case SetSensVRangeByte:
	case ClearEvLogByte:
	case SetRadioByte:
	case SetLoraOTAA:
	case OTAARejoin:
	case SendLoraKeyByte:
	case SendSensAccelByte:
	case ReplButtByte:
	case NullStatByte:
	case SetAlarmSignalingByte:
	case SendRadioPacketByte:
	case SetInterfacesByte:
	case SetWifiByte:
	case NFCSaveMipexBackup:
		bWriteCommand = 1;
		break;
	}
	int nL = c->get->GetSize();
	if (nL < 32)return 1;
	BYTE* arr =&(*c->get)[0];
	int nSize = arr[8];
	if (IsHaveProperty(BITLEN16) && arr[9] != 0xd1)nSize |= (arr[9] << 8);
	if (bWriteCommand)
	{
		if (arr[20] != c->nCommand)return 0;
		if (bReturnArg && arr[27] != (*c->send)[0])return 0;
		if (bReturnOne && arr[27] != 1)return 0;
		return 1;
	}
	try
	{
		if (nSize < 22)throw 0;
		if (nL < nSize + 11)return 1;
		if (arr[nSize + 10] != 0xfe)throw 0;
		ULONG calc, send = *(UINT*)&arr[nSize + 6];
		g_crc.Calculate2(&arr[27], nSize - 21, calc);
		if (calc == send)return 1;
	}
	catch (int)
	{

	}
	if (c->nCommand == GetInfDeviceByte)dev_info = prev;
	return 0;
}

DWORD WINAPI CNFC_Device::CommandThread(void* ptr)
{
	InitThread();
	CNFC_Command* c = (CNFC_Command*)ptr;
	CString strDescr = GetCommandString(c->nCommand),str;
	while (1)//while command queue not empty
	{
		CNFC_Device* dev = c->dev;
		dev->nCurrentCommand = c->nCommand;
		dev->bLastCommandStatus = 1;
		if (strDescr != "")
		{
			str.Format("%s %s", S_O::LoadString(IDS_EXECUTING), strDescr);
			dev->WriteLog(str);
		}
		int nBlockRead = 2;
		int nTotalLen = 0;
		UINT nTimeout = 2000;
		switch (c->nCommand)
		{
		case NFCMipexRestore:
		case NFCMipexBackup:
			nTimeout = 70000;
			nBlockRead = 2;
			break;
		case SendSensAccelByte:
		case SendCH4CoefByte:
		case SendCoefO2Byte:
		case SendCoefCOByte:
		case SendCoefH2SByte:
		case SetCH4Mult:
		case SetWifiByte:
		case NFCSaveMipexBackup:
		//case NFCMipexBackup:
		//case NFCMipexRestore:
		//case NFCSaveMipexBackup:
			nBlockRead = 1;
			break;
		case NFCSendMipexBackup:
			nBlockRead = 8;
			break;
		case SetSettingsByte:
//			if (dev->IsHaveProperty(GASSIM))nBlockRead = 3;
			break;
//		case GetInfDeviceByte:
		case GetAlarmSignalingByte:
			nBlockRead = 3;
			break;
		case SetRadioByte:
			nBlockRead = 4;
			break;
		case GetDataSensorsByte:
			nBlockRead = 4;
			break;
		case GetSettingsByte:
		case GetLoraKeyByte:
			nBlockRead = 5;
			break;
		case GetWorkStatByte:
			nBlockRead = 7;
			break;
		case GetLastMIPEXCommand:
			nBlockRead = 6;
			break;
		case SetCommandMIPEXByte:
		case SetCommandUARTByte:
		case SetCommandSPIByte:
			nBlockRead = dev->GetMIPEXCommandBlocksNum();
			nTimeout = dev->GetMIPEXCommandTimeout();
			break;
		case GetAllCoefByte:
			nBlockRead = 9;
			break;
		case GetRadioSettingsByte:
			if (dev->interfaces.Standard == RT_WIFI)
			{
				nBlockRead = 3;
				if (dev->IsHaveProperty(RSSI_PROPERTY))nBlockRead++;
			}
			break;
		}
		int nChunk = 192, nLen = 0;
		if (c->send)nTotalLen = c->send->GetSize();
		if (dev->IsHaveProperty(CRC) && nTotalLen)
		{
			CByteArray arr;
			int nChunkCRC = nChunk - 4, nChunks= nTotalLen / nChunkCRC + ((nTotalLen % nChunkCRC) != 0),nSend=0;
			for (int i = 0; i < nChunks; i++)
			{
				if (nSend + nChunkCRC < nTotalLen)nLen = nChunkCRC;
				else nLen = nTotalLen - nSend;
				ULONG nCRC = 0;
				g_crc.Calculate2(&(*c->send)[nSend], nLen, nCRC);
				for (int k = 0; k < nLen; k++)arr.Add((*c->send)[nSend + k]);
				BYTE* b = (BYTE*)&nCRC;
				for (int k = 0; k < 4; k++)arr.Add(b[k]);
				nSend += nLen;
			}
			c->send->Copy(arr);
			nTotalLen = c->send->GetSize();
		}
		dev->dev->ClearReadBuffer();
		int nRepCount = dev->additional_pars.nCommandTryCount,nCRCErrCount=1, nBlockSize = 32;
		for (int nRep = 0; nRep < nRepCount; nRep++)
		{
			try
			{
				int nByteSend = 0, nPackets = 1;
				if (nTotalLen)nPackets = nTotalLen / nChunk + ((nTotalLen % nChunk) != 0);
				int nBlocksTotal = (23*nPackets + nTotalLen + nPackets) / 4, nBlocksSend=0;
				dev->progressCtrl->SetRange(0, nBlocksTotal + 4 * nPackets + nBlockRead+(nPackets-1));
				bool bSuccess = 0;
				for (int nPacket = 0; nPacket < nPackets; nPacket++)
				{
					if (!dev->SwitchOn())throw 0;
					dev->Sleep(dev->exchange_settings.nDataWaitMS);
					bool bFinal = nPacket == (nPackets - 1);
					if (nTotalLen)
					{
						if (nByteSend + nChunk < nTotalLen)nLen = nChunk;
						else nLen = nTotalLen - nByteSend;
					}
					else nLen = 0;
					BYTE* buf = new BYTE[23 + nLen + 16];
					memset(buf, 0, 23 + nLen + 4);
					buf[0] = 0x00;
					buf[1] = 0x00;
					buf[2] = 0x00;
					buf[3] = 0x00;
					buf[4] = 0x03;
					buf[5] = nLen + 0x0A + 0x07;
					// +0x07 так по стандарту, а +0x0A потому что наш пакет состоит из command (2 байта), numpacket (4 байта), Count_byte (4 байта) + buf_data (n_byte)
					buf[6] = 0xD1;
					buf[7] = 0x01;
					buf[8] = nLen + 0x0A + 0x03;
					buf[9] = 0x54;
					buf[10] = 0x02;
					buf[11] = 0x65;
					buf[12] = 0x6E;
					buf[13] = 0x00;
					buf[14] = 0x00;
					buf[15] = 0x00;
					buf[16] = 0x00;
					UINT nCommand = c->nCommand;
					/*if (nCommand == NFCSaveMipexBackup)
					{
						if (nPacket == 1)nCommand = NFCSaveMipexBackup + 1;
						buf[19] = 1;
						buf[20] = 1;
					}
					else*/
					{
						buf[19] = nPacket + 1;
						buf[20] = nPackets;
					}
					buf[17] = LOBYTE(nCommand);
					buf[18] = HIBYTE(nCommand);
					buf[21] = 0;
					buf[22] = 0;
					for (int i = 0; i < nLen; i++)
						buf[i + 23] = (*c->send)[i + nByteSend];
					buf[23 + nLen] = 0xFE;
					int nBlocks = (23 + nLen + 1) / 4;
					for (int i = 0; i <= nBlocks; i++)
					{
						if (!dev->SendBuffer(&buf[i * 4], i))
						{
							delete[] buf;
							throw 0;
						}
						dev->progressCtrl->SetPos(++nBlocksSend);
					}
					delete[] buf;
					if (!dev->SendCountByte(nLen + 6))throw 0;
					dev->progressCtrl->SetPos(++nBlocksSend);
					if (!dev->SendE140FF01())throw 0;
					if (nTotalLen)nByteSend += nLen;
					dev->progressCtrl->SetPos(++nBlocksSend);
					if (!dev->SwitchOff())throw 0;
					dev->Sleep(dev->exchange_settings.nSwitchOffWait);
					dev->progressCtrl->SetPos(++nBlocksSend);
					if (!dev->SwitchOn())throw 0;
					if (bFinal && c->nCommand == ClearLogByte)dev->Sleep(ClearLogTimeOut);
					else dev->Sleep(dev->exchange_settings.nSwitchOnWait);
					dev->progressCtrl->SetPos(++nBlocksSend);
					int nBlockReadFinal = 1;
					if (bFinal)
					{
						nBlockReadFinal = nBlockRead;
						if (!c->get)c->get = new CByteArray;
						int nL = (nBlockReadFinal + 1) * nBlockSize + 4;
						c->get->SetSize(nL);
						memset(&(*c->get)[0], 0, nL);
					}
					for (int i = 0; i <= nBlockReadFinal; i++)
					{
						if (!dev->GetBuffer(i, 0, (i || !bFinal)?2000:nTimeout))
						{
							throw 0;
						}
						if (bFinal)
						{
							for (int k = 0; k < nBlockSize; k++)
							{
								(*c->get)[i * nBlockSize + k + 3] = dev->arrRead[k + 3];
							}
						}
						dev->progressCtrl->SetPos(++nBlocksSend);
					}
					if (!dev->SwitchOff())throw 0;
					if (bFinal)
					{
						if (!dev->CheckCRC(c))
						{
							str.Format(S_O::LoadString(IDS_CRCERRORCOMMAND), nCRCErrCount++);
							dev->WriteLog(str, LOG_CRCERROR);
							throw 0;
						}
						c->nRet = dev->ParseData(c);
						bSuccess = 1;
					}
					dev->ExchangeSuccess(1);
				}
				if (bSuccess)break;
			}
			catch (int n)
			{
				if (!dev->IsBreak())dev->ExchangeSuccess(0);
				if (dev->wndNotify)dev->wndNotify->SendMessage(WM_NFC_MARKERROR, (WPARAM)dev);
				if (dev->IsBreak())break;
				if (nRep == (nRepCount - 1))c->nRet = n;
				else
				{
					dev->SwitchOff();
					dev->Sleep(dev->exchange_settings.nSwitchOffWait);
				}
			}
		}
		if (strDescr != "")
		{
			str.Format("%s", S_O::LoadString(c->nRet?IDS_DONE:IDS_ERROR));
			dev->WriteLog(str, c->nRet ? LOG_OK:LOG_ERROR);
		}
		dev->bLastCommandStatus = c->nRet;
		if (IsWindow(dev->wndNotify->GetSafeHwnd()))
		{
			if (c->nCommand == SetCommandMIPEXByte || c->nCommand == SetCommandUARTByte || c->nCommand == SetCommandSPIByte)dev->additional_pars.bInMIPEXSend = 0;
			dev->wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
		}
		dev->progressCtrl->SetPos(0);
		bool bBreak = 1;
		WaitForSingleObject(dev->hMutex, INFINITE);
		if (dev->commandQueue.GetSize())
		{
			if (dev->IsBreak())
			{
				for (int i = 0; i < dev->commandQueue.GetSize(); i++)
					delete dev->commandQueue[i];
				dev->additional_pars.bInMIPEXSend = 0;
				dev->commandQueue.RemoveAll();
			}
			else
			{
				c = dev->commandQueue[0];
				strDescr = GetCommandString(c->nCommand);
				dev->commandQueue.RemoveAt(0);
				bBreak = 0;
			}
		}
		dev->evBreak.ResetEvent();
		if(bBreak)dev->evRunning.ResetEvent();
		ReleaseMutex(dev->hMutex);
		if (bBreak)break;
	}
	return 1;
}

bool CNFC_Device::IsBreak()
{
	return WaitForSingleObject(evBreak.m_hObject, 0) == WAIT_OBJECT_0;
}

bool CNFC_Device::IsInCommand()
{
	return WaitForSingleObject(evRunning.m_hObject, 0) == WAIT_OBJECT_0;
}

bool CNFC_Device::IsInService()
{
	return WaitForSingleObject(evService.m_hObject, 0) == WAIT_OBJECT_0;
}

bool CNFC_Device::SendCommand(UINT nCommand, CByteArray * send,CByteArray * get, bool bForce)
{
	if (IsVirtualDevice())return 1;
	if (!dev || (!bForce && nCurrentOperation== STARTALL_FIRMWARE))return 0;
	CNFC_Command* c = new CNFC_Command();
	c->dev = this;
	c->nCommand = nCommand;
	c->send = send;
	c->get = get;
	c->bReportCommandStatus = additional_pars.bReportNextCommandStatus;
	c->strCommandDescription = additional_pars.strCommandDescription;
	additional_pars.bReportNextCommandStatus = 0;
	additional_pars.strCommandDescription = "";
	WaitForSingleObject(hMutex, INFINITE);
	if (IsInCommand())
	{
		if (c->nCommand != GetDataSensorsByte)
		{
			bool bGood = 1;
			if (nCurrentCommand == c->nCommand)bGood = 0;
			int nS = commandQueue.GetSize();
			if (nS && commandQueue[nS - 1]->nCommand == c->nCommand)bGood = 0;
			if(!bGood)
			{
				if (c->nCommand == SetCommandMIPEXByte || c->nCommand == SetCommandUARTByte || c->nCommand == SetCommandSPIByte)additional_pars.bInMIPEXSend = 0;
				ReleaseMutex(hMutex);
				delete c;
				return 1;
			}
		}
		commandQueue.Add(c);
	}
	else
	{
		if (c->nCommand != GetDataSensorsByte)
		{
			if (!tmLastCommand.IsNull() && c->nCommand == nCurrentCommand)
			{
				COleDateTimeSpan ts = CDateTime::GetCurrentTime() - tmLastCommand;
				if (ts.GetTotalSeconds() < 2)
				{
					if (c->nCommand == SetCommandMIPEXByte || c->nCommand == SetCommandUARTByte || c->nCommand == SetCommandSPIByte)additional_pars.bInMIPEXSend = 0;
					ReleaseMutex(hMutex);
					delete c;
					return 1;
				}
			}
		}
		evRunning.SetEvent();
		evBreak.ResetEvent();
		nCurrentCommand = c->nCommand;
		hCommandThread = CreateThread(NULL, 0, CommandThread, (void*)c, 0, 0);
	}
	ReleaseMutex(hMutex);
	tmLastCommand = CDateTime::GetCurrentTime();
	return 1;
}

bool CNFC_Device::SwitchOn()
{
	BYTE write_buff[5];
	write_buff[0]= 0x01;
	write_buff[1]= 0x02;
	write_buff[2]= 0x02;
	write_buff[3]= 0x01;
	write_buff[4]= 0x0D;
	if (!Write(write_buff, 5))return 0;
	return GetAnswer(0);
}

bool CNFC_Device::SwitchOff()
{
	BYTE write_buff[5];
	write_buff[0] = 0x01;
	write_buff[1] = 0x02;
	write_buff[2] = 0x02;
	write_buff[3] = 0x00;
	write_buff[4] = 0x00;
	if (!Write(write_buff, 5))return 0;
	return GetAnswer(0);
}

bool CNFC_Device::GetAnswer(bool bSleepAfter,UINT nTimeout)
{
//	memset(arrRead, 0, dev->getReadBufferSize());
	if (IsBreak())return 0;
	memset(arrRead, 0, 64);
	std::string str = dev->read(nTimeout);
	if (str == "")
	{
		return 0;
	}
	memcpy(arrRead, &str[1], (str.size()<63?str.size():63));//first byte is report id
	if(bSleepAfter)Sleep(exchange_settings.nDataWaitMS);
	return 1;
}

bool CNFC_Device::SendBuffer(BYTE* data, BYTE nBlock,bool bSleepAfter)
{
	BYTE write_buff[10];
	write_buff[0]=0x01;
	write_buff[1]=0x04;
	write_buff[2]=0x07;
	write_buff[3]=0x02;
	write_buff[4]=0x21;
	write_buff[5]= nBlock;
	write_buff[6]=data[0];
	write_buff[7]=data[1];
	write_buff[8]=data[2];
	write_buff[9]=data[3];
	for (int i = 0; i < exchange_settings.nSendRetryCount; i++)
	{
		if (Write(write_buff, 10))
		{
			if (GetAnswer(bSleepAfter))
			{
				if (arrRead[0] == 0x80 && arrRead[1] == 0x04)return 1;
			}
			else Sleep(exchange_settings.nRetryWaitMS);
		}
	}
	return 0;
}

bool CNFC_Device::GetBuffer(BYTE nBlock, bool bGetLog, UINT nTimeout)
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x04;
	write_buff[3] = 0x02;
	write_buff[4] = 0x23;
	write_buff[5] = nBlock << 3;
	write_buff[6] = 0x07;
	write_buff[7] = 0x00;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	bool bExtendedWait = nTimeout > 2000;
	CRealtimeObject obj;
	for (int i = 0; i < exchange_settings.nGetRetryCount; i++)
	{
		obj.ResetTime();
		while (1)
		{
			if (Write(write_buff, 10))
			{
				if (GetAnswer(1, i ? 2000 : nTimeout))
				{
					if (arrRead[0] == 0x80 && arrRead[1] == 0x24)
					{
						if (nBlock)return 1;
						if (!bGetLog && arrRead[21] == 1)return 1;
						else if (bGetLog && arrRead[3] == 0xa3 && arrRead[4] == 0xb3)return 1;
						else
						{
							if (bGetLog)return 0;
							if (bExtendedWait)
							{
								if (obj.GetTime() * 1000 > nTimeout)
								{
									bExtendedWait = 0;
									break;
								}
								Sleep(exchange_settings.nRetryWaitMS);
								continue;
							}
							Sleep(exchange_settings.nRetryWaitMS);
							break;
						}
					}
					else break;
				}
				else break;
			}
			else break;
		}
		Sleep(exchange_settings.nRetryWaitMS);
	}
	return 0;
}

bool CNFC_Device::Write(BYTE* data, UINT nLen)
{
	if (IsBreak())return 0;
	int nRet = dev->write(data, nLen);
	return nRet != -1;
}

bool CNFC_Device::SendCountByte(UINT nCount)
{
	BYTE write_buff[10];
	write_buff[0]=0x01;
	write_buff[1]=0x04;
	write_buff[2]=0x07;
	write_buff[3]=0x02;
	write_buff[4]=0x21;
	write_buff[5]=0x03;
	write_buff[6]=0x6E;
	write_buff[7] = LOBYTE(nCount);
	write_buff[8]=HIBYTE(nCount);
	write_buff[9]=0x00;
	if (!Write(write_buff, 10))return 0;
	return GetAnswer();
}

bool CNFC_Device::SendE140FF01()
{
	BYTE write_buff[10];
	write_buff[0]=0x01;
	write_buff[1]=0x04;
	write_buff[2]=0x07;
	write_buff[3]=0x02;
	write_buff[4]=0x21;
	write_buff[5]=0x00;
	write_buff[6]=0xE1;
	write_buff[7]=0x40;
	write_buff[8]=0xFF;
	write_buff[9]=0x01;
	if (!Write(write_buff, 10))return 0;
	return GetAnswer(0);
}


bool CNFC_Device::Blink()
{
	return SendCommand(BlinkByte);
}

bool CNFC_Device::Beep()
{
	return SendCommand(OneBeepByte);
}


bool CNFC_Device::SetSerialNumber(UINT nNum)
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(4);
	*(UINT*)&(*arr)[0] = nNum;
	return SendCommand(SetSerialNumberByte,arr);
}

bool CNFC_Device::SetLogTimeout(BYTE nTimeout)
{
	CByteArray* arr = new CByteArray;
	arr->Add(nTimeout);
	dev_settings.LogTimeOut = nTimeout;
	return SendCommand(SetLogTimeOutByte, arr);
}

bool CNFC_Device::SetWeekToScale(BYTE nWeeks)
{
	CByteArray* arr = new CByteArray;
	arr->Add(nWeeks);
	return SendCommand(SetScaleTime, arr);
}

bool CNFC_Device::Reboot()
{
	CByteArray* arr = new CByteArray;
	arr->Add(LOBYTE(RebootBytes));
	arr->Add(HIBYTE(RebootBytes));
	return SendCommand(RebootByte, arr);
}

bool CNFC_Device::ReplaceBattery()
{
	return SendCommand(ReplButtByte);
}

bool CNFC_Device::EnableBeep(bool bEnable)
{
	CByteArray* arr = new CByteArray;
	arr->Add(!bEnable);
	return SendCommand(SwitchBeepByte, arr);
}

bool CNFC_Device::EnableVibro(bool bEnable)
{
	CByteArray* arr = new CByteArray;
	arr->Add(!bEnable);
	return SendCommand(SwitchVibrByte, arr);
}

bool CNFC_Device::EnableAlarm(bool bEnable)
{
	CByteArray* arr = new CByteArray;
	arr->Add(!bEnable);
	return SendCommand(UnlockSignByte, arr);
}

bool CNFC_Device::EnableNFCPowerDown(bool bEnable)
{
	CByteArray* arr = new CByteArray;
	arr->Add(!bEnable);
	return SendCommand(NFCPowerDownByte, arr);
}

bool CNFC_Device::SetDeviceMode(UINT nMode)
{
	CByteArray* arr = new CByteArray;
	arr->Add(nMode);
	return SendCommand(ModeDevByte, arr);
}


bool CNFC_Device::SetDateTime(UINT nTime,bool bDaylightSave)
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(5);
	*(UINT*)&(*arr)[0] = nTime;
	(*arr)[4] = bDaylightSave;
	return SendCommand(SetTimeByte, arr);
}

bool CNFC_Device::GetDeviceInfo(bool bClearVer, bool bOnlyInfo, bool bForce)
{
	if(bClearVer)dev_info.nSWVer = 0;
	additional_pars.bInfoOnly = bOnlyInfo;
	if (!SendCommand(GetInfDeviceByte, 0, 0, bForce))return 0;
	return 1;
}

void DEVICE_INFO::FillSWVer()
{
	nSWVer = sw_min;
	if (sw_maj > 1)nSWVer += (sw_maj - 1) * 100;
}

bool CNFC_Device::ParseDeviceInfo(CByteArray* arr)
{
	if (arr->GetSize() < 64)return 0;
	bool bPrevLoaded = dev_info.bLoaded;
	UINT nPrevSerial = dev_info.nSerialNo;
	dev_info.bLoaded = 1;
	BYTE btCompatibility = 0;
	dev_info.strCPUId = "";
	CString str;
	for (int i = 0; i <= 11; i++)
	{
		str.Format("%02x", (UINT)(*arr)[i + 27]);
		dev_info.strCPUId += str;
	}
	dev_info.nSerialNo = *(UINT*)&(*arr)[39];
	if ((*arr)[43] == 64 && (*arr)[44] == 226)
	{
		btCompatibility = 0;
	}
	else
	{
		btCompatibility = 4;
		dev_info.btReplaceCount = (*arr)[43];
	}
	dev_info.sw_min = (*arr)[51-btCompatibility];
	dev_info.sw_maj = (*arr)[52 - btCompatibility];
	dev_info.hw_min = (*arr)[53 - btCompatibility];
	dev_info.hw_maj = (*arr)[54 - btCompatibility];
	dev_info.FillSWVer();
	CString strHW;
	if (dev_info.nSWVer >= 97)
	{
		strHW = " ";
		dev_info.btEnabledHW = (*arr)[46];
		if (dev_info.btEnabledHW & HW_LORA)strHW += "L";
		else strHW += "N";
		if (dev_info.btEnabledHW & HW_SUBSEC_TIM)strHW += "T";
		else strHW += "R";
		if (dev_info.btEnabledHW & HW_RTC_CLOCK_QUARZ)strHW += "Q";
		else if (dev_info.btEnabledHW & HW_RTC_CLOCK_LSI)strHW += "I";
		else strHW += "G";
	}
	else dev_info.btEnabledHW = 0;
	dev_info.strSoftwareVer.Format("%u.%02u%s", (UINT)dev_info.sw_maj, (UINT)dev_info.sw_min, strHW);
	dev_info.strHardwareVer.Format("%u.%02u", (UINT)dev_info.hw_maj, (UINT)dev_info.hw_min);

	if (dev_info.nSWVer < 27)dev_info.btVerControl = 1;
	else dev_info.btVerControl = 0;
	dev_info.nTime = *(UINT*)&(*arr)[55 - btCompatibility];
	if (dev_info.nTime == 0)
	{
		return 0;
	}
	for (int i = 59; i <= 61; i++)
	{
		if ((*arr)[59 - btCompatibility] != 0 && (*arr)[59 - btCompatibility] != 1)
		{
			return 0;
		}
	}
	dev_info.base.bBeep= !(*arr)[59 - btCompatibility];
	dev_info.base.bVibro = !(*arr)[60 - btCompatibility];
	dev_info.base.bAlarm = !(*arr)[61 - btCompatibility];
	dev_info.base.bDaylight = (*arr)[67 - btCompatibility];
	dev_info.base.nLogTimeout = (*arr)[68 - btCompatibility];
	if (bPrevLoaded && nPrevSerial != dev_info.nSerialNo)ClearLoadState();
	if (!additional_pars.bInfoOnly && IsHaveProperty(INTERFACES))return GetInterfaces();
	additional_pars.bInfoOnly = 0;
	return 1;
}

bool CNFC_Device::GetAllCoeffs()
{
	additional_pars.bGetOnlySensorCoeff = 0;
	return SendCommand(GetAllCoefByte);
}

bool CNFC_Device::GetCoeffsOnly()
{
	additional_pars.bGetOnlySensorCoeff = 1;
	return SendCommand(GetAllCoefByte);
}

bool CNFC_Device::ParseSensorsInfo(CByteArray* arr)
{
	sensors_info.bLoaded = 1;
	float* f = (float*)&(*arr)[27];
	for (int i = 0; i < 19; i++)
		sensors_info.fO2Coeff[i] = f[i];
	for (int i = 0; i < 14; i++)
		sensors_info.fCOCoeff[i] = f[i+19];
	for (int i = 0; i < 14; i++)
		sensors_info.fH2SCoeff[i] = f[i+33];
	sensors_info.fAccO2=*(float*)&(*arr)[215];
	sensors_info.fAccCO = *(float*)&(*arr)[219];
	sensors_info.fAccH2S = *(float*)&(*arr)[223];
	sensors_info.fAccMIPEX = *(float*)&(*arr)[227];
	if (IsHaveProperty(CH4_COEFF))
	{
		sensors_info.fCH4Threshold = *(UINT*)&(*arr)[231];
		sensors_info.fCH4K1 = *(float*)&(*arr)[235];
		sensors_info.fCH4K2 = *(float*)&(*arr)[239];
		sensors_info.fCH4K3 = *(float*)&(*arr)[243];
		sensors_info.fCH4K4 = *(float*)&(*arr)[247];
		sensors_info.fCH4K5 = *(float*)&(*arr)[251];
		sensors_info.fCH4K6 = *(float*)&(*arr)[255];
	}
	if (IsHaveProperty(CH4_MULT_NEW))
	{
		SendCommand(GetCH4Poly);
	}
	else if (IsHaveProperty(CH4_MULT))
	{
		memcpy(sensors_info.fCoefCH4_Mult, &(*arr)[259], 6 * sizeof(float));
		memcpy(sensors_info.fCoefCH4_Mult2, &(*arr)[283], 6 * sizeof(float));
		memcpy(&sensors_info.nSwitchConc, &(*arr)[307], 4);
	}
	if (!additional_pars.bGetOnlySensorCoeff)
	{
		GetSensorStatus();
		GetSensorVoltRange();
		GetSettings();
		GetGasRange();
		if (IsHaveProperty(MIPEXCOEFF))MipexGetBackup();
	}
	return 1;
}

bool CNFC_Device::GetVibroPower()
{
	if (dev_info.btVerControl != 0)return 1;
	return SendCommand(GetRegVibroByte);
}

bool CNFC_Device::GetGasRange()
{
	return SendCommand(GetGasRangeByte);
}

bool CNFC_Device::GetSensorVoltRange()
{
	return SendCommand(GetSensVRangeByte);
}

bool CNFC_Device::ParseGasRange(CByteArray* arr)
{
	sensors_info.bGasRangeLoaded = 1;
	WORD* w = (WORD*)&(*arr)[27];
	sensors_info.o2Range.wFrom = w[0];
	sensors_info.o2Range.wTo = w[1];
	sensors_info.o2Range.bInited = 1;
	sensors_info.coRange.wFrom = w[2];
	sensors_info.coRange.wTo = w[3];
	sensors_info.coRange.bInited = 1;
	sensors_info.h2sRange.wFrom = w[4];
	sensors_info.h2sRange.wTo = w[5];
	sensors_info.h2sRange.bInited = 1;
	sensors_info.ch4Range.wFrom = w[6];
	sensors_info.ch4Range.wTo = w[7];
	sensors_info.ch4Range.bInited = 1;
	return 1;
}

bool CNFC_Device::ParseSensorVRange(CByteArray* arr)
{
	WORD* w = (WORD*)&(*arr)[27];
	sensors_info.o2VRange.wFrom = w[0];
	sensors_info.o2VRange.wTo = w[1];
	sensors_info.o2VRange.bInited = 1;
	sensors_info.coVRange.wFrom = w[2];
	sensors_info.coVRange.wTo = w[3];
	sensors_info.coVRange.bInited = 1;
	sensors_info.h2sVRange.wFrom = w[4];
	sensors_info.h2sVRange.wTo = w[5];
	sensors_info.h2sVRange.bInited = 1;
	return 1;
}

bool CNFC_Device::SetO2Coeff()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(76);
	memcpy(&(*arr)[0], &sensors_info.fO2Coeff[0], 76);
	return SendCommand(SendCoefO2Byte,arr);
}

bool CNFC_Device::SetCOCoeff()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(56);
	memcpy(&(*arr)[0], &sensors_info.fCOCoeff[0], 56);
	return SendCommand(SendCoefCOByte, arr);
}

bool CNFC_Device::SetH2SCoeff()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(56);
	memcpy(&(*arr)[0], &sensors_info.fH2SCoeff[0], 56);
	return SendCommand(SendCoefH2SByte, arr);
}

bool CNFC_Device::SetCH4Coeff()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(28);
	memcpy(&(*arr)[0], &sensors_info.fCH4Threshold, 28);
	return SendCommand(SendCH4CoefByte, arr);
}

bool CNFC_Device::SetCH4CoeffMult()
{
	if (!IsHaveProperty(CH4_MULT))return 0;
	CByteArray* arr = new CByteArray;
	arr->SetSize(52);
	memcpy(&(*arr)[0], sensors_info.fCoefCH4_Mult, 24);
	memcpy(&(*arr)[24], sensors_info.fCoefCH4_Mult2, 24);
	memcpy(&(*arr)[48], &sensors_info.nSwitchConc, 4);
	return SendCommand(SetCH4Mult, arr);
}

bool CNFC_Device::SetSensorAccel()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(16);
	memcpy(&(*arr)[0], &sensors_info.fAccO2, 16);
	return SendCommand(SendSensAccelByte, arr);
}

bool CNFC_Device::SetGasRange()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(16);
	WORD* w = (WORD*)&(*arr)[0];
	w[0]=sensors_info.o2Range.wFrom;
	w[1]=sensors_info.o2Range.wTo;
	w[2]=sensors_info.coRange.wFrom;
	w[3]=sensors_info.coRange.wTo;
	w[4]=sensors_info.h2sRange.wFrom;
	w[5]=sensors_info.h2sRange.wTo;
	w[6]=sensors_info.ch4Range.wFrom;
	w[7]=sensors_info.ch4Range.wTo;
	return SendCommand(SetGasRangeByte, arr);
}

bool CNFC_Device::SetSensorVRange()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(12);
	WORD* w = (WORD*)&(*arr)[0];
	w[0]=sensors_info.o2VRange.wFrom;
	w[1]=sensors_info.o2VRange.wTo;
	w[2]=sensors_info.coVRange.wFrom;
	w[3]=sensors_info.coVRange.wTo;
	w[4]=sensors_info.h2sVRange.wFrom;
	w[5]=sensors_info.h2sVRange.wTo;
	return SendCommand(SetSensVRangeByte, arr);
}


bool CNFC_Device::GetSensorStatus()
{
	return SendCommand(GetSensStatusByte);
}

bool CNFC_Device::ParseSensorStatus(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	sensors_info.bO2En = b[0];
	sensors_info.bCOEn = b[1];
	sensors_info.bH2SEn = b[2];
	sensors_info.bCH4En = b[3];
	sensors_info.bStatusLoaded = 1;
	return true;
}

bool CNFC_Device::SetSensorStatus()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(4);
	BYTE* b = &(*arr)[0];
	b[0]=sensors_info.bO2En;
	b[1]=sensors_info.bCOEn;
	b[2]=sensors_info.bH2SEn;
	b[3]=sensors_info.bCH4En;
	bool bSet = 0;
	if (!dev_settings.bLoaded)
	{
		GetSettings();
		if (!WaitComplete(1))return 0;
	}
	if (sensors_info.bO2En > 1)
	{
		if (!dev_settings.base.O2Chem)
		{
			dev_settings.base.O2Chem = 1;
			bSet = 1;
		}
	}
	else
	{
		if (dev_settings.base.O2Chem)
		{
			dev_settings.base.O2Chem = 0;
			bSet = 1;
		}
	}
	if (bSet)SetSettings();
	if (!SendCommand(SetSensStatusByte, arr))return 0;
	if (bSet)GetSettings();
	return 1;
}

bool CNFC_Device::ParseVibroPower(CByteArray* arr)
{
	dev_info.bRegVibroLoaded = 1;
	dev_info.base.btVibroPower = (*arr)[27];
	return 1;
}

bool CNFC_Device::SetVibroPower()
{
	CByteArray* arr = new CByteArray;
	arr->Add(dev_info.base.btVibroPower);
	return SendCommand(SetRegVibroByte, arr);
}

bool CNFC_Device::GetAlarms()
{
	return SendCommand(GetAlarmSignalingByte);
}

bool CNFC_Device::ParseAlarms(CByteArray* arr)
{
	WORD* w = (WORD*)&(*arr)[27];
	ALARM_INFO* a = &sensors_info.alarms;
	a->bLoaded = 1;
	a->ch4Alarm.wFrom = w[0];
	a->ch4Alarm.wTo = w[4];
	a->ch4Alarm.bInited = 1;
	a->o2Alarm.wFrom = w[1];
	a->o2Alarm.wTo = w[5];
	a->o2Alarm.bInited= 1;
	a->coAlarm.wFrom = w[2];
	a->coAlarm.wTo= w[6];
	a->coAlarm.bInited = 1;
	a->h2sAlarm.wFrom = w[3];
	a->h2sAlarm.wTo = w[7];
	a->h2sAlarm.bInited = 1;
	return 1;
}


bool CNFC_Device::SetAlarms()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(16);
	ALARM_INFO* a = &sensors_info.alarms;
	WORD* w = (WORD*)&(*arr)[0];
	w[0] = a->ch4Alarm.wFrom;
	w[4] = a->ch4Alarm.wTo;
	w[1] = a->o2Alarm.wFrom;
	w[5] = a->o2Alarm.wTo;
	w[2] = a->coAlarm.wFrom;
	w[6] = a->coAlarm.wTo;
	w[3] = a->h2sAlarm.wFrom;
	w[7] = a->h2sAlarm.wTo;
	return SendCommand(SetAlarmSignalingByte, arr);
}

bool CNFC_Device::GetSettings()
{
	return SendCommand(GetSettingsByte);
}

bool CNFC_Device::ParseSettings(CByteArray* arr)
{
	dev_settings = DEVICE_SETTINGS();
	dev_settings.Led_PWM = *(WORD*)&(*arr)[27];
	dev_settings.Vibro_PWM = *(WORD*)&(*arr)[29];
	dev_settings.Led_Time = *(WORD*)&(*arr)[31];
	dev_settings.Led_Slow_Time = *(WORD*)&(*arr)[33];
	dev_settings.BeepOff_TimeOut = *(WORD*)&(*arr)[35];
	dev_settings.Wait_TimeOut = *(WORD*)&(*arr)[37];
	dev_settings.LedRedSCR_PWM = *(WORD*)&(*arr)[39];
	dev_settings.FreezeDeltaTemper = *(WORD*)&(*arr)[41];
	dev_settings.FreezeDeltaTime = *(WORD*)&(*arr)[43];
	dev_settings.Vref_WarmUp_Time = *(WORD*)&(*arr)[45];
	dev_settings.CH4_Buffer_Term = (*arr)[47];
	dev_settings.CH4_Buffer_Time = (*arr)[48];
	if (dev_info.btVerControl == 1)
	{
		dev_settings.Flash_WriteTime = *(WORD*)&(*arr)[49];
		dev_settings.Task_Latency = *(WORD*)&(*arr)[51];
		dev_settings.Stop_Time = *(WORD*)&(*arr)[53];
		dev_settings.Mipex_State=(*arr)[55];
		dev_settings.Log_State=(*arr)[56];
		dev_settings.Pressure_State=(*arr)[57];
		dev_settings.Life_Time_W = (*arr)[58];
		dev_settings.FreezeStаtusMask = *(WORD*)&(*arr)[59];
		dev_settings.base.CoefVolToLEL = *(WORD*)&(*arr)[63];
		dev_settings.LogTimeOut = (*arr)[65];
		dev_settings.LogAlarmTimeOut = (*arr)[66];
	}
	else
	{
		dev_settings.base.CoefH2SppmToMg = *(WORD*)&(*arr)[49];
		dev_settings.base.CoefCOppmToMg = *(WORD*)&(*arr)[51];
		dev_settings.NFCTimeOutDetectSeconds = (*arr)[53];
		dev_settings.NFCTimeOutWaitMinutes = (*arr)[54];
		dev_settings.BattLow = (*arr)[55];
		dev_settings.Log_State = (*arr)[56];
		dev_settings.Pressure_State = (*arr)[57];
		dev_settings.Life_Time_W = (*arr)[58];
		if (IsHaveProperty(FREEZE_MASK))
		{
			dev_settings.FreezeStаtusMask = *(WORD*)&(*arr)[59];
			dev_settings.FreezeLimit = *(WORD*)&(*arr)[61];
		}
		dev_settings.base.CoefVolToLEL = *(WORD*)&(*arr)[63];
		dev_settings.LogTimeOut = (*arr)[65];
		dev_settings.LogAlarmTimeOut = (*arr)[66];
		dev_settings.SensorsUnits = (*arr)[67];
		dev_settings.SNSRef_ADC = *(WORD*)&(*arr)[68];
		dev_settings.base.O2Chem = (*arr)[70];
		dev_settings.base.CoefCHEMppmToMg = *(WORD*)&(*arr)[71];
		if (IsHaveProperty(PRECISION))dev_settings.SensorsPrecisions = (*arr)[73];
		if (IsHaveProperty(SKIPSELFTEST))dev_settings.SkipSelfTest = (*arr)[74];
		if (IsHaveProperty(AUTOZERO))dev_settings.SensorsAutoZero = (*arr)[75];
		if (IsHaveProperty(ALTSCREENTIME))dev_settings.AltScreenTime = (*arr)[76];
		if (IsHaveProperty(LORALOW))
		{
			dev_settings.RssiLow=*(WORD*)&(*arr)[77];
			dev_settings.SnrLow = (*arr)[79];
			dev_settings.AlarmType = (*arr)[80];
			dev_settings.LostSec = *(WORD*)&(*arr)[81];
			dev_settings.LostPackets = (*arr)[83];
		}
		if (IsHaveProperty(GASSIM))
		{
			dev_settings.O2Sim = *(WORD*)&(*arr)[84];
			dev_settings.COSim = *(WORD*)&(*arr)[86];
			dev_settings.H2SSim = *(WORD*)&(*arr)[88];
			dev_settings.CH4Sim = *(WORD*)&(*arr)[90];
		}
		if(IsHaveProperty(O2CHEMPOS))
		{
			dev_settings.O2ChemScrPos = (*arr)[92];
		}
		if (IsHaveProperty(SCALEPOINT))
		{
			dev_settings.ScalePoint.O2 = *(WORD*)&(*arr)[93];
			dev_settings.ScalePoint.CO = *(WORD*)&(*arr)[95];
			dev_settings.ScalePoint.H2S = *(WORD*)&(*arr)[97];
			dev_settings.ScalePoint.CH4 = *(WORD*)&(*arr)[99];
		}
		if (IsHaveProperty(CHPRESSURE)) dev_settings.Options=(*arr)[101];
		if (IsHaveProperty(WEEKTOSCALE)) dev_settings.WeekToScale = (*arr)[102];
		if (IsHaveProperty(TRANSPORTALARM))
		{
			dev_settings.TransportAlarmOffMin = (*arr)[103];
			dev_settings.Unfreeze = (*arr)[104];
		}
	}
	dev_settings.bLoaded = 1;
	return 1;
}

bool CNFC_Device::SetSettings(bool bGet)
{
	CByteArray* arr = new CByteArray;
	int nc = 57;
	if (IsHaveProperty(GASSIM))nc += 21;
	arr->SetSize(nc);
	memset(&(*arr)[0], 0, arr->GetSize());
	WORD* w = (WORD*)&(*arr)[0];
	w[0] = dev_settings.Led_PWM;
	w[1] = dev_settings.Vibro_PWM;
	w[2] = dev_settings.Led_Time;
	w[3] = dev_settings.Led_Slow_Time;
	w[4]= dev_settings.BeepOff_TimeOut;
	w[5]= dev_settings.Wait_TimeOut;
	w[6]= dev_settings.LedRedSCR_PWM;
	w[7]= dev_settings.FreezeDeltaTemper;
	w[8]= dev_settings.FreezeDeltaTime;
	w[9]= dev_settings.Vref_WarmUp_Time;
	(*arr)[20]= dev_settings.CH4_Buffer_Term;
	(*arr)[21] = dev_settings.CH4_Buffer_Time;
	if (dev_info.btVerControl == 1)
	{
		w[11] = dev_settings.Flash_WriteTime;
		w[12] = dev_settings.Task_Latency;
		w[13] = dev_settings.Stop_Time;
		(*arr)[28] = dev_settings.Mipex_State;
		(*arr)[29] = dev_settings.Log_State;
		(*arr)[30] = dev_settings.Pressure_State;
		(*arr)[31] = dev_settings.Life_Time_W;
		*(WORD*)&(*arr)[32] = dev_settings.FreezeStаtusMask;
		*(WORD*)&(*arr)[36] = dev_settings.base.CoefVolToLEL;
		(*arr)[38] = dev_settings.LogTimeOut;
		(*arr)[39] = dev_settings.LogAlarmTimeOut;
	}
	else
	{
		w[11] = dev_settings.base.CoefH2SppmToMg;
		w[12] = dev_settings.base.CoefCOppmToMg;
		(*arr)[26] = dev_settings.NFCTimeOutDetectSeconds;
		(*arr)[27] = dev_settings.NFCTimeOutWaitMinutes;
		(*arr)[28] = dev_settings.BattLow;
		(*arr)[29] = dev_settings.Log_State;
		(*arr)[30] = dev_settings.Pressure_State;
		(*arr)[31] = dev_settings.Life_Time_W;
		if (IsHaveProperty(FREEZE_MASK))
		{
			*(WORD*)&(*arr)[32] = dev_settings.FreezeStаtusMask;
			*(WORD*)&(*arr)[34] = dev_settings.FreezeLimit;
		}
		*(WORD*)&(*arr)[36] = dev_settings.base.CoefVolToLEL;
		(*arr)[38] = dev_settings.LogTimeOut;
		(*arr)[39] = dev_settings.LogAlarmTimeOut;
		(*arr)[40] = dev_settings.SensorsUnits;
		*(WORD*)&(*arr)[41] = dev_settings.SNSRef_ADC;
		(*arr)[43] = dev_settings.base.O2Chem;
		*(WORD*)&(*arr)[44] = dev_settings.base.CoefCHEMppmToMg;
		if (IsHaveProperty(PRECISION))(*arr)[46] = dev_settings.SensorsPrecisions;
		if (IsHaveProperty(SKIPSELFTEST))(*arr)[47] = dev_settings.SkipSelfTest;
		if (IsHaveProperty(AUTOZERO))(*arr)[48] = dev_settings.SensorsAutoZero;
		if (IsHaveProperty(ALTSCREENTIME))(*arr)[49]= dev_settings.AltScreenTime;
		if (IsHaveProperty(LORALOW))
		{
			*(WORD*)&(*arr)[50] = dev_settings.RssiLow;
			(*arr)[52] = dev_settings.SnrLow;
			(*arr)[53] = dev_settings.AlarmType;
			*(WORD*)&(*arr)[54] = dev_settings.LostSec;
			(*arr)[56] = dev_settings.LostPackets;
		}
		if (IsHaveProperty(GASSIM))
		{
			*(WORD*)&(*arr)[57]= dev_settings.O2Sim;
			*(WORD*)&(*arr)[59] = dev_settings.COSim;
			*(WORD*)&(*arr)[61] = dev_settings.H2SSim;
			*(WORD*)&(*arr)[63] = dev_settings.CH4Sim;
		}
		if (IsHaveProperty(O2CHEMPOS))(*arr)[65] = dev_settings.O2ChemScrPos;
		if (IsHaveProperty(SCALEPOINT))
		{
			*(WORD*)&(*arr)[66] = dev_settings.ScalePoint.O2;
			*(WORD*)&(*arr)[68] = dev_settings.ScalePoint.CO;
			*(WORD*)&(*arr)[70] = dev_settings.ScalePoint.H2S;
			*(WORD*)&(*arr)[72] = dev_settings.ScalePoint.CH4;
		}
		if (IsHaveProperty(CHPRESSURE))(*arr)[74] = dev_settings.Options;
		if (IsHaveProperty(WEEKTOSCALE))(*arr)[75] = dev_settings.WeekToScale;
		if (IsHaveProperty(TRANSPORTALARM))
		{
			(*arr)[76] = dev_settings.TransportAlarmOffMin;
			(*arr)[77] = dev_settings.Unfreeze;
		}
	}
//	(*arr)[56] = 0xdb;
	SendCommand(SetSettingsByte,arr);
	return bGet ? GetSettings() : 1;
}


bool CNFC_Device::TestAlarm(UINT nAlarm)
{
	return SendCommand(nAlarm? TestAlarm2Byte: TestAlarm1Byte);
}

bool CNFC_Device::GetLoraSettings()
{
	return SendCommand(GetRadioSettingsByte);
}

bool CNFC_Device::ParseLoraSettings(CByteArray* arr)
{
	LORA_SETTINGS* s = &lora_settings;
	s->bLoaded = 1;
	s->f1.nFreq = *(UINT*)&(*arr)[27];
	s->f1.bEn = s->f1.nFreq != 0;
	s->f1.btPower = (*arr)[31];
	s->btBW = (*arr)[32];
	s->f1.nSF= (*arr)[33];
	s->btCR = (*arr)[34];
	if (s->btCR != 0)s->btCR -= 1;
	s->nAddress= *(UINT*)&(*arr)[35];
	s->nDataPeriod= *(WORD*)&(*arr)[39];
	s->f2.nSF = (*arr)[41] >> 4;
	s->f3.nSF = (*arr)[41] & 0xf;
	s->f2.btPower = (*arr)[42] >> 4;
	s->f3.btPower = (*arr)[42] & 0xf;
	s->f2.nFreq = *(UINT*)&(*arr)[43];
	s->f2.bEn = s->f2.nFreq != 0;
	s->f3.nFreq = *(UINT*)&(*arr)[47];
	s->f3.bEn = s->f3.nFreq != 0;
	if (IsHaveProperty(LORAOTAA))
	{
		memcpy(&s->rx,&(*arr)[51],s->rx.GetSize());
		/*s->rx.RX1Delay = *(WORD*)&(*arr)[51];
		s->rx.RX2Delay = *(WORD*)&(*arr)[53];
		s->rx.RX2Frequency = *(DWORD*)&(*arr)[55];
		s->rx.RX2SpreadingFactor = (*arr)[59];
		s->rx.Window = (*arr)[60];*/
	}
	GetLoraKey();
	return 1;
}

bool CNFC_Device::SetLoraSettings()
{
	CByteArray* arr = new CByteArray;
	int nSz = 24;
	LORA_SETTINGS* s = &lora_settings;
	if (IsHaveProperty(LORAOTAA))nSz += s->rx.GetSize();
	arr->SetSize(nSz);

	if (s->f1.bEn)*(UINT*)&(*arr)[0] = s->f1.nFreq;
	else *(UINT*)&(*arr)[0] = 0;
	(*arr)[4] = s->f1.btPower;
	(*arr)[5] = s->btBW;
	(*arr)[6] = s->f1.nSF;
	(*arr)[7] = s->btCR + 1;
	*(UINT*)&(*arr)[8] = s->nAddress;
	*(WORD*)&(*arr)[12] = s->nDataPeriod;
	(*arr)[14] = (s->f2.nSF << 4) | (s->f3.nSF & 0xf);
	(*arr)[15] = (s->f2.btPower << 4) | (s->f3.btPower & 0xf);
	if (s->f2.bEn)*(UINT*)&(*arr)[16] = s->f2.nFreq;
	else *(UINT*)&(*arr)[16] = 0;
	if (s->f3.bEn)*(UINT*)&(*arr)[20] = s->f3.nFreq;
	else *(UINT*)&(*arr)[20] = 0;
	if (IsHaveProperty(LORAOTAA))
	{
		memcpy(&(*arr)[24],&s->rx,s->rx.GetSize());
	}
	return SendCommand(SetRadioByte,arr);
}

bool CNFC_Device::GetLoraKey()
{
	if (dev_info.nSWVer < 20)return 0;
	return SendCommand(GetLoraKeyByte);
}

bool CNFC_Device::ParseLoraKey(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	CString str;
	LORA_SETTINGS* s = &lora_settings;
	strcpy_s(s->strAppKey, "");
	strcpy_s(s->strNetworkKey, "");
	for (int i = 0; i < 16; i++)
	{
		str.Format("%02x", (UINT)b[i]);
		strcat_s(s->strNetworkKey, str);
	}
	for (int i = 0; i < 16; i++)
	{
		str.Format("%02x", (UINT)b[i+16]);
		strcat_s(s->strAppKey,str);
	}
	return 1;
}

bool CNFC_Device::SetLoraKey()
{
	LORA_SETTINGS* s = &lora_settings;
	CByteArray* arr = new CByteArray;
	arr->SetSize(32);
	int n = 0;
	CString strNetworkKey = s->strNetworkKey, strAppKey = s->strAppKey;
	for (int i = 0; i < 32; i+=2)
	{
		(*arr)[n++] = (BYTE)strtol(strNetworkKey.Mid(i, 2), 0, 16);
	}
	for (int i = 0; i < 32; i += 2)
	{
		(*arr)[n++] = (BYTE)strtol(strAppKey.Mid(i, 2), 0, 16);
	}
	return SendCommand(SendLoraKeyByte, arr);
}

bool CNFC_Device::SendLoraPacket()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(11);
	(*arr)[0] = 1;
	return SendCommand(SendRadioPacketByte, arr);
}

bool CNFC_Device::GetWorkStat()
{
	return SendCommand(GetWorkStatByte);
}

bool CNFC_Device::ParseWorkStat(CByteArray* arr)
{
	UINT* u = (UINT*)&(*arr)[27];
	UINT* d = &work_stat.AlarmTime;
	memcpy(d, u, 64);
	memcpy(&work_stat.VoltBattPerWeek[0], &(*arr)[91], 104);
	work_stat.bLoaded = 1;
	return true;
}

bool CNFC_Device::ResetWorkStat()
{
	return SendCommand(NullStatByte);
}

bool CNFC_Device::SendCommandMIPEX()
{
	if (IsVirtualDevice())
	{
		CNFC_Command* c = new CNFC_Command;
		c->dev = this;
		c->nCommand = SetCommandMIPEXByte;
		c->get = new CByteArray;
		CString ans = "VirtualDevice";
		WORD len = ans.GetLength();
		c->get->SetSize(27 + 2 + len);
		BYTE* b = &(*c->get)[27];
		memcpy(b, &len, 2);
		memcpy(&b[2], ans.GetBuffer(), len);
		c->nRet = 1;
		wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
		return 1;
	}
	CString strCommand = additional_pars.strMIPEXCommand;
	S_O::Trim(strCommand);
	int nL = strCommand.GetLength();
	if (!nL || strCommand == "@")return 0;
	CByteArray* arr = new CByteArray;
	arr->SetSize(nL+ additional_pars.strMipexEnd.GetLength()+1);
	(*arr)[0] = nL + additional_pars.strMipexEnd.GetLength();
	memcpy(&(*arr)[1], strCommand.GetBuffer(), nL);
	for(int i=0;i< additional_pars.strMipexEnd.GetLength();i++)
	(*arr)[nL + 1 + i] = additional_pars.strMipexEnd.GetAt(i);
	UINT nCommand = SetCommandMIPEXByte;
	if (additional_pars.nMipexDest == DEST_UART)nCommand = SetCommandUARTByte;
	else if (additional_pars.nMipexDest == DEST_SPI)nCommand = SetCommandSPIByte;
	additional_pars.bInMIPEXSend = 1;
	return SendCommand(nCommand, arr);
}

CString CNFC_Device::GetMIPEXAnswer(CByteArray* arr)
{
	CString str, buf;
	if (!arr->GetSize())return str;
	BYTE* b = &(*arr)[27];
	WORD nL = b[0], nDataPtr = 1;
	if (IsHaveProperty(MIPEX_16BIT))
	{
		nL |= ((WORD)b[1] << 8);
		nDataPtr++;
	}
	if (arr->GetSize() - nDataPtr < nL)return str;
	if (additional_pars.strMIPEXCommand == "DATAE2")
	{
		for (int i = 0; i < nL; i++)
		{
			buf.Format("%02x", (UINT)b[i + nDataPtr]);
			str += buf;
		}
	}
	else
	{
		if (!IsHaveProperty(MIPEX_16BIT) && S_O::FindNoCase(additional_pars.strMIPEXCommand,"tabz")!=-1)
		{
			for (int i = nL + nDataPtr + 27; i < arr->GetSize(); i++)
			{
				if ((*arr)[i] == 0)break;
				nL++;
			}
		}
		char* buf = str.GetBufferSetLength(nL);
		memcpy(buf, &b[nDataPtr], nL);
		str.ReleaseBuffer(nL);
		if (additional_pars.strMIPEXCommand == "F" && additional_pars.bCheckMIPEXFAnswer)
		{
			if (!IsMIPEX_FAnswerCorrect(str))return "";
			str.GetBufferSetLength(69);
			str.ReleaseBuffer(69);
			str.Replace("\r", "");
		}
		str.Replace("\n", "");
		str.Replace("\t", " ");
		str.Replace("\x0e", "");
	}
	return str;
}

bool CNFC_Device::GetSensorData()
{
	if (IsVirtualDevice())
	{
		CNFC_Command* c = new CNFC_Command;
		c->dev = this;
		c->nCommand = GetDataSensorsByte;
		c->nRet = 1;
		sensors_data = SENSORS_DATA();
		sensors_data.time.SetToCurrent();
		wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
		return 1;
	}
	return SendCommand(GetDataSensorsByte);
}

bool CNFC_Device::ParseSensorsData(CByteArray* arr)
{
	if (!arr->GetSize())return 0;
	SENSORS_DATA* d = &sensors_data;
	d->time = CDateTime::GetCurrentTime();
	d->O2 = *(WORD*)&(*arr)[27];
	d->CO = *(WORD*)&(*arr)[29];
	d->H2S = *(WORD*)&(*arr)[31];
	d->CH4VOL = *(WORD*)&(*arr)[33];
	d->CH4LEL = *(DWORD*)&(*arr)[35];
	d->Press = *(DWORD*)&(*arr)[39];
	d->O2Volt = *(WORD*)&(*arr)[43];
	d->COVolt = *(WORD*)&(*arr)[45];
	d->H2SVolt = *(WORD*)&(*arr)[47];
	d->TempVolt = *(WORD*)&(*arr)[49];
	d->Batt = *(WORD*)&(*arr)[51];
	d->SensorTemp = *(short int*)&(*arr)[53];
	d->PressTemp = *(short int*)&(*arr)[55];
	d->CPUTemp = *(short int*)&(*arr)[57];
	return 1;
}

bool CNFC_Device::StartDeviceDetection()
{
	WaitForSingleObject(hMutex, INFINITE);
	if (IsInService() || IsInCommand())
	{
		evBreak.SetEvent();
		WaitForSingleObject(hCommandThread, INFINITE);
	}
	evService.SetEvent();
	hCommandThread = CreateThread(NULL, 0, DeviceDetectionThread, (void*)this, 0, 0);
	ReleaseMutex(hMutex);
	return 1;
}

int CNFC_Device::ReaderSequence1()
{
	BYTE write_buff[64];
	memset(write_buff, 0xcc, 64);
	write_buff[0] = 0x02;
	write_buff[1] = 0xbc;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	write_buff[0] = 0x02;
	write_buff[1] = 0xb2;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	write_buff[0] = 0x02;
	write_buff[1] = 0xbd;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	write_buff[0] = 0x01;
	write_buff[1] = 0x55;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	if (arrRead[0] != 0x55)return 2;
	return 1;
}

int CNFC_Device::ReaderSequence2()
{
	BYTE write_buff[64];
	memset(write_buff, 0xcc, 64);
	write_buff[0] = 0x01;
	write_buff[1] = 0x02;
	write_buff[2] = 0x02;
	write_buff[3] = 0x01;
	write_buff[4] = 0x0D;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	if (arrRead[0] != 0)return 2;
	return 1;
}

int CNFC_Device::ReaderSequence3()
{
	BYTE write_buff[64];
	memset(write_buff, 0xcc, 64);
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x02;
	write_buff[3] = 0x02;
	write_buff[4] = 0x2B;
	if (!Write(write_buff, 64))return 0;
	if (!GetAnswer())return 0;
	if (arrRead[11] == 0xe0 && arrRead[10] == 0x02 && arrRead[0] == 0x80)
	{
		dev_info.strSensorId = "";
		CString str;
		for (int i = 0; i <= 7; i++)
		{
			str.Format("%02x", (UINT)arrRead[i + 4]);
			dev_info.strSensorId = str + dev_info.strSensorId;
		}
		return 1;
	}
	return 2;
}


DWORD WINAPI CNFC_Device::DeviceDetectionThread(void* ptr)
{
	InitThread();
	CNFC_Device* dev = (CNFC_Device*)ptr;
	int n=0;
	while (!dev->IsBreak())
	{
		try
		{
			if (!dev->dev->isOpened())
			{
				if (!dev->dev->open(0))
				{
					dev->nDeviceStatus = NFC_STATUS_NONE;
					throw 0;
				}
			}
			dev->nDeviceStatus = NFC_STATUS_NONE;
			if (IsWindow(dev->wndNotify->GetSafeHwnd()))dev->wndNotify->PostMessage(WM_NFC_DEVICE_DETECTION_STATUS, (WPARAM)dev);
			while (!dev->IsBreak())
			{
				n=dev->ReaderSequence1();
				if (!n)throw 0;
				if (n==2)
				{
					dev->Sleep(1000);
					continue;
				}
				n = dev->ReaderSequence2();
				if(!n)throw 0;
				if (n == 2)
				{
					dev->Sleep(1000);
					continue;
				}
				dev->nDeviceStatus |= NFC_STATUS_HASREADER;
				if (IsWindow(dev->wndNotify->GetSafeHwnd()))dev->wndNotify->PostMessage(WM_NFC_DEVICE_DETECTION_STATUS, (WPARAM)dev);
				break;
			}
			for(int i=0;i<5;i++)
			{
				n=dev->ReaderSequence3();
				if (!n)throw 0;
				if (n==1)
				{
					dev->nDeviceStatus |= NFC_STATUS_DEVICEINSTALLED;
					if (IsWindow(dev->wndNotify->GetSafeHwnd()))dev->wndNotify->PostMessage(WM_NFC_DEVICE_DETECTION_STATUS, (WPARAM)dev);
					break;
				}
				dev->Sleep(1000);
			}
			if (dev->nDeviceStatus & NFC_STATUS_DEVICEINSTALLED)break;
		}
		catch (int)
		{
			dev->Sleep(1000);
		}
	}
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return 1;
}

CMap<UINT, UINT, CNFC_Device *, CNFC_Device *> mapThread;

bool CNFC_Device::WaitComplete(bool bAll,bool bIgnoreInputs)
{
	static CCriticalSection cs;
	bool bMessageQueue = 1;
	UINT nThreadId = GetCurrentThreadId();
	CNFC_Device* sel = 0;
	HANDLE h[] = { evBreak.m_hObject };
	bool bBreak = 0, bRet = 0;
	while (1)
	{
		if (!IsInCommand())
		{
			bool b = 1;
			if (bAll && IsInService())b = 0;
			if (b)
			{
				evBreak.ResetEvent();
				bRet=!bBreak;
				break;
			}
		}
		cs.Lock();
		if (mapThread.Lookup(nThreadId, sel))
		{
			bMessageQueue = sel==this;
		}
		else
		{
			mapThread.SetAt(nThreadId, this);
			bMessageQueue = 1;
		}
		cs.Unlock();
		if(bMessageQueue)MsgWaitForMultipleObjects(1, h, 0, 100, QS_ALLEVENTS);
		if (IsBreak())bBreak = 1;
		if (bMessageQueue)CMainAppTemplate::MessageQueue(100, bIgnoreInputs);
		else ::Sleep(100);
	}
	if (bMessageQueue)
	{
		cs.Lock();
		if (mapThread.Lookup(nThreadId, sel) && sel==this)mapThread.RemoveKey(nThreadId);
		cs.Unlock();
	}
	return bRet && bLastCommandStatus;
}

bool CNFC_Device::BreakExecution()
{
	WaitForSingleObject(hMutex, INFINITE);
	evBreak.SetEvent();
	ReleaseMutex(hMutex);
	return 1;
}

bool CNFC_Device::WriteLog(CString str, UINT  nStatus)
{
	if (!bLogEnabled)
	{
		bLogEnabled = 1;
		return 1;
	}
	if (nStatus == LOG_MEASURED && !bManufact)return 1;
	if (nStatus == LOG_CRCERROR)
	{
		if (!bManufact)return 1;
	}
	if (IsWindow(wndNotify->GetSafeHwnd()))
	{
		CString strOut;
		strOut.Format("%s --- %s", CDateTime::GetCurrent().FormatStandard(1,0),str);
		NFC_LOG* l = new NFC_LOG;
		l->strLog = strOut;
		if (nStatus == LOG_ERROR)l->clr = 0xff;
		else if (nStatus == LOG_PROCESSCOMPLETE) l->clr = 0xff0000;
		else if (nStatus == LOG_MEASURED) l->clr = RGB(128, 0, 255);
		else if (nStatus == LOG_ATTENTION) l->clr = RGB(255, 128, 64);
		else if (nStatus == LOG_CRCERROR)l->clr = 0xff;
		else l->clr = 0;
		l->from = this;
		wndNotify->PostMessage(WM_NFC_DEVICE_LOG, (WPARAM)l);
	}
	return 1;
}

bool CNFC_Device::Calibrate()
{
	if (bInMonitoring)bPauseMonitoring = 1;
	else
	{
		BreakExecution();
	}
	WaitComplete(1);
	evService.SetEvent();
	OperationStatusChange(STARTALL_CALIBRATION, 0);
	hCommandThread = CreateThread(NULL, 0, CalibrationThread, (void*)this, 0, 0);
	return 1;
}

bool CNFC_Device::CompareArrays(float* f1, float* f2, int nCount)
{
	for (int i = 0; i < nCount; i++)
	{
		double dev = fabs(f1[i] - f2[i]);
		if (dev > 0.001)
		{
			return 0;
		}
	}
	return 1;
}

DWORD WINAPI CNFC_Device::CalibrationThread(void* ptr)
{
	InitThread();
	CString str,buf;
	CNFC_Device* dev = (CNFC_Device*)ptr;
	CALIBRATION* c = &dev->calibration_settings;
	c->bInCalibration = 1;
	SENSORS_INFO* s = &dev->sensors_info,sensor_save;
	dev->additional_pars.nCommandTryCount = 10;
	dev->WriteLog(S_O::LoadString(IDS_CALIBRATION), LOG_PROCESSCOMPLETE);
	if (dev->dev_settings.base.O2Chem == 0)c->GasCalbInitOfs[1] = 3;
	else c->GasCalbInitOfs[1] = 40;
	c->GasCalbInitOfs[2] = 40;
	c->GasCalbInitOfs[3] = 40;
	c->GasCalbInitAmp[1] = 1;
	c->GasCalbInitAmp[2] = 1;
	c->GasCalbInitAmp[3] = 1;
	bool GasCalbSensInit[5],CheckSensor[5],bCalibGases=0,bCoeffChanged[5];
	BYTE SensorsUnitsPrev=dev->dev_settings.SensorsUnits, SensorsPrecisionsPrev= dev->dev_settings.SensorsPrecisions;
	for (int i = 1; i <= 4; i++)
	{
		c->GasCalbSensFinished[i] = !c->GasCalbSens[i];
		GasCalbSensInit[i] = CheckSensor[i] = c->GasCalbSens[i];
		bCoeffChanged[i] = 0;
		if (i < 4 && GasCalbSensInit[i])
		{
			if (i == 2 && dev->GetSelectedGas(GAS_CO) == CO_MPC)continue;
			if (i == 3 && dev->GetSelectedGas(GAS_H2S) == H2S_MPC)continue;
			bCalibGases = 1;
		}
	}
	try
	{
		c->nResult = 0;
		if (!dev->TestIfDeviceChanged())throw 0;
		if (!dev->dev_settings.bLoaded)
		{
			dev->GetSettings();
			if (!dev->WaitComplete())throw 0;
		}
		SensorsUnitsPrev = dev->dev_settings.SensorsUnits;
		SensorsPrecisionsPrev = dev->dev_settings.SensorsPrecisions;
		bool bUpdateUnits = 0,bUpdatePrecisions=0;
/*		if (dev->dev_settings.base.O2Chem)
		{
			if (dev->dev_settings.SensorsUnits != 1)
			{
				dev->dev_settings.SensorsUnits = 1;
				bUpdateUnits = 1;
			}
		}
		else*/
		if (dev->dev_settings.SensorsUnits != 0)
		{
			UINT g[] = { GAS_O2,GAS_CO,GAS_H2S };
			UINT nSet = 0;
			for (int i = 0; i < 3; i++)
			{
				if (i == 1 && dev->GetSelectedGas(GAS_CO) == CO_MPC)continue;
				if (i == 2 && dev->GetSelectedGas(GAS_H2S) == H2S_MPC)continue;
				if (!dev->IsExtendedUnits(g[i]))continue;
				if (dev->GetUnits(g[i]) == 3)nSet |= dev->GetExtendedUnitsMask(g[i]);
			}
			dev->dev_settings.SensorsUnits = nSet;
			bUpdateUnits = 1;
		}
		if (dev->IsHaveProperty(PRECISION))
		{
			UINT n[] = { 1<<3,1<<0,1<<1,1<<2};

			for (int i = 0; i < 4; i++)
			{
				if (c->GasCalbSens[i+1] && (dev->dev_settings.SensorsPrecisions & n[i]))
				{
					bUpdatePrecisions = 1;
					dev->dev_settings.SensorsPrecisions &= ~n[i];
				}
			}
		}
		if (bUpdateUnits)
		{
			dev->SetSensorUnits(0);
			if (!dev->WaitComplete())throw 0;
		}
		if (bUpdatePrecisions)
		{
			dev->SetSettings();
			if (!dev->WaitComplete())throw 0;
		}
		for (int k = 2; k <= 4; k++)//optical sensor calibration
		{
			if (!c->GasCalbSens[k])continue;
			if (k == 2 && dev->GetSelectedGas(GAS_CO) != CO_MPC)continue;
			if (k == 3 && dev->GetSelectedGas(GAS_H2S) != H2S_MPC)continue;
			c->GasCalbSens[k] = 0;
			int i = 0;
			for (i = 0; i < 2; i++)
			{
				if (c->bCalibZero)str = "ZERO2";
				else
				{
					UINT nRef = c->GasCalbRefVal[k];
					if (nRef > 9999)nRef = 9999;
					str.Format("CALB %04u", nRef);
				}
				dev->additional_pars.strMIPEXCommand = str;
				dev->SendCommandMIPEX();
				if (!dev->WaitComplete())throw 0;
				if (dev->additional_pars.strLastMIPEXAnswer.Find("OK") != -1)break;
			}
			if (i == 2)
			{
				buf = dev->additional_pars.strLastMIPEXAnswer;
				if (buf == "")str = "No answer";
				else
				{
					buf.Replace("\r", "");
					str.Format("Answer: %s", buf);
				}
				dev->strAttention.Format("MIPEX: %s", str);
				dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONFAILED) + ". " + dev->strAttention, LOG_ERROR);
				c->nResult = RESULT_ERROR;
				throw 2;
			}
			c->GasCalbSensFinished[k] = 1;
		}
		dev->GetAllCoeffs();
		if (!dev->WaitComplete())throw 0;
		sensor_save = *s;
		if (!bCalibGases)
		{
			dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONCOMPLETE), LOG_PROCESSCOMPLETE);
			c->nResult = RESULT_GOOD;
		}
		SENSORS_INFO chk;
		float* coeff[] = { 0,s->fO2Coeff,s->fCOCoeff,s->fH2SCoeff };
		float* chkcoeff[] = { 0,chk.fO2Coeff,chk.fCOCoeff,chk.fH2SCoeff };
		char* coeffDescr[] = { "InitOffs","InitAmp","TarOffs","TarAmp" };
		CString gas[] = { 0,dev->GetGasString(GAS_O2),dev->GetGasString(GAS_CO),dev->GetGasString(GAS_H2S)};
		UINT nGas[] = { 0,GAS_O2,GAS_CO,GAS_H2S};
		WORD* wVal[] = { 0,&dev->sensors_data.O2,&dev->sensors_data.CO,&dev->sensors_data.H2S };
		WORD* wADCVal[] = { 0,&dev->sensors_data.O2Volt,&dev->sensors_data.COVolt,&dev->sensors_data.H2SVolt };
		double dOffsInc[] = { 0.0,dev->dev_settings.base.O2Chem ? 70.0 : 5.0,70.0,70.0 };
		WORD nZeroCompareVal[] = { 0,2,40,40 };
		double dCalCompare[] = { 0,1,c->GasCalbRefVal[2] * 0.04,c->GasCalbRefVal[3] * 0.05 };

		bool bInitCoeffs = c->bCalibZero;
		CRealtimeObject time;
		int i = 0;
		while (bCalibGases)
		{
			if (bInitCoeffs)
			{
				for (i = 1; i <= 3; i++)
				{
					if (!c->GasCalbSens[i])continue;
					if (dev->bManufact && ((c->bCalibZero && c->GasCalbSensFinished[i]) || !c->bCalibZero))
					{
						coeff[i][2] = 0;
						coeff[i][3] = 1;
					}
					if (c->bCalibZero)
					{
						if (dev->bManufact)coeff[i][0] = (float)c->GasCalbInitOfs[i];
						else coeff[i][2] = (float)c->GasCalbInitOfs[i];
					}
					else
					{
						if (dev->bManufact)coeff[i][1] = (float)c->GasCalbInitAmp[i];
						else coeff[i][3] = (float)c->GasCalbInitAmp[i];
					}
					str.Format("%s: TarAmp=%g, TarOfs=%g, InitAmp=%g, InitOfs=%.2f", gas[i], coeff[i][3], coeff[i][2], coeff[i][1], coeff[i][0]);
					dev->WriteLog(str, LOG_MEASURED);
				}
			}
			if(bInitCoeffs)
			{
				//check initamp and taramp
				str = "";
				for (i = 1; i <= 3; i++)
				{
					if (!GasCalbSensInit[i])
					{
						CheckSensor[i] = 0;
						continue;
					}
					CheckSensor[i] = 1;
					for (int k = 1; k < 4; k += 2)
					{
						if (fabs(coeff[i][k]) >= dev->calibration_settings.dAmpGasFlowError)
						{
							CheckSensor[i] = 0;
							break;
						}
					}
				}
				if (c->GasCalbSens[1])
				{
					if (CheckSensor[1])
					{
						dev->SetO2Coeff();
						bCoeffChanged[1] = 1;
						if (!dev->WaitComplete())throw 0;
					}
					if (c->GasCalbSensFinished[1])c->GasCalbSens[1] = 0;
				}
				if (c->GasCalbSens[2])
				{
					if (CheckSensor[2])
					{
						dev->SetCOCoeff();
						bCoeffChanged[2] = 1;
						if (!dev->WaitComplete())throw 0;
					}
					if (c->GasCalbSensFinished[2])c->GasCalbSens[2] = 0;
				}
				if (c->GasCalbSens[3])
				{
					if (CheckSensor[3])
					{
						dev->SetH2SCoeff();
						bCoeffChanged[3] = 1;
						if (!dev->WaitComplete())throw 0;
					}
					if (c->GasCalbSensFinished[3])c->GasCalbSens[3] = 0;
				}
			}
			else bInitCoeffs = 1;
			if (c->GasCalbSens[1] || c->GasCalbSens[2] || c->GasCalbSens[3])
			{
				for (i = 0; i < 2; i++)
				{
					str.Format("%s %gs", S_O::LoadString(IDS_WAIT),c->dParamWaitTime);
					dev->WriteLog(str);
					if (!time.WaitTime(c->dParamWaitTime, 0, &dev->evBreak, dev->progressCtrl))throw 0;
					dev->WriteLog(S_O::LoadString(IDS_MEASURE));
					dev->GetSensorData();
					if (!dev->WaitComplete())throw 0;
					str.Format("%s O2=%u, CO=%u, H2S=%u", S_O::LoadString(IDS_MEASURED),(UINT)*wVal[1], (UINT)*wVal[2], (UINT)*wVal[3]);
					dev->WriteLog(str, LOG_MEASURED);
					str.Format("%s ADC O2=%u, CO=%u, H2S=%u", S_O::LoadString(IDS_MEASURED), (UINT)*wADCVal[1], (UINT)*wADCVal[2], (UINT)*wADCVal[3]);
					dev->WriteLog(str, LOG_MEASURED);
					str = "";
					if (c->bCalibZero)
					{
						for (int k = 1; k <= 3; k++)
						{
							if (!c->GasCalbSens[k] || !CheckSensor[k])continue;
							c->GasCalbVals[k][i] = (double)(*wVal[k]) / dev->GetGasDevider(nGas[k],1) / coeff[k][3] - coeff[k][2];
							if (dev->bManufact)
							{
								c->GasCalbVals[k][i] = c->GasCalbVals[k][i] / coeff[k][1] - coeff[k][0];
							}
							buf.Format("%sCalc=%.5f",gas[k],c->GasCalbVals[k][i]);
							if (str != "")str += ",";
							str += buf;
						}
					}
					else
					{
						for (int k = 1; k <= 3; k++)
						{
							if (!c->GasCalbSens[k] || !CheckSensor[k])continue;
							c->GasCalbVals[k][i] = (double)(*wVal[k]) / dev->GetGasDevider(nGas[k],1) / coeff[k][3];
							if (dev->bManufact)
							{
								c->GasCalbVals[k][i] = (c->GasCalbVals[k][i] - coeff[k][2]) / coeff[k][1];
							}
							buf.Format("%sCalc=%.5f", gas[k], c->GasCalbVals[k][i]);
							if (str != "")str += ",";
							str += buf;

						}
					}
					if (str != "")dev->WriteLog(str, LOG_MEASURED);
					else break;
				}
				bool bContinue = 1;
				if (i != 2)bContinue=0;
				if (c->bCalibZero)
				{
					for (int i = 1; i <= 3; i++)
					{
						if (!c->GasCalbSens[i] || !CheckSensor[i])continue;
						double dAvg = (c->GasCalbVals[i][0] + c->GasCalbVals[i][1]) / 2;
						str.Format("%s: Avg=%g", gas[i], dAvg);
						dev->WriteLog(str, LOG_MEASURED);
						if (*wVal[i] == 0)
						{
							c->GasCalbInitOfs[i] += dOffsInc[i];
							str.Format("%s: %s", gas[i], S_O::LoadString(IDS_COEFFCORRECTED));
						}
						else
						{
							c->GasCalbInitOfs[i] = -dAvg;
							c->GasCalbSensFinished[i] = 1;
							str.Format("%s: %s", gas[i], S_O::LoadString(IDS_COEFFCALCULATED));
						}
						dev->WriteLog(str,LOG_MEASURED);
					}
				}
				else
				{
					for (int i = 1; i <= 3; i++)
					{
						if (!c->GasCalbSens[i] || !CheckSensor[i])continue;
						double dAvg = (c->GasCalbVals[i][0] + c->GasCalbVals[i][1]) / 2;
						str.Format("%s: Avg=%g", gas[i], dAvg);
						dev->WriteLog(str, LOG_MEASURED);
						if (dAvg == 0)
						{
							c->GasCalbSens[i] = 0;
							c->GasCalbSensFinished[i] = 1;
							str.Format("%s: %s", gas[i], S_O::LoadString(IDS_REMOVEZEROSENSOR));
						}
						else
						{
							c->GasCalbInitAmp[i] = c->GasCalbRefVal[i] / dev->GetGasDevider(nGas[i],1) / dAvg;
							c->GasCalbSensFinished[i] = 1;
							str.Format("%s: %s", gas[i], S_O::LoadString(IDS_COEFFCALCULATED));
						}
						dev->WriteLog(str,LOG_MEASURED);
					}
				}
				if(bContinue)continue;//correct coeff
			}
			dev->WriteLog(S_O::LoadString(IDS_STARTCHECKING));
			str.Format("%s %gs", S_O::LoadString(IDS_WAIT), c->dParamWaitTime);
			dev->WriteLog(str);
			if (!time.WaitTime(c->dParamWaitTime, 0, &dev->evBreak,dev->progressCtrl))throw 0;
			dev->GetSensorData();
			if (!dev->WaitComplete())throw 0;
			str.Format("%s O2=%u, CO=%u, H2S=%u", S_O::LoadString(IDS_MEASUREDCALIBRATED),(UINT)*wVal[1], (UINT)*wVal[2], (UINT)*wVal[3]);
			dev->WriteLog(str, LOG_MEASURED);
			str.Format("%s ADC O2=%u, CO=%u, H2S=%u", S_O::LoadString(IDS_MEASUREDCALIBRATED), (UINT)*wADCVal[1], (UINT)*wADCVal[2], (UINT)*wADCVal[3]);
			dev->WriteLog(str, LOG_MEASURED);
			chk = dev->sensors_info;
			dev->GetAllCoeffs();
			if (!dev->WaitComplete())throw 0;
			if ((CheckSensor[1] && !CompareArrays(coeff[1], chk.fO2Coeff, 18)) ||
				(CheckSensor[2] && !CompareArrays(coeff[2], chk.fCOCoeff, 13)) ||
				(CheckSensor[3] && !CompareArrays(coeff[3], chk.fH2SCoeff, 13)))
			{
				dev->WriteLog(S_O::LoadString(IDS_COEFFTRANSFERERROR), LOG_ERROR);
				break;
			}
			str = "";
			for (int i = 1; i <= 3; i++)
			{
				if (!GasCalbSensInit[i] || !CheckSensor[i])continue;
				buf = "";
				if (c->bCalibZero)
				{
					if(*wVal[i] > nZeroCompareVal[i])	buf.Format("%s: %s > %u (=%u)", gas[i], S_O::LoadString(IDS_VALUE), (UINT)nZeroCompareVal[i],(UINT) *wVal[i]);
				}
				else
				{
					double d = c->GasCalbRefVal[i] - (double)(UINT)*wVal[i];
					if (fabs(d) > dCalCompare[i])buf.Format("%s: %s > %g (=%g)", gas[i], S_O::LoadString(IDS_DIFFERENCE), dCalCompare[i]/ dev->GetGasDevider(nGas[i],1),(double)(UINT)*wVal[i]/ dev->GetGasDevider(nGas[i],1));
				}
				if (buf != "")
				{
					if (str != "")str +=", ";
					str += buf;
				}
			}
			if (str != "")
			{
				buf=S_O::LoadString(IDS_DEVICECALIBRATIONFAILED)+ ": " + str;
				dev->WriteLog(buf, LOG_ERROR);
				c->nResult = RESULT_ERROR;
				break;
			}
			//check initamp and taramp
			str = "";
			for (i = 1; i <= 3; i++)
			{
				if (!GasCalbSensInit[i])continue;
				for (int k = 1; k < 4; k += 2)
				{
					if (fabs(chkcoeff[i][k]) >= dev->calibration_settings.dAmpGasFlowError)
					{
						buf.Format("%s %s %s=%g", S_O::LoadString(IDS_COEFF), gas[i], coeffDescr[k], chkcoeff[i][k]);
						if (str != "")str += ",";
						str += buf;
					}
				}
			}
			if (str != "")
			{
				dev->strAttention.Format("%s. %s: %s", S_O::LoadString(IDS_CHECKGASFLOW), S_O::LoadString(IDS_ERRORCAUSE), str);
				dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONFAILED) + ". " + dev->strAttention, LOG_ERROR);
				c->nResult = RESULT_ERROR;
				break;
			}
			//check coeffs
			str = "";
			for (i = 1; i <= 3; i++)
			{
				if (!GasCalbSensInit[i])continue;
				for (int k = 0; k < 4; k++)
				{
					if (c->dCoeffMin[k] != c->dCoeffMax[k] && (fabs(coeff[i][k]) <= c->dCoeffMin[k] || fabs(coeff[i][k]) >= c->dCoeffMax[k]))
					{
						buf.Format("%s %s=%g", gas[i], coeffDescr[k], coeff[i][k]);
						if (str != "")str += ",";
						str += buf;
					}
				}
			}

			if (str != "")
			{
				dev->strAttention.Format("%s: %s", S_O::LoadString(IDS_ATTENTION), str);
				dev->WriteLog(dev->strAttention, LOG_ATTENTION);
				dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONCOMPLETE), LOG_PROCESSCOMPLETE);
				c->nResult = RESULT_ATTENTION;
				break;
			}

			dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONCOMPLETE), LOG_PROCESSCOMPLETE);
			c->nResult = RESULT_GOOD;
			break;
		}
	}
	catch (int n)
	{
		if (n == 0)
		{
			c->nResult = RESULT_TERMINATED;
			dev->WriteLog(S_O::LoadString(IDS_DEVICECALIBRATIONTERMINATED), LOG_ERROR);
		}
		else c->nResult = RESULT_ERROR;
	}
	bool bGetSettings = 0, bGetAlarms = 0, bGetCoeff = 0;
	if (c->nResult != RESULT_GOOD && c->nResult!= RESULT_ATTENTION)
	{
		for (int i = 1; i <= 3; i++)
		{
			if (bCoeffChanged[i])
			{
				*s = sensor_save;
				bGetCoeff = 1;
				break;
			}
		}
		if (bCoeffChanged[1])
		{
			dev->SetO2Coeff();
			dev->WaitComplete();
		}
		if (bCoeffChanged[2])
		{
			dev->SetCOCoeff();
			dev->WaitComplete();
		}
		if (bCoeffChanged[3])
		{
			dev->SetH2SCoeff();
			dev->WaitComplete();
		}
	}
	if (SensorsUnitsPrev != dev->dev_settings.SensorsUnits)
	{
		dev->dev_settings.SensorsUnits = SensorsUnitsPrev;
		dev->SetSensorUnits(0);
		dev->WaitComplete();
		bGetSettings = bGetAlarms = 1;
	}
	if (SensorsPrecisionsPrev != dev->dev_settings.SensorsPrecisions)
	{
		dev->dev_settings.SensorsPrecisions = SensorsPrecisionsPrev;
		dev->SetSettings();
		dev->WaitComplete();
		bGetSettings = 1;
	}
	if (bGetCoeff)
	{
		dev->GetAllCoeffs();
		dev->WaitComplete();
	}
	if (bGetSettings)
	{
		dev->GetSettings();
		dev->WaitComplete();
	}
	if (bGetAlarms)
	{
		dev->GetAlarms();
		dev->WaitComplete();
	}
	c->bInCalibration = 0;
	dev->additional_pars.nCommandTryCount = DEFAULT_COMMAND_TRY_COUNT;
	dev->OperationStatusChange(STARTALL_CALIBRATION, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	dev->bPauseMonitoring = 0;
	return 1;
}

CString CNFC_Device::GetCommandString(UINT nCommand)
{
	CString str;
	switch (nCommand)
	{
	case NFCMipexBackup:
		str = "MipexBackup";
		break;
	case NFCMipexRestore:
		str = "MipexRestore";
		break;
	case NFCSendMipexBackup:
		str = "SendMipexBackup";
		break;
	case NFCSaveMipexBackup:
		str = "SaveMipexBackup";
		break;
	case GetCH4Poly:
		str = "GetCH4Poly";
		break;
	case GetInterfacesByte:
		str = "GetInterfaces";
		break;
	case SetInterfacesByte:
		str = "SetInterfaces";
		break;
	case SetScaleTime:
		str = "SetScaleTime";
		break;
	case GetLORAInfo:
		str = "GetLORAInfo";
		break;
	case SendLoraKeyByte:
		str = "SetLoraKey";
		break;
	case ModeDevByte:
		str = "SetDevMode";
		break;
	case NFCPowerDownByte:
		str = "NFCPowerDown";
		break;
	case ReplButtByte:
		str = "ReplaceBattery";
		break;
	case SetSerialNumberByte:
		str = "SetSerialNumber";
		break;
	case SetTimeByte:
		str = "SetTime";
		break;
	case SetSensUnitsByte:
		str = "SetSensUnits";
		break;
	case GetRegVibroByte:
		str = "GetRegVibro";
		break;
	case SetSensStatusByte:
		str = "SetSensStatus";
		break;
	case GetSensStatusByte:
		str = "GetSensStatus";
		break;
	case ClearLogByte:
		str = "ClearLogByte";
		break;
	case SetSettingsByte:
		str = "SetSettingsByte";
		break;
	case OTAARejoin:
		str = "OTAARejoin";
		break;
	case SetLoraOTAA:
		str = "SetLoraOTAA";
		break;
	case GetLoraOTAA:
		str = "GetLoraOTAA";
		break;
	case SwitchBeepByte:
		str = "SwitchBeep";
		break;
	case UnlockSignByte:
		str = "SwitchAlarm";
		break;
	case SwitchVibrByte:
		str = "SwitchVibro";
		break;
	case SetCommandMIPEXByte:
		str = "SendMIPEXCommand";
		break;
	case SetCommandUARTByte:
		str = "SendUARTCommand";
		break;
	case SetCommandSPIByte:
		str = "SendSPICommand";
		break;
	case GetDataSensorsByte:
		str = "GetData";
		break;
	case SetLogTimeOutByte:
		str = "SetLogTimeOut";
		break;
	case BlinkByte:
		str = "Blink";
		break;
	case OneBeepByte:
		str = "Beep";
		break;
	case RebootByte:
		str = "Reboot";
		break;
	case GetGasRangeByte:
		str = "GetGasRange";
		break;
	case GetSensVRangeByte:
		str = "GetVRange";
		break;
	case SetGasRangeByte:
		str = "SetGasRange";
		break;
	case SetSensVRangeByte:
		str = "SetSensVRange";
		break;
	case SetAlarmSignalingByte:
		str = "SetAlarmSignalling";
		break;
	case GetRadioSettingsByte:
		str = "GetRadioSettings";
		break;
	case NullStatByte:
		str = "WorkStat";
		break;
	case GetInfDeviceByte:
		str = "GetDeviceInfo";
		break;
	case GetAlarmSignalingByte:
		str = "GetAlarms";
		break;
	case SetRadioByte:
		str = "SetRadio";
		break;
	case SetWifiByte:
		str = "SetWifi";
		break;
	case GetSettingsByte:
		str = "GetSettings";
		break;
	case GetLoraKeyByte:
		str = "GetLoraKey";
		break;
	case GetWorkStatByte:
		str = "GetWorkStat";
		break;
	case GetAllCoefByte:
		str="GetAllCoef";
		break;
	case SendSensAccelByte:
		str = "SendSensAccel";
		break;
	case SendCH4CoefByte:
		str = "SendCH4Coef";
		break;
	case SendCoefO2Byte:
		str = "SendCoefO2";
		break;
	case SendCoefCOByte:
		str = "SendCoefCO";
		break;
	case SendCoefH2SByte:
		str = "SendCoefH2S";
		break;
	case SetCH4Mult:
		str = "SendCH4CoefMult";
		break;
	}
	return str;
}

void CNFC_Device::Sleep(UINT nMS)
{
	WaitTime((double)nMS / 1000);
}

bool CNFC_Device::GetLastLogAddr()
{
	return SendCommand(GetLastLogAddrByte);
}

bool CNFC_Device::ParseLastLogAddr(CByteArray* arr)
{
	log.LastLogAddr = *(UINT*)&(*arr)[27];
	return 1;
}

bool CNFC_Device::SendSwitchBootloader()
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x07;
	write_buff[3] = 0x02;
	write_buff[4] = 0x21;
	write_buff[5] = 0x00;
	write_buff[6] = 0x66;
	write_buff[7] = 0x99;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	return Write(write_buff, 10);
}

bool CNFC_Device::SendExitBootloader()
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x07;
	write_buff[3] = 0x02;
	write_buff[4] = 0x21;
	write_buff[5] = 0x00;
	write_buff[6] = 0x55;
	write_buff[7] = 0x88;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	return Write(write_buff, 10);
}


bool CNFC_Device::ReadOneByte()
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x04;
	write_buff[3] = 0x02;
	write_buff[4] = 0x23;
	write_buff[5] = 0x00;
	write_buff[6] = 0x07;
	write_buff[7] = 0x00;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	return Write(write_buff, 10);
}

bool CNFC_Device::IsInBootLoader(BYTE& nBootloaderVersion)
{
	if (!(arrRead[0] == 0x80 && arrRead[1] == 0x24 && arrRead[3] == 0x0A && arrRead[4] == 0x0B && arrRead[5] == 0x0C))return 0;
	if (arrRead[6] == 0x0D)nBootloaderVersion = 0;
	else if (arrRead[6] == 0x40)nBootloaderVersion = 1;
	else return 0;
	return 1;
}

bool CNFC_Device::SwitchToBootloader(BYTE& nBootloaderVersion,bool bCheckFirst)
{
	dev->ClearReadBuffer();
	WriteLog(S_O::LoadString(IDS_SWITCHINGBOOTLOADER));
	if (bCheckFirst)
	{
		if (IsInBootLoader(nBootloaderVersion))
		{
			SwitchOff();
			WriteLog(S_O::LoadString(IDS_INBOOTLOADER), LOG_PROCESSCOMPLETE);
			return 1;
		}
	}
	if (!SwitchOn())return 0;
	for (int i = 0; i < 100; i++)
	{
		if (!SendSwitchBootloader())return 0;
		if (!GetAnswer(0, 5000))return 0;
		if (arrRead[0] != 0x80 || arrRead[1] != 0x04)
		{
			Sleep(100);
			continue;
		}
		Sleep(2000);
		if (!ReadOneByte())return 0;
		if (!GetAnswer(0, 5000))return 0;
		if (IsInBootLoader(nBootloaderVersion))
		{
			SwitchOff();
			WriteLog(S_O::LoadString(IDS_INBOOTLOADER),LOG_PROCESSCOMPLETE);
			ExchangeSuccess(1);
			return 1;
		}
	}
	return 0;
}

bool CNFC_Device::GetLog()
{
	BreakExecution();
	WaitComplete(1);
	evService.SetEvent();
	OperationStatusChange(STARTALL_GETLOG, 0);
	hCommandThread = CreateThread(NULL, 0, LogThread, (void*)this, 0, 0);
	return true;
}


DWORD WINAPI CNFC_Device::LogThread(void* ptr)
{
	InitThread();
	CString str;
	CNFC_Device* dev = (CNFC_Device*)ptr;
	EXCHANGE_SETTINGS save = dev->exchange_settings;
	dev->log.records.RemoveAll();
	try
	{
		if (!dev->TestIfDeviceChanged())throw 0;
		if (dev->dev_info.nSWVer < 50)dev->log.nVersion = 0;
		else if (dev->dev_info.nSWVer < 80)dev->log.nVersion = 1;
		else if (dev->dev_info.nSWVer < 99)dev->log.nVersion = 2;
		else dev->log.nVersion = 3;
		dev->log.bShort = !dev->bManufact || dev->log.nVersion == 0;
		dev->log.nResult = 0;
		if (!dev->dev_settings.bLoaded)
		{
			dev->GetSettings();
			if (!dev->WaitComplete() || !dev->dev_settings.bLoaded)throw 0;
		}
		if (dev->IsHaveProperty(SHORTLOG))
		{
			dev->log.bShort2 = (dev->dev_settings.Options & DEVOPT_SHORTLOG) != 0;
			if(!dev->log.bShort2)dev->log.bMIPEXLog = (dev->dev_settings.Options & DEVOPT_MIPEXLOG) != 0;
		}
		else dev->log.bShort2 = dev->log.bMIPEXLog = 0;
		if (!dev->sensors_info.bStatusLoaded)
		{
			dev->GetSensorStatus();
			if (!dev->WaitComplete())throw 0;
		}
		dev->GetLastLogAddr();
		if (!dev->WaitComplete())throw 0;
		dev->exchange_settings.nDataWaitMS = 6;
		dev->exchange_settings.nSwitchOnWait = 450;
		BYTE nVersion = 0;
		if (!dev->SwitchToBootloader(nVersion))
		{
			dev->WriteLog(S_O::LoadString(IDS_ERROR),LOG_ERROR);
			throw 0;
		}
		if (IsWindow(dev->wndNotify->GetSafeHwnd()))
		{
			CNFC_Command* c = new CNFC_Command;
			c->dev = dev;
			c->nCommand = GetLogByte;
			c->nRet = 1;
			dev->wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
		}
		int nCount = 0;
		int nMax = MaxBlockLogNorm - 20, nInc = dev->log.bFromPast ? 1 : -1, nS, nE;
		int nLastBlock = dev->log.LastLogAddr / PageLogNorm ;
		for (int nStage = 0; nStage < 2; nStage++)
		{
			if (dev->log.bFromPast)
			{
				if (nStage == 1)
				{
					nS = 21;
					nE = nLastBlock;
				}
				else
				{
					nS = nLastBlock +1;
					nE = MaxBlockLogNorm+1;
				}
			}
			else
			{
				if (nStage == 1)
				{
					nS = MaxBlockLogNorm+1;
					nE = nLastBlock;
				}
				else
				{
					nS =  nLastBlock - 1;
					nE = 21;
				}
			}
			for (int i = nS;(nInc>0)?(i <= nE):(i>=nE); i+=nInc)
			{
				if (dev->IsBreak())throw 2;
				for (int j = 0; j < 25; j++)
				{
					if (j == 24)throw 0;
					try
					{
						if (dev->IsBreak())throw 2;
						if (!dev->SwitchOn())throw 0;
						int nBlockRead = 16;
						UINT nAddr = i * dev->log.nPageLog + 32;
						dev->Sleep(dev->exchange_settings.nDataWaitMS);
						BYTE nNumPacket = 1;
						BYTE buf[10], bufAddit[4];
						buf[0] = 0x00;
						buf[1] = 0x00;
						buf[2] = LOBYTE(LOWORD(nAddr));
						buf[3] = HIBYTE(LOWORD(nAddr));
						buf[4] = LOBYTE(HIWORD(nAddr));
						buf[5] = HIBYTE(HIWORD(nAddr));
						bufAddit[0] = 0xA2;
						bufAddit[1] = 0xB2;
						bufAddit[2] = buf[2];
						bufAddit[3] = buf[3];
						int nBlocks = 1;
						dev->progressCtrl->SetRange(0, nBlocks + 1 + 1 + 3);
						for (int i = 0; i <= nBlocks; i++)
						{
							if (!dev->SendBuffer(&buf[i * 4], i))throw 0;
							dev->progressCtrl->SetPos(i + 1);
						}
						if (!dev->SendBlock0ForBoot(bufAddit))throw 0;
						dev->progressCtrl->SetPos(nBlocks + 2);
						if (!dev->SwitchOff())throw 0;
						dev->Sleep(dev->exchange_settings.nSwitchOffWait);
						dev->progressCtrl->SetPos(nBlocks + 3);
						if (!dev->SwitchOn())throw 0;
						dev->Sleep(dev->exchange_settings.nSwitchOnWait);
						dev->progressCtrl->SetPos(nBlocks + 4);
						int nBlockSize = 32;
						CByteArray get;
						get.SetSize((nBlockRead + 1) * nBlockSize + 4);
						for (int i = 0; i < nBlockRead; i++)
						{
							if (!dev->GetBuffer(i, 1))
							{
								throw 0;
							}
							for (int k = 0; k < nBlockSize; k++)
							{
								get[i * nBlockSize + k + 3] = dev->arrRead[k + 3];
							}
						}
						dev->progressCtrl->SetPos(nBlocks + 5);
						if (!dev->log.bShowBadData)
						{
							ULONG calc, send = *(UINT*)&get[487];
							g_crc.Calculate2(&get[7], 480, calc);
							if (calc != send)
							{
								dev->ExchangeSuccess(0);
								str.Format(S_O::LoadString(IDS_CRCERROR), nCount + 1);
								dev->WriteLog(str, LOG_CRCERROR);
								continue;
							}
						}
						dev->progressCtrl->SetPos(nBlocks + 4 + nBlockRead);
						if (!dev->SwitchOff())throw 0;
						nCount++;
						str.Format(S_O::LoadString(IDS_GETLOGBLOCK), nCount, nMax);
						dev->WriteLog(str);
						dev->ParseLog(&get, i);
						if (IsWindow(dev->wndNotify->GetSafeHwnd()))
						{
							CNFC_Command* c = new CNFC_Command;
							c->dev = dev;
							c->nCommand = GetLogByte;
							c->get = new CByteArray;
							c->get->Copy(get);
							c->nRet = 1;
							c->nParam = i;
							dev->wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
						}
						dev->ExchangeSuccess(1);
						break;
					}
					catch (int n)
					{
						if(n==0)dev->ExchangeSuccess(0);
					}
				}
			}
		}
		dev->WriteLog(S_O::LoadString(IDS_LOGCOMPLETE),LOG_PROCESSCOMPLETE);
		dev->log.nResult = RESULT_GOOD;
	}
	catch (int n)
	{
		if (dev->IsBreak())
		{
			dev->WriteLog(S_O::LoadString(IDS_LOGTERMINATED),LOG_ERROR);
			n = RESULT_TERMINATED;
		}
		else
		{
			dev->ExchangeSuccess(0);
			dev->WriteLog(S_O::LoadString(IDS_LOGFAILED), LOG_ERROR);
		}
		dev->log.nResult = n;
	}
	dev->ExitBootloader();
	dev->progressCtrl->SetPos(0);
	dev->exchange_settings = save;
	dev->OperationStatusChange(STARTALL_GETLOG, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return 1;
}

bool CNFC_Device::SendBlock0ForBoot(BYTE* buf)
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x07;
	write_buff[3] = 0x02;
	write_buff[4] = 0x21;
	write_buff[5] = 0x00;
	write_buff[6] = buf[0];
	write_buff[7] = buf[1];
	write_buff[8] = buf[2];
	write_buff[9] = buf[3];
	if (!Write(write_buff, 10))return 0;
	return GetAnswer();
}

UINT CNFC_Device::ConvertFromBin(BYTE* bin, int& nFrom, int nCount)
{
	UINT w=0;
	for (int i = nFrom; i < nFrom + nCount; i++)
	{
		w |= (bin[i] << (i - nFrom));
	}
	nFrom += nCount;
	return w;
}

void DEVICE_LOG_RECORD::SetFromMSB(double* to, BYTE* from, int nB)
{
	UINT n = 0;
	BYTE* ptr = (BYTE*)&n;
	for (int i = 0; i < nB; i++)
	{
		ptr[i] = from[nB - i - 1];
	}
	*to = n;
}

bool CNFC_Device::IsCorrectDate(UINT nDate)
{
	if (nDate == 0 || nDate == 0xffffffff)return 0;
	SYSTEMTIME st;
	CNFC_Device::TimeToSystemTIME(nDate, st);
	if (st.wYear <= 2000)return 0;
	CTime tm(st);
	return tm !=0;
}

bool CNFC_Device::ParseLog(CByteArray* arr, UINT nBlock)
{
	BYTE* b = &(*arr)[0];
	if (b[27] == 0 && b[28] == 0)
	{
//		WriteLog(S_O::LoadString(IDS_INCORRECTDATA),LOG_ERROR);
		return 0;
	}
	CArray< DEVICE_LOG_RECORD, DEVICE_LOG_RECORD&> a;
	DEVICE_LOG_RECORD r;
	if (log.nVersion > 1)
	{
		int nCount = 15;
		if (nBlock == 546)nCount = 1;
		if (log.bShort2)
		{
			int nS = 0;
			if (nBlock == 21)nS = 8;
			nCount *= 2;
			DWORD dw;
			for (int i = nS; i < nCount; i++)
			{
				BYTE* data = &b[i * 16 + 7];
				r.nDateTime = *(UINT*)data;
				if (!log.bShowBadData && !IsCorrectDate(r.nDateTime))continue;
				r.nFlag = *(WORD*)&data[4];
				r.nTemp = (double)(char)data[5];
				dw = data[6] | (data[7] << 8);
				r.nCO = dw;
				r.nCO /= GetGasDevider(GAS_CO,1);
				dw = data[8] | (data[9] << 8);
				r.nH2S = dw;
				r.nH2S /= GetGasDevider(GAS_H2S,1);
				dw = data[10] | (((data[12] >> 7) & 1) << 8);
				r.nO2 = dw ;
				r.nO2 /= GetGasDevider(GAS_O2,1);
				dw = data[11] | ((data[12] & 0x3f) << 8);
				r.nCH4 = dw;
				r.nCH4 /= GetGasDevider(GAS_MIPEX,1);
				dw = data[13] | ((data[14] & 0xf) << 8);
				r.nCOVolt = dw;
				dw = data[15] | (((data[14] >> 4) & 0xf) << 8);
				r.nH2SVolt = dw;
				if (!log.bFromPast)a.Add(r);
				else log.records.Add(r);
			}
		}
		else
		{
			int nS = 0;
			if (nBlock == 21)nS = 4;
			for (int i = nS; i < nCount; i++)
			{
				BYTE* data = &b[i * 32 + 7];
				r.nDateTime = *(UINT*)data;
				if (!log.bShowBadData && !IsCorrectDate(r.nDateTime))continue;
				r.nFlag = *(WORD*)&data[4];
				r.nTemp = (double)(char)data[6];
				r.nPressure = *(UINT*)&data[7];
				r.nO2 = *(WORD*)&data[11];
				if(!log.bMIPEXLog)r.nO2 /= GetGasDevider(GAS_O2,1);
				r.nCO = *(WORD*)&data[13];
				if (!log.bMIPEXLog)r.nCO /= GetGasDevider(GAS_CO,1);
				r.nH2S = *(WORD*)&data[15];
				if (!log.bMIPEXLog)r.nH2S /= GetGasDevider(GAS_H2S,1);
				r.nCH4 = *(WORD*)&data[17];
				if (!log.bMIPEXLog)r.nCH4 /= GetGasDevider(GAS_MIPEX,1);
				r.nO2Volt = *(WORD*)&data[19];
				r.nCOVolt = *(WORD*)&data[21];
				r.nH2SVolt = *(WORD*)&data[23];
				if (IsHaveProperty(VBATINLOG))r.nBattVolt = *(WORD*)&data[25];
				if (IsHaveProperty(LORACONFIRM))
				{
					r.nRSSI = (data[27] | (data[28] << 8));
					r.nSNR = data[29];
				}
				if (IsHaveProperty(SKIPSELFTTESTTRANSPORT))r.nExt = data[30];
				if (IsHaveProperty(FREEZE_IN_LOG))r.nFreeze = data[31];
				if (!log.bFromPast)a.Add(r);
				else log.records.Add(r);
			}
		}
	}
	else
	{
		int nCount = PageLogNorm / CountByteOneDataLog;
		int nS = 0;
		if (nBlock == 21)nS = 8;
		if (nBlock == 546)nCount = 1;
		BYTE bin[80];
		BYTE map[7] = { 9,13,11,14,12,12,9 };
		UINT u[7];
		UINT gas[] = { GAS_O2,GAS_CO,GAS_H2S,GAS_MIPEX };
		for (int i = nS; i < nCount; i++)
		{
			BYTE* data = &b[i * CountByteOneDataLog + 7];
			r.nDateTime = *(UINT*)data;
			if (!IsCorrectDate(r.nDateTime))continue;
			r.nFlag = data[4];
			r.nTemp = (double)(char)data[5];
			for (int i = 0; i < 10; i++)//convert to bin
			{
				for (int k = 0; k < 8; k++)
				{
					bin[i * 8 + k] = (data[i + 6] & (1 << k)) != 0;
				}
			}
			double* d = &r.nO2;
			int nFrom = 0;
			for (int k = 0; k < 7; k++)
			{
				u[k] = ConvertFromBin(bin, nFrom, map[k]);
				if (log.nVersion != 0 && k == 6)u[k] |= ((r.nFlag & 7) << 9);
				d[k] = u[k];
				if (k < 4)d[k] /= GetGasDevider(gas[k],1);
			}
			if (!log.bFromPast)a.Add(r);
			else log.records.Add(r);
		}
	}
	if (!log.bFromPast)
	{
		for (int i = a.GetSize() - 1; i >= 0; i--)
			log.records.Add(a[i]);
	}
	return 1;
}

bool CNFC_Device::FactoryReset()
{
	BreakExecution();
	WaitComplete(1);
	evService.SetEvent();
	OperationStatusChange(STARTALL_FACTORYRESET, 0);
	hCommandThread = CreateThread(NULL, 0, FactoryResetThread, (void*)this, 0, 0);
	return true;
}


DWORD WINAPI CNFC_Device::FactoryResetThread(void* ptr)
{
	InitThread();
	CNFC_Device* dev = (CNFC_Device*)ptr;
	UINT nGasSel = dev->additional_pars.nFactoryResetGasSel;
	dev->WriteLog(S_O::LoadString(IDS_STARTFACTORYRESET),LOG_PROCESSCOMPLETE);
	int nRet = 1;
	try
	{
		dev->GetAllCoeffs();
		if (!dev->WaitComplete())throw 0;
		if (nGasSel & GAS_O2)
		{
			dev->sensors_info.fO2Coeff[2] = 0;
			dev->sensors_info.fO2Coeff[3] = 1;
			if (dev->bManufact)
			{
				dev->sensors_info.fO2Coeff[0] = 0;
				dev->sensors_info.fO2Coeff[1] = 1;
			}
			dev->SetO2Coeff();
			if (!dev->WaitComplete())throw 0;
		}
		if ((nGasSel & GAS_CO) && dev->GetSelectedGas(GAS_CO) != CO_MPC)
		{
			dev->sensors_info.fCOCoeff[2] = 0;
			dev->sensors_info.fCOCoeff[3] = 1;
			if (dev->bManufact)
			{
				dev->sensors_info.fCOCoeff[0] = 0;
				dev->sensors_info.fCOCoeff[1] = 1;
			}
			dev->SetCOCoeff();
			if (!dev->WaitComplete())throw 0;
		}
		if ((nGasSel & GAS_H2S) && dev->GetSelectedGas(GAS_H2S) != H2S_MPC)
		{
			dev->sensors_info.fH2SCoeff[2] = 0;
			dev->sensors_info.fH2SCoeff[3] = 1;
			if (dev->bManufact)
			{
				dev->sensors_info.fH2SCoeff[0] = 0;
				dev->sensors_info.fH2SCoeff[1] = 1;
			}
			dev->SetH2SCoeff();
			if (!dev->WaitComplete())throw 0;
		}
		if ((nGasSel & GAS_MIPEX) || dev->IsPDK())
		{
			dev->additional_pars.strMIPEXCommand = "ZERO2";
			dev->SendCommandMIPEX();
			if (!dev->WaitComplete())throw 0;
		}
		dev->GetAllCoeffs();
		if (!dev->WaitComplete())throw 0;
	}
	catch (int)
	{
		nRet = 0;
	}
	dev->WriteLog(S_O::LoadString(nRet?IDS_FACTORYRESETCOMPLETE: IDS_FACTORYRESETCOMPLETEERROR), nRet?LOG_PROCESSCOMPLETE:LOG_ERROR);
	dev->additional_pars.nFactoryResetResult = nRet;
	dev->OperationStatusChange(STARTALL_FACTORYRESET, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return nRet;
}

bool CNFC_Device::ResetLog()
{
	return SendCommand(ClearLogByte);
}

bool CNFC_Device::StartFirmware()
{
	BreakExecution();
	WaitComplete(1);
	evService.SetEvent();
	OperationStatusChange(STARTALL_FIRMWARE, 0);
	hCommandThread = CreateThread(NULL, 0, FirmwareThread, (void*)this, 0, 0);
	return true;
}

bool CNFC_Device::ExitBootloader(int nWaitTime)
{
	evBreak.ResetEvent();
	SendExitBootloader();
	CRealtimeObject time;
	time.WaitTime(nWaitTime?nWaitTime:exchange_settings.nBootloaderExitWait, 0, &evBreak, progressCtrl);
	int nc = 0;
	for (int i = 0; i < 4; i++)
	{
		dev_info.bLoaded = 0;
		GetDeviceInfo(0, 1, 1);
		if (WaitComplete() && dev_info.bLoaded)
		{
			nc++;
			if (nc == 3)break;
			else
			{
				i--;
				continue;
			}
		}
		SendExitBootloader();
		for (int k = 0; k < 3; k++)
		{
			if (k)Sleep(1000);
			if (ReaderSequence1() != 1)continue;
			if (ReaderSequence2() != 1)continue;
			if (ReaderSequence3() != 1)continue;
			break;
		}
	}
	additional_pars.bInfoOnly = 0;
	ClearLoadState();
	ExchangeSuccess(1);
	return 1;
}



void CNFC_Device::SendFirmwareStatus(int nBlock, int nTotal)
{
	CNFC_Command* c = new CNFC_Command;
	c->dev = this;
	c->nCommand = FirmwareStatus;
	c->nRet = 1;
	c->nParam = nBlock | (nTotal << 16);
	nFirmwareProgress = nBlock | (nTotal << 16);
	wndNotify->PostMessage(WM_NFC_COMMAND_DONE, (WPARAM)c);
}

DWORD WINAPI CNFC_Device::FirmwareThread(void* ptr)
{
	InitThread();
	CString str;
	CNFC_Device* dev = (CNFC_Device*)ptr;
	dev->nFirmwareProgress = 0;
	dev->firmware_settings.nResult = 0;
	BYTE* data = 0,FW[4]="129",EndBytes[8];
	EXCHANGE_SETTINGS save = dev->exchange_settings;
	dev->exchange_settings.nRetryWaitMS = 1;
	dev->exchange_settings.nDataWaitMS = 5;
	try
	{
		CFile f;
		if (!f.Open(dev->firmware_settings.strPath, CFile::modeRead|CFile::shareDenyWrite))throw 0;
		UINT nLen = (UINT)f.GetLength();
		if (nLen<4)throw 0;
		data = new BYTE[nLen];
		f.Read(data, nLen);
		f.Close();
		for (UINT i = 0; i < nLen - 3; i++)
		{
			if (memcmp(&data[i], "SWH", 3) == 0)
			{
				str = &data[i];
				str.Replace("SWH", "");
				str.Replace("HWSSWL", ".");
				str.Replace("LWS", "");
				str.Replace(".", "");
				for (int k = 0; k < 3; k++)FW[k] = str.GetAt(k);
				break;
			}
		}
		int nBlocks = nLen / 384 + ((nLen % 384) != 0);
		dev->SendFirmwareStatus(0, nBlocks);
		dev->dev->ClearReadBuffer();
		for (int i = 0; i < 2; i++)
		{
			if (!dev->SendBeforeStartFirmware(i))throw 0;
			if (!dev->GetAnswer(0))throw 0;
		}
		if (!dev->SwitchOn())throw 0;
		if (!dev->ReadOneByte())throw 0;
		if (!dev->GetAnswer())throw 0;
		BYTE nBootloaderVersion = 0;
		if (!dev->SwitchToBootloader(nBootloaderVersion,1))throw 0;
		if (!dev->SwitchOn())throw 0;
		if (nBootloaderVersion == 0 && nBlocks > 256)
		{
			dev->WriteLog(S_O::LoadString(IDS_WRONGBOOTLOADER));
			throw 0;
		}
		dev->exchange_settings.nDataWaitMS = 2;
		dev->WriteLog(S_O::LoadString(IDS_FLASHERASE));
		for (int i = 0; i < 1000; i++)
		{
			if(!dev->ReadOneByte())throw 0;
			if (!dev->GetAnswer())throw 0;
			if (dev->IsInBootLoader(nBootloaderVersion))break;
			dev->Sleep(100);
			if (i == 999)throw 0;
		}
		bool bErase = 1;
		for (int i = 0; i < 1000; i++)
		{
			if (bErase && !dev->WriteC0D0())throw 0;
			if (!dev->GetAnswer())throw 0;
			if (dev->arrRead[0] == 0x80 && dev->arrRead[1] == 0x04)bErase=0;
			if (dev->arrRead[3] == 0xC1 && dev->arrRead[4] == 0xD1)break;
			if (i == 999)throw 0;
			dev->Sleep(100);
			if (!bErase && !dev->ReadOneByte())throw 0;
		}
		dev->WriteLog(S_O::LoadString(IDS_FLASHERASED));
		BYTE  BufM24LRCRC[396];
		dev->SendFirmwareStatus(0, nBlocks);
		for (int nB = 0; nB < nBlocks; nB++)
		{
			memset(BufM24LRCRC, 0, 396);
			for (UINT j = 0; j < 384; j++)
			{
				UINT n = j + 384 * nB;
				if (n >= nLen)break;
				BufM24LRCRC[j+4] = data[n];
			}
			BufM24LRCRC[0] = 0xB0;
			BufM24LRCRC[1] = 0xA0;
			BufM24LRCRC[2] = LOBYTE(nB);
			BufM24LRCRC[3] = HIBYTE(nB);
			g_crc.Calculate2(BufM24LRCRC, 388, *(ULONG*)&BufM24LRCRC[388]);
			for (int nBlockTry = 0; nBlockTry < 51; nBlockTry++)
			{
				if (nBlockTry == 50)throw 0;
				str.Format(S_O::LoadString(IDS_WRITEFLASHBLOCK), nB + 1, nBlocks);
				dev->WriteLog(str);
				try
				{
					for (int nF = 0; nF < 98; nF++)
					{
						for (int nRep = 0; nRep < 51; nRep++)
						{
							if (nRep == 50)throw 0;
							UINT nBuf = (nF == 97) ? 0 : (nF + 1);
							if (!dev->SendBuffer(&BufM24LRCRC[nBuf*4], nBuf))continue;
							else break;
						}
					}
					dev->Sleep(80);
					for (int i = 0; i < 3; i++)
					{
						if (i == 2)throw 0;
						if (!dev->GetAnswerFlash())throw 0;
						if (!dev->GetAnswer(0, 1000))throw 0;
						if (dev->arrRead[4] == 0xA1 && dev->arrRead[3] == 0xB1)
						{
							unsigned short int nNumBlock = *(unsigned short int*) & dev->arrRead[5];
							if(nNumBlock==nB)break;
						}
						dev->Sleep(1000);
					}
					dev->SendFirmwareStatus(nB+1, nBlocks);
					dev->ExchangeSuccess(1);
					break;
				}
				catch (int)
				{
					dev->ExchangeSuccess(0);
				}
			}
		}
		dev->WriteLog(S_O::LoadString(IDS_SENDLASTBYTES));
		EndBytes[0] = 0xE0;
		EndBytes[1] = 0xF0;
		EndBytes[2] = FW[0];
		EndBytes[3] = FW[1];
		EndBytes[4] = FW[2];
		EndBytes[5] = 0;
		EndBytes[6] = 0;
		EndBytes[7] = 0;
		for (int i = 0; i < 2; i++)
		{
			if (!dev->SendBuffer(&EndBytes[i * 4], 1))
			{
				dev->WriteLog(S_O::LoadString(IDS_ERROR),LOG_ERROR);
				throw 0;
			}
		}
		dev->ExitBootloader();
		dev->WriteLog(S_O::LoadString(IDS_FIRMWARECOMPLETE),LOG_PROCESSCOMPLETE);
		dev->firmware_settings.nResult = 1;
	}
	catch (int)
	{
		dev->ExchangeSuccess(0);
		dev->WriteLog(S_O::LoadString(IDS_FIRMWAREERROR),LOG_ERROR);
	}
	if (data)delete[] data;
	dev->SendFirmwareStatus(dev->firmware_settings.nResult, 0);
	dev->exchange_settings = save;
	dev->OperationStatusChange(STARTALL_FIRMWARE, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return 1;
}

bool CNFC_Device::SendBeforeStartFirmware(int n)
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x07;
	write_buff[3] = 0x02;
	write_buff[4] = 0x21;
	write_buff[5] = n;
	write_buff[6] = 0x00;
	write_buff[7] = 0x00;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	if (!Write(write_buff, 10))return 0;
	return 1;
}


bool CNFC_Device::WriteC0D0()
{
	BYTE write_buff[10];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x07;
	write_buff[3] = 0x02;
	write_buff[4] = 0x21;
	write_buff[5] = 0x00;
	write_buff[6] = 0xC0;
	write_buff[7] = 0xD0;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	return Write(write_buff, 10);
}


bool CNFC_Device::GetAnswerFlash()
{
	BYTE write_buff[64];
	write_buff[0] = 0x01;
	write_buff[1] = 0x04;
	write_buff[2] = 0x04;
	write_buff[3] = 0x02;
	write_buff[4] = 0x23;
	write_buff[5] = 0x00;
	write_buff[6] = 0x07;
	write_buff[7] = 0x00;
	write_buff[8] = 0x00;
	write_buff[9] = 0x00;
	for(int i=10;i<64;i++)
		write_buff[i] = 0xCC;
	return Write(write_buff,64);
}


void CNFC_Device::OperationStatusChange(UINT nOperation, UINT nStatus)
{
	if (!IsWindow(wndNotify->GetSafeHwnd()))return;
	OPERATION_STATUS* s = new OPERATION_STATUS;
	s->nOperation = nOperation;
	s->nStatus = nStatus;
	if (nStatus == 0)strAttention = "";
	if (nOperation != STARTALL_MONITORING)
	{
		if (nStatus)nCurrentOperation = -1;
		else nCurrentOperation = nOperation;
	}
	wndNotify->SendMessage(WM_NFC_OPERATION_DONE,(WPARAM)this,(LPARAM)s);
}

bool CNFC_Device::ParseData(CNFC_Command* c)
{
	switch (c->nCommand)
	{
	case NFCSendMipexBackup:
		if (!ParseMipexCoeff(c->get))return 0;
		break;
	case GetLORAInfo:
		if (!ParseLoraInfo(c->get))return 0;
		break;
	case GetInterfacesByte:
		if (!ParseInterfaces(c->get))return 0;
		break;
	case GetCH4Poly:
		if (!ParseCH4Poly(c->get))return 0;
		break;
	case GetRadioSettingsByte:
		if (interfaces.Standard == RT_LORA || !IsHaveProperty(INTERFACES))
		{
			if (!ParseLoraSettings(c->get))return 0;
		}
		else if (interfaces.Standard == RT_WIFI)
		{
			if (!ParseWifiSettings(c->get))return 0;
		}
		break;
	case SetCommandMIPEXByte:
	case SetCommandUARTByte:
	case SetCommandSPIByte:
		additional_pars.strLastMIPEXAnswer = GetMIPEXAnswer(c->get);
		break;
	case GetLoraOTAA:
	{
		if (!ParseLoraOTAA(c->get))return 0;
		break;
	}
	case GetSettingsByte:
	{
		if (!ParseSettings(c->get))return 0;
		break;
	}
	case GetRegVibroByte:
	{
		if (!ParseVibroPower(c->get))return 0;
		break;
	}
	case GetAlarmSignalingByte:
	{
		if (!ParseAlarms(c->get))return 0;
		break;
	}
	case GetDataSensorsByte:
	{
		if (!ParseSensorsData(c->get))return 0;
		break;
	}
	case GetLastLogAddrByte:
	{
		if (!c->dev->ParseLastLogAddr(c->get))return 0;
		break;
	}
	case GetLogByte:
	{
		if (!ParseLog(c->get, c->nParam))return 0;
		break;
	}
	case GetInfDeviceByte:
	{
		if (!ParseDeviceInfo(c->get))return 0;
		break;
	}
	case GetAllCoefByte:
	{
		if (!ParseSensorsInfo(c->get))return 0;
		break;
	}
	case GetGasRangeByte:
	{
		if (!ParseGasRange(c->get))return 0;
		break;
	}
	case GetSensVRangeByte:
	{
		if (!ParseSensorVRange(c->get))return 0;
		break;
	}
	case GetSensStatusByte:
	{
		if (!ParseSensorStatus(c->get))return 0;
		break;
	}
	case GetLoraKeyByte:
	{
		if (!ParseLoraKey(c->get))return 0;
		break;
	}
	case GetWorkStatByte:
	{
		if (!ParseWorkStat(c->get))return 0;
		break;
	}
	}
	return 1;
}


bool CNFC_Device::SetSensorUnits(bool bGetSettings)
{
	CByteArray* arr = new CByteArray;
	arr->Add(dev_settings.SensorsUnits);
	if (!SendCommand(SetSensUnitsByte, arr))return 0;
	return bGetSettings?GetSettings():1;
}


int CNFC_Device::InitThread()
{
	SetThreadUILanguage(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL));
	SetThreadLocale(MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT));
	return 1;
}

bool CNFC_Device::GetLoraOTAASettings()
{
	lora_settings.bOTAALoaded = 0;
	return SendCommand(GetLoraOTAA);
}

bool CNFC_Device::SetLoraOTAASettings()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(lora_settings.otaa.GetSize());
	memcpy(&(*arr)[0],&lora_settings.otaa, lora_settings.otaa.GetSize());
	if (!SendCommand(SetLoraOTAA, arr))return 0;
	return GetLoraOTAASettings();
}

bool CNFC_Device::ParseLoraOTAA(CByteArray* arr)
{
BYTE* b = &(*arr)[27];
memcpy(&lora_settings.otaa, b, lora_settings.otaa.GetSize());
lora_settings.bOTAALoaded = 1;
return 1;
}


bool CNFC_Device::LoraOTAARejoin()
{
	return SendCommand(OTAARejoin);
}

CString CNFC_Device::GetAlarmString(UINT nGas, SENSORS_DATA& data)
{
	CString str = "Normal";
	GAS_RANGE* r[] = { &sensors_info.alarms.coAlarm,&sensors_info.alarms.h2sAlarm,&sensors_info.alarms.ch4Alarm,&sensors_info.alarms.o2Alarm };
	DWORD d[] = { data.CO,data.H2S,data.CH4LEL,data.O2 };
	for (int i = 0; i < 4; i++)
	{
		if (nGas != (1 << i))continue;
		if (nGas == GAS_O2 && GetSelectedGas(nGas)== O2_O2)
		{
			if (d[i] < r[i]->wFrom)str = "ALARM1";
			else if (d[i] > r[i]->wTo)str = "ALARM2";
		}
		else
		{
			if (d[i] > r[i]->wTo)str = "ALARM2";
			else if (d[i] > r[i]->wFrom)str = "ALARM1";
		}
		break;
	}
	return str;
}

typedef CString _LoadString(UINT nStringId);

CString CNFC_Device::LoadString(UINT nStringId, bool bUnicode)
{
	_LoadString* loadstring = bUnicode ? S_O::LoadStringW : S_O::LoadStringAnsi;
	CString s = loadstring(nStringId);
	if (bUnicode)
	{
		if (nStringId == IDS_MGM3_UNITS || nStringId == IDS_COPPMTOMG ||
			nStringId == IDS_H2SPPMTOMG || nStringId == IDS_O2PPMTOMG ||
			nStringId == IDS_PPMTOMG || nStringId == IDS_MLN_UNITS)
		{
			s.Replace("3", "\xc2\xb3");
			s.Replace("-1", "\xe2\x81\xbb\xc2\xb9");
		}
	}
	return s;
}



void CNFC_Device::GetAvailableUnits(UINT nGas, RM_DATABASEIDLIST& l, bool bUnicode)
{
	l.RemoveAll();
	if (nGas == GAS_MIPEX)
	{
		if (sensors_info.bCH4En == MIPEX_CO2)l.Add(0, LoadString(IDS_VOL_UNITS, bUnicode));
		else
		{
			l.Add(0, LoadString(IDS_VOL_UNITS, bUnicode));
			l.Add(1, LoadString(IDS_LEL_UNITS, bUnicode));
			if (IsExtendedUnits(nGas))
			{
				l.Add(2, LoadString(IDS_MLN_UNITS, bUnicode));
				l.Add(3, LoadString(IDS_MGM3_UNITS, bUnicode));
			}
		}
	}
	else if (nGas == GAS_O2 && !dev_settings.base.O2Chem)l.Add(0, LoadString(IDS_VOL_UNITS, bUnicode));
	else
	{
		l.Add(0, LoadString(IDS_MLN_UNITS, bUnicode));
		l.Add(1, LoadString(IDS_MGM3_UNITS, bUnicode));
		if (IsExtendedUnits(nGas))
		{
			l.Add(2, LoadString(IDS_VOL_UNITS, bUnicode));
			l.Add(3, LoadString(IDS_LEL_UNITS, bUnicode));
		}
	}
}

bool CNFC_Device::IsExtendedUnits(UINT nGas)
{
	if (nGas == GAS_MIPEX)
	{
		return IsHaveProperty(CH4_PDK_UNITS);
	}
	else if (nGas == GAS_O2 && !dev_settings.base.O2Chem)return 0;
	else
	{
		if ((nGas == GAS_O2 && sensors_info.bO2En == O2_H2) || (nGas == GAS_H2S && sensors_info.bH2SEn == H2S_H2)
			|| (nGas == GAS_CO && sensors_info.bCOEn == CO_H2))
		{
			return 1;
		}
	}
	return 0;
}

UINT CNFC_Device::GetExtendedUnitsMask(UINT nGas)
{
	UINT nMask = 0;
	if (IsExtendedUnits(nGas))
	{
		if (nGas == GAS_O2)nMask = O2_UNITS_H2;
		else if (nGas == GAS_CO)nMask = CO_UNITS_H2;
		else if (nGas == GAS_H2S)nMask = H2S_UNITS_H2;
		else if (nGas == GAS_MIPEX)nMask = CH4_UNITS_PDK;
	}
	return nMask;
}

UINT CNFC_Device::GetUnits(UINT nGas)
{
	//	UINT u[] = { sensors_info.alarms.nUnitsCO,sensors_info.alarms.nUnitsH2S,sensors_info.alarms.nUnitsCh4,sensors_info.alarms.nUnitsO2 };
	UINT nUnits = (dev_settings.SensorsUnits & nGas) != 0;
	if (nGas == GAS_MIPEX)
	{
		if (sensors_info.bCH4En == MIPEX_CO2)return 0;
	}
	if (IsExtendedUnits(nGas))
	{
		nUnits |= (((dev_settings.SensorsUnits & GetExtendedUnitsMask(nGas)) != 0) << 1);
	}
	return nUnits;
}


CString CNFC_Device::GetUnitsString(UINT nGas, bool bUnicode)
{
	RM_DATABASEIDLIST l;
	GetAvailableUnits(nGas, l, bUnicode);
	UINT nUnits = GetUnits(nGas);
	CString str;
	if(nUnits !=-1)str = l.Get(nUnits);
	return str;
}

time_t CNFC_Device::SystemTimeToTime(SYSTEMTIME& st)
{
	std::tm tm;
	tm.tm_sec = st.wSecond;
	tm.tm_min = st.wMinute;
	tm.tm_hour = st.wHour;
	tm.tm_mday = st.wDay;
	tm.tm_mon = st.wMonth - 1;
	tm.tm_year = st.wYear - 1900;
	tm.tm_isdst = -1;
	std::time_t t = _mkgmtime(&tm);
	return t;
}

void CNFC_Device::TimeToSystemTIME(UINT nTime, SYSTEMTIME& st)
{
	if (nTime == 0 || nTime==-1)
	{
		GetLocalTime(&st);
		return;
	}
	FILETIME ft;
	LONGLONG ll;
	ll = Int32x32To64(nTime, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = ll >> 32;
	FileTimeToSystemTime(&ft, &st);
}

bool CNFC_Device::IsMIPEX_FAnswerCorrect(CString str)
{
	int nL = str.GetLength();
	if (nL < 71)return 0;
	if (str.GetAt(0) != 0xe)return 0;
	BYTE nSum = str.GetAt(1),nComp=str.GetAt(70);
	for (int i = 2; i < 70; i++)
	{
		nSum ^= str.GetAt(i);
	}
	if (nSum != nComp)return 0;
	return 1;
}

bool CNFC_Device::IsBigMIPEXReply(CString strCommand)
{
	if (S_O::FindNoCase(strCommand, "const") != -1)return 1;
	else if (S_O::FindNoCase(strCommand, "tab") != -1)return 1;
	else if (S_O::CompareNoCase(strCommand, "fuls") == 0)return 1;
	else if (S_O::CompareNoCase(strCommand, "fulu") == 0)return 1;
	return 0;
}

int CNFC_Device::GetMIPEXCommandBlocksNum()
{
	int nBlocks = 2;
	if (S_O::FindNoCase(additional_pars.strMIPEXCommand, "tab") != -1)nBlocks = 14;
	else if(IsBigMIPEXReply(additional_pars.strMIPEXCommand))nBlocks = 6;
	return nBlocks;
}

UINT CNFC_Device::GetMIPEXCommandTimeout()
{
	UINT nTimeout = 2000;
	if (IsBigMIPEXReply(additional_pars.strMIPEXCommand))nTimeout = 60000;
	return nTimeout;
}

bool CNFC_Device::IsHasLora()
{
	if (!lora_settings.bLoaded)return 0;
	if (lora_settings.otaa.Options & OTAA_ENABLED)return 1;
	if (lora_settings.nAddress==0)return 0;
	return 1;
}


bool CNFC_Device::SetSensorUnitsAll()
{
	BreakExecution();
	WaitComplete(1);
	evService.SetEvent();
	OperationStatusChange(STARTALL_UNITS, 0);
	hCommandThread = CreateThread(NULL, 0, SetSensorUnitsThread, (void*)this, 0, 0);
	return true;
}

DWORD WINAPI CNFC_Device::SetSensorUnitsThread(void* ptr)
{
	InitThread();
	CNFC_Device* dev = (CNFC_Device*)ptr;
	int nRet = 1;
	try
	{
		if (!dev->TestIfDeviceChanged())throw 0;
		if (!dev->dev_settings.bLoaded)
		{
			dev->GetSettings();
			if (!dev->WaitComplete())throw 0;
		}
		BYTE b = dev->dev_settings.SensorsUnits;
		dev->SetSensorUnits(0);
		dev->dev_settings.SensorsUnits = 0xff;
		dev->GetSettings();
		if (!dev->WaitComplete())throw 0;
		if (dev->dev_settings.SensorsUnits != b)throw 0;
	}
	catch (int)
	{
		nRet = 0;
	}
	dev->dev_settings.bSensorUnitsResult = nRet;
	dev->OperationStatusChange(STARTALL_UNITS, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return nRet;
}

void CNFC_Device::GetAvailableGas(UINT nGas, RM_DATABASEIDLIST& l, bool bShort, bool bUnicode)
{
	l.RemoveAll();
	RM_STRINGWITHID sid;
	CString strFormat = "%s (%s)";
	if (nGas == GAS_MIPEX)
	{
		CString g[] = { bShort ? "CH4" : S_O::Format(strFormat,"CH4",S_O::LoadString(IDS_CH4)),
		bShort ? "C3H8" : S_O::Format(strFormat,"C3H8",LoadString(IDS_C3H8,bUnicode)),
		bShort ? "CO2" : S_O::Format(strFormat,"CO2",LoadString(IDS_CO2,bUnicode)),
		bShort ? "C2H6" : S_O::Format(strFormat,"C2H6",LoadString(IDS_C2H6,bUnicode)),
		bShort ? "C4H10" : S_O::Format(strFormat,"C4H10",LoadString(IDS_C4H10,bUnicode)),
		bShort ? "C5H12" : S_O::Format(strFormat,"C5H12",LoadString(IDS_C5H12,bUnicode)),
		bShort ? "C3H6" : S_O::Format(strFormat,"C3H6",LoadString(IDS_C3H6,bUnicode)),
		bShort ? "CH3OH" : S_O::Format(strFormat,"CH3OH",LoadString(IDS_CH3OH,bUnicode)),
		bShort ? "C6H6" : S_O::Format(strFormat,"C6H6",LoadString(IDS_C6H6,bUnicode)),
		bShort ? "C7H16" : S_O::Format(strFormat,"C7H16",LoadString(IDS_C7H16,bUnicode)),
		LoadString(bShort ? IDS_DIESEL_SHORT: IDS_DIESEL,bUnicode),
		LoadString(bShort ? IDS_KEROSENE_SHORT : IDS_KEROSENE,bUnicode),
		LoadString(bShort ? IDS_AV_GASOLINE_SHORT : IDS_AV_GASOLINE,bUnicode),
		LoadString(bShort ? IDS_UNLEADED_GASOLINE_SHORT : IDS_UNLEADED_GASOLINE,bUnicode),
		LoadString(bShort ? IDS_GASOLINE_VAPOUR_SHORT : IDS_GASOLINE_VAPOUR,bUnicode),
		LoadString(bShort ? IDS_HYDROCARBON_MIX_SHORT : IDS_HYDROCARBON_MIX,bUnicode),
		bShort? "CH3COCH3": S_O::Format(strFormat, "CH3COCH3", LoadString(IDS_ACETONE,bUnicode)) };

		for (int i = 0; i < sizeof(g) / sizeof(char*); i++)
		{
			sid.nId = i + 1;
			sid.str = g[i];
			l.list.Add(sid);
		}
	}
	else if (nGas == GAS_O2)
	{
		CString g[] = { bShort ? "O2" : S_O::Format(strFormat,"O2",LoadString(IDS_O2,bUnicode)),
			bShort ? "CO" : S_O::Format(strFormat,"CO",LoadString(IDS_CO,bUnicode)),
			bShort ? "H2S" : S_O::Format(strFormat,"H2S",LoadString(IDS_H2S,bUnicode)) ,
			bShort ? "Cl2" : S_O::Format(strFormat,"Cl2",LoadString(IDS_CL2,bUnicode)),
			bShort ? "NH3" : S_O::Format(strFormat,"NH3",LoadString(IDS_NH3,bUnicode)),
			bShort ? "SO2" : S_O::Format(strFormat,"SO2",LoadString(IDS_SO2,bUnicode)),
			bShort ? "NO2" : S_O::Format(strFormat,"NO2",LoadString(IDS_NO2,bUnicode)),
			bShort ? "H2" : S_O::Format(strFormat,"H2",LoadString(IDS_H2,bUnicode)),
			bShort ? "CH3SH" : S_O::Format(strFormat,"CH3SH",LoadString(IDS_CH3SH,bUnicode)),
			bShort ? "CH3OH" : S_O::Format(strFormat,"CH3OH",LoadString(IDS_CH3OH,bUnicode)),
			bShort ? "C2H5SH" : S_O::Format(strFormat,"C2H5SH",LoadString(IDS_C2H5SH,bUnicode)),
			bShort ? "C2H4O" : S_O::Format(strFormat,"C2H4O",LoadString(IDS_C2H4O,bUnicode)),
			bShort ? "NO" : S_O::Format(strFormat,"NO",LoadString(IDS_GAS_NO,bUnicode)) };
		for (int i = 0; i < sizeof(g) / sizeof(char*); i++)
		{
			sid.nId = i + 1;
			sid.str = g[i];
			l.list.Add(sid);
		}
	}
	else if (nGas == GAS_CO || nGas == GAS_H2S)
	{
		UINT nId[] = { 1,2,3,0x12,0x22,0x32,0x42,0x52,0x62,0x72,0x82,0x92,0xa2,0xb2 ,0xc2,0xd2,0xe2,0xf2,0x11};
		CString g[] = { "CO-DT" ,
			bShort ? "CO" : S_O::Format(strFormat,"CO",LoadString(IDS_CO,bUnicode)),
			S_O::Format("%s %s", GetGasString(GAS_MIPEX,1,bUnicode), LoadString(IDS_MPC,bUnicode)),
			bShort ? "O2" : S_O::Format(strFormat,"O2",LoadString(IDS_O2,bUnicode)),
			bShort ? "CO" : S_O::Format(strFormat,"CO",LoadString(IDS_CO,bUnicode)),
			bShort ? "H2S" : S_O::Format(strFormat,"H2S",LoadString(IDS_H2S,bUnicode)) ,
			bShort ? "Cl2" : S_O::Format(strFormat,"Cl2",LoadString(IDS_CL2,bUnicode)),
			bShort ? "NH3" : S_O::Format(strFormat,"NH3",LoadString(IDS_NH3,bUnicode)),
			bShort ? "SO2" : S_O::Format(strFormat,"SO2",LoadString(IDS_SO2,bUnicode)),
			bShort ? "NO2" : S_O::Format(strFormat,"NO2",LoadString(IDS_NO2,bUnicode)),
			bShort ? "H2" : S_O::Format(strFormat,"H2",LoadString(IDS_H2,bUnicode)),
			bShort ? "CH3SH" : S_O::Format(strFormat,"CH3SH",LoadString(IDS_CH3SH,bUnicode)),
			bShort ? "CH3OH" : S_O::Format(strFormat,"CH3OH",LoadString(IDS_CH3OH,bUnicode)),
			"CO (DT)",//for H2S chan
			"H2S (DT)",
			bShort?"C2H5SH": S_O::Format(strFormat,"C2H5SH",LoadString(IDS_C2H5SH,bUnicode)),
			bShort ? "C2H4O" : S_O::Format(strFormat,"C2H4O",LoadString(IDS_C2H4O,bUnicode)),
			bShort ? "NO" : S_O::Format(strFormat,"NO",LoadString(IDS_GAS_NO,bUnicode)),
			"SO2+DT",};
//		if (nGas == GAS_H2S)g[0] = bShort ? "H2S" : S_O::Format(strFormat, "H2S", LoadString(IDS_H2S));
		int nC = sizeof(nId) / sizeof(UINT);
		if (!IsHaveProperty(EXTENDEDCO))nC = (nGas == GAS_CO) ? 3 : 1;
		for (int i = 0; i < nC; i++)
		{
			if (nGas == GAS_H2S && (i==0 || i == 2))continue;
			if (i == 3)continue;
			if (nGas == GAS_CO && (i==13 || i==14 || i==17))continue;
			sid.nId = nId[i];
			sid.str = g[i];
			l.list.Add(sid);
		}
	}
	/*else if (nGas == GAS_H2S)
	{
		sid.nId = 1;
		sid.str = "H2S";
		l.list.Add(sid);
	}*/
}

CString CNFC_Device::GetGasStringByVal(UINT nGas, UINT nVal, bool bShort, bool bUnicode)
{
	CString str;
	RM_DATABASEIDLIST l;
	GetAvailableGas(nGas, l, bShort, bUnicode);
	if (nGas == GAS_H2S)
	{
		l.Add(1, bShort ? "H2S" : S_O::Format("%s (%s)", "H2S", LoadString(IDS_H2S, bUnicode)));
	}
	if (l.IsIdInList(nVal))str = l.Get(nVal);
	else str = l.Get(1);
	return str;
}

CString CNFC_Device::GetGasString(UINT nGas, bool bShort, bool bUnicode)
{
	CString str;
	BYTE en[] = { sensors_info.bCOEn,sensors_info.bH2SEn,sensors_info.bCH4En,sensors_info.bO2En};
	for (int i = 0; i < 4; i++)
	{
		if (nGas == (1 << i))
		{
			str = GetGasStringByVal(nGas, en[i], bShort, bUnicode);
			break;
		}
	}
	return str;
}

BYTE CNFC_Device::GetSelectedGas(UINT nGas)
{
	BYTE en[] = { sensors_info.bCOEn,sensors_info.bH2SEn,sensors_info.bCH4En,sensors_info.bO2En };
	for (int i = 0; i < 4; i++)
	{
		if (nGas == (1 << i))
		{
			return en[i];
		}
	}
	return 0;
}


bool CNFC_Device::TestLimit(UINT nGas, bool bLimit2)
{
	BreakExecution();
	WaitComplete(1);
	limitTest.nGas = nGas;
	limitTest.bTest2 = bLimit2;
	evService.SetEvent();
	OperationStatusChange(STARTALL_LIMITTEST, 0);
	hCommandThread = CreateThread(NULL, 0, TestLimitThread, (void*)this, 0, 0);
	return true;
}

DWORD WINAPI CNFC_Device::TestLimitThread(void* ptr)
{
	InitThread();
	CNFC_Device* dev = (CNFC_Device*)ptr;
	GAS_LIMIT_TEST t = dev->limitTest;
	CString strDescr;
	strDescr.Format(" %u %s", t.bTest2 + 1, dev->GetGasString(t.nGas));
	dev->WriteLog(S_O::LoadString(IDS_STARTLIMITTEST)+ strDescr,LOG_PROCESSCOMPLETE);
	int nRet = 1;
	SENSORS_INFO save;
	try
	{
		CRealtimeObject time;
		SENSORS_INFO* s = &dev->sensors_info;
		float* coeff[] = { s->fCOCoeff,s->fH2SCoeff,0,s->fO2Coeff }, * c = 0;
		GAS_RANGE* range[] = { &s->alarms.coAlarm,&s->alarms.h2sAlarm,0,&s->alarms.o2Alarm }, * r = 0;
		for (int i = 0; i < 4; i++)
		{
			if (t.nGas == (1 << i))
			{
				c = coeff[i];
				r = range[i];
				break;
			}
		}
		if (c == 0)throw 0;
		if (!dev->TestIfDeviceChanged())throw 0;
		if (!dev->sensors_info.bLoaded)
		{
			dev->GetAllCoeffs();
			if (!dev->WaitComplete())throw 0;
		}
		if (!dev->sensors_info.alarms.bLoaded)
		{
			dev->GetAlarms();
			if (!dev->WaitComplete())throw 0;
		}
		if (!dev->dev_settings.bLoaded)
		{
			dev->GetSettings();
			if (!dev->WaitComplete())throw 0;
		}
		save = *s;
		double dLimit = t.bTest2 ? r->wTo : r->wFrom;
		dLimit /= dev->GetGasDevider(t.nGas,1);
		if (t.nGas == GAS_O2 && !dev->dev_settings.base.O2Chem && !t.bTest2)c[2] = (float)(dLimit - 1);
		else c[2] = (float)(dLimit + 1);
		for (int i = 0; i < 2; i++)
		{
			if (t.nGas == GAS_CO)dev->SetCOCoeff();
			else if (t.nGas == GAS_H2S)dev->SetH2SCoeff();
			else dev->SetO2Coeff();
			if (!dev->WaitComplete())throw 0;
			if (!i)
			{
				dev->WriteLog(S_O::LoadString(IDS_WAIT));
				time.WaitTime(10, 0, &dev->evBreak, dev->progressCtrl);
				*s = save;
			}
		}
	}
	catch (int)
	{
		nRet = 0;
	}
	dev->limitTest.nResult = nRet;
	dev->WriteLog(S_O::LoadString(nRet?IDS_LIMITTESTDONE: IDS_LIMITTESTERROR),nRet?LOG_PROCESSCOMPLETE:LOG_ERROR);
	dev->OperationStatusChange(STARTALL_LIMITTEST, 1);
	WaitForSingleObject(dev->hMutex, INFINITE);
	dev->evService.ResetEvent();
	dev->evBreak.ResetEvent();
	ReleaseMutex(dev->hMutex);
	return nRet;
}

bool CNFC_Device::TestIfDeviceChanged()
{
	UINT nSerial = dev_info.nSerialNo;
	GetDeviceInfo();
	if (!WaitComplete(0,0))return 0;
	if (nSerial != dev_info.nSerialNo)
	{
		ClearLoadState();
	}
	arrExchanges.RemoveAll();
	dev_info.dExchangeQuality = -1;
	return 1;
}

void CNFC_Device::ClearLoadState()
{
	dev_settings.bLoaded = 0;
	sensors_info.bLoaded = sensors_info.alarms.bLoaded = sensors_info.bGasRangeLoaded= 0;
	sensors_info.bStatusLoaded = 0;
	lora_settings.bLoaded = 0;
	lora_settings.bOTAALoaded = 0;
	work_stat.bLoaded = 0;
	dev_info.bRegVibroLoaded = 0;
}


bool CNFC_Device::UpdateSensorsData(bool bSendF, CString strFCommand)
{
	if (!GetSensorData())return 0;
	if (bSendF)
	{
		if (strFCommand == "")strFCommand = "F";
		additional_pars.strMIPEXCommand = strFCommand;
		if (!SendCommandMIPEX())return 0;
	}
	return 1;
}


bool CNFC_Device::StartMonitoring(bool bSendF, CString strFCommand)
{
	bInMonitoring = 1;
	bPauseMonitoring = 0;
	monitoring.RemoveAll();
	dcarMIPEX.arr.RemoveAll();
	additional_pars.bMonitoringSendF = bSendF;
	additional_pars.strMIPEXCommand = strFCommand;
	CreateThread(NULL, 0, StartMonitoringThread, (void*)this, 0, 0);
	return 1;
}

DWORD WINAPI CNFC_Device::StartMonitoringThread(void* ptr)
{
	InitThread();
	CNFC_Device* dev = (CNFC_Device*)ptr;
	try
	{
		if (!dev->TestIfDeviceChanged())throw 0;
		if (!dev->sensors_info.bStatusLoaded)dev->GetSensorStatus();
		if (!dev->sensors_info.alarms.bLoaded)dev->GetAlarms();
		if (!dev->dev_settings.bLoaded)dev->GetSettings();
		dev->UpdateSensorsData(dev->additional_pars.bMonitoringSendF, dev->additional_pars.strMIPEXCommand);
	}
	catch (int)
	{
		dev->bInMonitoring = 0;
		dev->OperationStatusChange(STARTALL_MONITORING, 1);
	}
	return 1;
}

bool CNFC_Device::IsHaveProperty(device_property nProperty, UINT nSWVer)
{
	switch (nProperty)
	{
	case CH4_COEFF:
		return nSWVer >= 70;
	case PRECISION:
		return nSWVer >= 95;
	case VBATINLOG:
		return nSWVer >= 98;
	case LORAOTAA:
		return nSWVer >= 90;
	case SKIPSELFTEST:
	case AUTOZERO:
	case ALTSCREENTIME:
	case CRC:
	case LORACONFIRM:
	case LORALOW:
		return nSWVer >= 99;
	case GASSIM:
		return  nSWVer >= 100;
	case O2CHEMPOS:
		return nSWVer >= 101;
	case SCALEPOINT:
		return  nSWVer >= 103;
	case RAK811:
		return nSWVer >= 104;
	case LORAP2P:
		return  nSWVer >= 105;
	case CHPRESSURE:
		return  nSWVer >= 113;
	case EXTENDEDCO:
		return  nSWVer >= 115;
	case WEEKTOSCALE:
		return  nSWVer >= 116;
	case GASPOS:
		return  nSWVer >= 119;
	case CH4_MULT:
	case BITLEN16:
		return	nSWVer >= 124;
	case SHORTLOG:
		return	nSWVer >= 125;
	case CH4_MULT_NEW:
		return nSWVer >= 126;
	case INTERFACES:
		return nSWVer >= 131;
	case RSSI_PROPERTY:
		return nSWVer >= 133;
	case CH4_PDK_UNITS:
	case MIPEXCOEFF:
		return nSWVer >= 135;
	case MIPEX_16BIT:
		return nSWVer >= 136;
	case MIPEX_LASTREAD:
		return nSWVer >= 139;
	case TRANSPORTALARM:
		return nSWVer >= 152;
	case CH4_BUFFER:
		return nSWVer >= 153;
	case SKIPSELFTTESTTRANSPORT:
		return nSWVer >= 155;
	case FREEZE_MASK:
		return nSWVer >= 157;
	case FREEZE_IN_LOG:
		return nSWVer >= 165;
	case ACCUMULATOR:
		return nSWVer >= 168;
	}
	return 0;
}

bool CNFC_Device::IsHaveProperty(device_property nProperty)
{
	if (IsVirtualDevice())return 1;
	if (!IsHaveProperty(nProperty, dev_info.nSWVer))return 0;
	if (nProperty == RAK811 && !(dev_info.btEnabledHW & HW_RADIO_RAK))return 0;
	return 1;
}

bool DEVICE_SAVED_SETTINGS::Save(CFile* f)
{
	WriteVersion(f, 3);
	S_O::SaveString(f,strDevNoFrom);
	S_O::SaveString(f, strDevVersion);
	int nSz = sizeof(base);
	f->Write(&nSz, sizeof(int));
	f->Write(&base, nSz);

	nSz = sizeof(dev_settings);
	f->Write(&nSz, sizeof(int));
	f->Write(&dev_settings, nSz);

	nSz = sizeof(sensors);
	f->Write(&nSz, sizeof(int));
	f->Write(&sensors, nSz);

	nSz = sizeof(lora);
	f->Write(&nSz, sizeof(int));
	f->Write(&lora, nSz);
	f->Write(&bUserOnly, 1);
	nSz = mipex.GetSize();
	f->Write(&nSz, sizeof(int));
	f->Write(&mipex, nSz);
	nSz = sizeof(INTERFACE_SETTINGS);
	f->Write(&nSz, sizeof(int));
	f->Write(&interfaces, nSz);
	nSz = sizeof(WIFI_SETTINGS);
	f->Write(&nSz, sizeof(int));
	f->Write(&wifi, nSz);
	return 1;
}

bool DEVICE_SAVED_SETTINGS::Load(CFile* f)
{
	int nVer = 3;
	if (!ReadVersion(f, nVer))return 0;
	if (!S_O::LoadString(f, strDevNoFrom))return 0;
	if (!S_O::LoadString(f, strDevVersion))return 0;
	int nSz = 0;
	if (!f->Read(&nSz, sizeof(int)))return 0;
	if (nSz != sizeof(base))return 0;
	if (!f->Read(&base, nSz))return 0;

	if (!f->Read(&nSz, sizeof(int)))return 0;
	if (nSz != sizeof(dev_settings))return 0;
	if (!f->Read(&dev_settings, nSz))return 0;

	if (!f->Read(&nSz, sizeof(int)))return 0;
	if (nSz != sizeof(sensors))return 0;
	if (!f->Read(&sensors, nSz))return 0;

	if (!f->Read(&nSz, sizeof(int)))return 0;
	if (nSz != sizeof(lora))return 0;
	if (!f->Read(&lora, nSz))return 0;
	if (nVer > 0)
	{
		if (!f->Read(&bUserOnly, 1))return 0;
	}
	else bUserOnly = 0;
	if (nVer > 1)
	{
		if (!f->Read(&nSz, sizeof(int)))return 0;
		if (nSz != mipex.GetSize())return 0;
		if (!f->Read(&mipex, nSz))return 0;
	}
	else mipex.Backup = 0;
	if (nVer > 2)
	{
		if (!f->Read(&nSz, sizeof(int)))return 0;
		if (nSz != sizeof(INTERFACE_SETTINGS))return 0;
		if (!f->Read(&interfaces, nSz))return 0;
		if (!f->Read(&nSz, sizeof(int)))return 0;
		if (nSz != sizeof(WIFI_SETTINGS))return 0;
		if (!f->Read(&wifi, nSz))return 0;
	}
	else
	{
		wifi.bLoaded = 0;
		interfaces.bLoaded = 0;
	}
	bLoaded = 1;
	nSWVer = (UINT)(S_O::ToDouble(strDevVersion) * 100 - 100);
	return 1;
}

CString DEVICE_SAVED_SETTINGS::GetDescr()
{
	CString str;
	str.Format("From #%s v.%s (%s)", strDevNoFrom, strDevVersion, tmCreate.FormatStandard(1, 1));
	return str;
}

bool CNFC_Device::GetLoraInfo()
{
	return SendCommand(GetLORAInfo);
}

bool CNFC_Device::ParseLoraInfo(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	memcpy(&loraInfo, b, sizeof(LORA_Info));
	return 1;
}

float CNFC_Device::GetGasDevider(UINT nGas, bool bForScalePoints)
{
	if (nGas == GAS_MIPEX)
	{
		if (bForScalePoints)
		{
			if (GetUnits(nGas) > 1)return 1.0;
			return 100.0;
		}
		return IsPDK()?1000.0:100.0;
	}
	if (nGas == GAS_CO)
	{
		if (bForScalePoints && GetSelectedGas(GAS_CO) == CO_MPC)return 1.0;
	}
	if (nGas == GAS_H2S)
	{
		if (bForScalePoints && GetSelectedGas(GAS_H2S) == H2S_MPC)return 1.0;
	}
	return 10.0;
}

bool CNFC_Device::IsGasSelected(UINT nGas)
{
	BYTE en[] = { sensors_info.bCOEn,sensors_info.bH2SEn,sensors_info.bCH4En,sensors_info.bO2En };
	for (int i = 0; i < 4; i++)
	{
		if(nGas==(1<<i))return en[i]!=0;
	}
	return 0;
}

void CNFC_Device::ExchangeSuccess(bool bSuccess)
{
	if (arrExchanges.GetSize() >= 20)arrExchanges.RemoveAt(0);
	arrExchanges.Add(bSuccess);
	int n = 0;
	for (int i = 0; i < arrExchanges.GetSize();i++)n += arrExchanges[i];
	dev_info.dExchangeQuality = (double)n / arrExchanges.GetSize();
}

bool CNFC_Device::GetInterfaces()
{
	interfaces.bLoaded = 0;
	return SendCommand(GetInterfacesByte);
}

bool CNFC_Device::SetInterfaces()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(interfaces.GetSize());
	memcpy(&(*arr)[0], &interfacesToSet, interfaces.GetSize());
	if (!SendCommand(SetInterfacesByte, arr))return 0;
	return GetInterfaces();
}

bool CNFC_Device::GetWifiSettings()
{
	wifi.bLoaded = 0;
	return SendCommand(GetRadioSettingsByte);
}

UINT WIFI_SETTINGS::GetSize(UINT nSWVer)
{
	UINT nSize=sizeof(WIFI_SETTINGS) - 1;
	if (!CNFC_Device::IsHaveProperty(RSSI_PROPERTY,nSWVer))nSize -= sizeof(RSSI);
	return nSize;
}

bool CNFC_Device::SetWifiSettings()
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(wifi.GetSize(dev_info.nSWVer));
	memcpy(&(*arr)[0], &wifi, wifi.GetSize(dev_info.nSWVer));
	if (!SendCommand(SetWifiByte, arr))return 0;
	return GetWifiSettings();
}

bool CNFC_Device::ParseInterfaces(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	memcpy(&interfaces, b, interfaces.GetSize());
	interfaces.bLoaded = 1;
	interfacesToSet = interfaces;
	return 1;
}

bool CNFC_Device::ParseWifiSettings(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	memcpy(&wifi, b, wifi.GetSize(dev_info.nSWVer));
	wifi.bLoaded = 1;
	return 1;
}

bool CNFC_Device::ParseCH4Poly(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	memcpy(sensors_info.fCoefCH4_Mult, b, 6 * sizeof(float));
	memcpy(sensors_info.fCoefCH4_Mult2, &b[24], 6 * sizeof(float));
	memcpy(&sensors_info.nSwitchConc, &b[48], 4);
	return 1;
}

void CNFC_Device::CopyUnits(CNFC_Device* from)
{
	BYTE en[] = { from->sensors_info.bCOEn,from->sensors_info.bH2SEn,from->sensors_info.bCH4En,from->sensors_info.bO2En };
	for (int i = 0; i < 4; i++)
	{
		if (!en[i])continue;
		UINT nGas = 1 << i;
		if (from->dev_settings.SensorsUnits & nGas)dev_settings.SensorsUnits |= nGas;
		else dev_settings.SensorsUnits &= ~nGas;
		if (IsExtendedUnits(nGas) && from->IsExtendedUnits(nGas))
		{
			UINT nMask = from->GetExtendedUnitsMask(nGas);
			if (from->dev_settings.SensorsUnits & nMask)dev_settings.SensorsUnits |= nMask;
			else dev_settings.SensorsUnits &= ~nMask;
		}
	}
}

int CNFC_Device::GetPrecision(UINT nGas)
{
	if (nGas == GAS_MIPEX)
	{
		UINT nUnits = GetUnits(nGas);
		if (nUnits > 1)return 0;
		return 3;
	}
	return 2;
}

bool CNFC_Device::MipexBackup()
{
	if (!SendCommand(NFCMipexBackup))return 0;
	return MipexGetBackup();
}

bool CNFC_Device::MipexRestore()
{
	return SendCommand(NFCMipexRestore);
}

bool CNFC_Device::MipexGetBackup()
{
	return SendCommand(NFCSendMipexBackup);
}

bool CNFC_Device::MipexSetBackup(MIPEX_COEFF* coeff)
{
	CByteArray* arr = new CByteArray;
	arr->SetSize(coeff->GetSize());
	memcpy(&(*arr)[0], coeff, coeff->GetSize());
	if (!SendCommand(NFCSaveMipexBackup, arr))return 0;
	return MipexGetBackup();
}

bool CNFC_Device::ParseMipexCoeff(CByteArray* arr)
{
	BYTE* b = &(*arr)[27];
	memcpy(&mipex, b, mipex.GetSize());
	mipex.bLoaded = 1;
	return 1;
}

bool CNFC_Device::IsPDK()
{
	return GetSelectedGas(GAS_H2S) == H2S_MPC || GetSelectedGas(GAS_CO) == CO_MPC;
}

double CNFC_Device::RecalcForCalibration(UINT nGas, double d)
{
	UINT nUnits = GetUnits(nGas);
	bool bMul = 1;
	if (nGas == GAS_O2)
	{
		if (nUnits == 1 || nUnits == 3)d = d * 100 / dev_settings.base.CoefCHEMppmToMg;
		//if (nUnits > 2)d = d * 10000;
	}
	else if (nGas == GAS_H2S)
	{
		if(nUnits == 1 || nUnits == 3)d = d * 100 / dev_settings.base.CoefH2SppmToMg;
		//if (nUnits > 2)d = d * 10000;
		if (GetSelectedGas(GAS_H2S) == H2S_MPC)
		{
			if (nUnits == 1)d *= 100;
			d /= 10;
			bMul = 0;
		}
	}
	else if (nGas == GAS_CO)
	{
		if(nUnits == 1 || nUnits == 3)d = d * 100 / dev_settings.base.CoefCOppmToMg;
		//if (nUnits > 2)d = d * 10000;
		if (GetSelectedGas(GAS_CO) == CO_MPC)
		{
			if (nUnits == 1)d *= 100;
			d /= 10;
			bMul = 0;
		}
	}
	else if (nGas == GAS_MIPEX)
	{
		if (nUnits == 0)
		{
			d *= GetGasDevider(nGas,0);
		}
		else if (nUnits == 1)d = d * dev_settings.base.CoefVolToLEL / 100 ;
		else if (nUnits == 2 || nUnits == 3)
		{
			if (nUnits == 3)d = d * 10000 / dev_settings.base.CoefVolToLEL;
			double dDev = 100;
			if (IsPDK())dDev = 10;
			d /= dDev;
		}
		bMul = 0;
	}
	//if (bMul && nGas == GAS_H2S && GetSelectedGas(GAS_CO) == CO_MPC)d *= 1000;
	//else
		if (bMul)d *= GetGasDevider(nGas,0);
	return d;
}

bool CNFC_Device::UpdateMIPEXCommand(CString& m_Command)
{
	S_O::Trim(m_Command);
	if (m_Command == "")return 0;
	CString str, buf;
	int nf = m_Command.Find("\\"), ns = 0, nl = m_Command.GetLength();
	while (nf != -1)
	{
		if (nf == nl - 1)
		{
			ns = nf;
			break;
		}
		if (nf != ns)str += m_Command.Mid(ns, nf - ns);
		char c = m_Command.GetAt(nf + 1);
		if (c == 'x')
		{
			if (nl - nf < 4)
			{
				ns = nf;
				break;
			}
			buf = m_Command.Mid(nf + 2, 2);
			BYTE b = (BYTE)strtol(buf, 0, 16);
			char* bb = buf.GetBufferSetLength(1);
			bb[0] = b;
			buf.ReleaseBuffer(1);
			str += buf;
			ns = nf + 4;
		}
		else if (c == '\\')
		{
			str += "\\";
			ns = nf + 2;
		}
		else
		{
			str += "\\";
			ns = nf + 1;
		}
		nf = m_Command.Find("\\", ns);
	}
	if (ns < nl)str += m_Command.Mid(ns, nl - ns);
	additional_pars.strMIPEXCommand = str;
	return 1;
}

void DELAYED_COMMAND_ARRAY::RecalcTime()
{
	double d = 0;
	for (int i = 0; i < arr.GetSize(); i++)
	{
		d += arr[i].dTimeDelay;
		arr[i].dTimeDelayFromStart = d;
	}
}

CString DELAYED_COMMAND_ARRAY::GetProfileFolder()
{
	return S_O::Format("%s\\delayed", g_szCurrentDir);
}

bool DELAYED_COMMAND_ARRAY::Save()
{
	CString str;
	CreateDirectory(GetProfileFolder(), NULL);
	if (strName == "")return 0;
	str.Format("%s\\%s.dca", GetProfileFolder(), strName);
	CFile f;
	if (!f.Open(str, CFile::modeCreate | CFile::modeReadWrite))return 0;
	WriteVersion(&f, 2);
	f.Write(&nMaxId, sizeof(int));
	int n = arr.GetSize();
	f.Write(&n, sizeof(int));
	for (int i = 0; i < n; i++)
	{
		DELAYED_COMMAND& c = arr[i];
		f.Write(&c.nCommandId, sizeof(int));
		S_O::SaveString(&f, c.strCommand);
		f.Write(&c.dTimeDelay, sizeof(double));
		f.Write(&c.bT, 1);
		f.Write(&c.dT, sizeof(double));
		f.Write(&c.dTTime, sizeof(double));
		f.Write(&c.bGas, 1);
		f.Write(&c.dGasTime, sizeof(double));
		f.Write(&c.dGasConc, sizeof(double));
		f.Write(&c.nValve, sizeof(UINT));
	}
	return 1;
}

bool DELAYED_COMMAND_ARRAY::Load()
{
	arr.RemoveAll();
	CString str;
	if (strName == "")strName = "base";
	str.Format("%s\\%s.dca", GetProfileFolder(), strName);
	CFile f;
	if (!f.Open(str, CFile::modeRead))return 0;
	int nVer = 2;
	if (!ReadVersion(&f, nVer))return 0;
	if (!f.Read(&nMaxId, sizeof(int)))return 0;
	int n = 0;
	if (!f.Read(&n, sizeof(int)))return 0;
	arr.SetSize(n);
	for (int i = 0; i < n; i++)
	{
		DELAYED_COMMAND& c = arr[i];
		if (!f.Read(&c.nCommandId, sizeof(int)))return 0;
		if (!S_O::LoadString(&f, c.strCommand))return 0;
		if (!f.Read(&c.dTimeDelay, sizeof(double)))return 0;
		if (nVer > 0)
		{
			if (!f.Read(&c.bT, 1))return 0;
			if (!f.Read(&c.dT, sizeof(double)))return 0;
			if (!f.Read(&c.dTTime, sizeof(double)))return 0;
		}
		if (nVer > 1)
		{
			if (!f.Read(&c.bGas, 1))return 0;
			if (!f.Read(&c.dGasTime, sizeof(double)))return 0;
			if (!f.Read(&c.dGasConc, sizeof(double)))return 0;
			if (!f.Read(&c.nValve, sizeof(UINT)))return 0;
		}
		c.nCommandId = i + 1;
	}
	nMaxId = arr.GetSize() + 1;
	RecalcTime();
	return 1;
}

bool CNFC_Device::MipexGetLastCommand()
{
	return SendCommand(GetLastMIPEXCommand);
}

bool CNFC_Device::ParseLastMIPEXCommand(CByteArray* arr, DELAYED_COMMAND_RESULT& r)
{
	BYTE* b = &(*arr)[27];
	UINT nDateTime = *(UINT*)b;
	BYTE len = b[4];
	r.tm = CTime(nDateTime);
	char* buf = r.strResult.GetBufferSetLength(len);
	memcpy(buf, &b[5], len);
	r.strResult.ReleaseBufferSetLength(len);
	return 1;
}

CString CNFC_Device::GetSerialNumber()
{
	return IsVirtualDevice() ? "Virtual" : S_O::FormatUINT(dev_info.nSerialNo);
}

CString DEVICE_LOG_RECORD::GetFreezeStatus()
{
	CStringArray a;
	if (nFreeze & 0x01)a.Add("Заморозка по статусу 21/22 разрешена");
	if (nFreeze & 0x02)a.Add("Показания заморожены по статусу 21/22");
	if (nFreeze & 0x04)a.Add("Очередь отложенных показаний по СH4 активна");
	if (nFreeze & 0x08)a.Add("Показания заморожены из-за температурной динамики");
	return S_O::CreateString(a,"\n");
}
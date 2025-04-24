/*
 ####################################################################################
 #  C++ HidApi Library                                                              #
 #  Copyright (C) 2015-2016 by Yigit YUCE                                           #
 ####################################################################################
 #                                                                                  #
 #  This file is part of C++ HidApi Library.                                        #
 #                                                                                  #
 #  C++ HidApi Library is free software: you can redistribute it and/or modify      #
 #  it under the terms of the GNU Lesser General Public License as published by     #
 #  the Free Software Foundation, either version 3 of the License, or               #
 #  (at your option) any later version.                                             #
 #                                                                                  #
 #  C++ HidApi Library is distributed in the hope that it will be useful,           #
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of                  #
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                    #
 #  GNU Lesser General Public License for more details.                             #
 #                                                                                  #
 #  You should have received a copy of the GNU Lesser General Public License        #
 #  along with this program. If not, see <http://www.gnu.org/licenses/>.            #
 #                                                                                  #
 #  For any comment or suggestion please contact the creator of C++ HidApi Library  #
 #  at ygtyce@gmail.com                                                             #
 #                                                                                  #
 ####################################################################################
 */


#include "pch.h"
#include "HidApi.h"



// ######################################### COMMON HIDERROR CLASS FUNCTION DEFINITIONS BEGIN ######################################### //
HidError::HidError(HidErrorCodes code, std::string str)
{
    this->errorCode   = code;
    this->errorString = ( str == "" ? errorStrings[code] : str );
}

HidError::~HidError()
{
}

HidError::HidErrorCodes HidError::getErrorCode()
{
    return this->errorCode;
}

std::string HidError::getErrorString()
{
    return this->errorString;
}

// ########################################## COMMON HIDERROR CLASS FUNCTION DEFINITIONS END ########################################## //





// ######################################## COMMON HIDDEVICE CLASS FUNCTION DEFINITIONS BEGIN ######################################### //
struct HidDllFunctions* HidDllFuncs=0;       /*!< stores the hid.dll library function pointers struct if the operating system is Windows. */
void           HidDevice::registerDeviceErrorCallback(void (*fptr)(HidDevice *,HidError))        { this->deviceErrorCallback  = fptr;     }
bool           HidDevice::isOpened()                                                     const { return this->opened;                   }
int            HidDevice::readAvailable()                                                const { return this->readFifoBuffer.size();    }
unsigned short HidDevice::getReadBufferSize() const { return this->internalReadBufferSize; }

std::string    HidDevice::read(int timeout)
{
    std::string ret = "";

    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_INITIALIZED));
        return ret;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_OPENED));
        return ret;
    }
    if (this->backgroundReader == 0)
    {
        if (timeout <= 0)timeout = 5000;
        DWORD       bytesRead;
        OVERLAPPED  readOL;
        memset(&(readOL), 0, sizeof(readOL));
        readOL.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        bytesRead = 0;
        ret.resize(internalReadBufferSize, 0);

        ResetEvent(readOL.hEvent);

        if (!ReadFile(devHandle, &(ret[0]), ret.size(), &bytesRead, &readOL))
        {
            DWORD nError = GetLastError();
            if (nError != ERROR_IO_PENDING)
            {
                CancelIo(devHandle);
                CloseHandle(readOL.hEvent);                
                return ret;
            }
        }
        std::clock_t start = std::clock();
        double end = (double)(timeout * ((double)(CLOCKS_PER_SEC / 1000)));
        while ((std::clock() - start) < end)
        {
            if (WaitForSingleObject(readOL.hEvent, POLLING_TIME_MS) == WAIT_OBJECT_0)
            {                                
                break;
            }
            if (GetOverlappedResult(devHandle, &readOL, &bytesRead, 0))
            {
                if (bytesRead > 0) break;
            }
        }
        if (!GetOverlappedResult(devHandle, &readOL, &bytesRead, 0))
        {
            CancelIo(devHandle);
        }
        CloseHandle(readOL.hEvent);
            
    }
    else
    {
        if (timeout < 0)
        {
            while (this->readAvailable() <= 0) { msleep(1); }

            ret = this->readFifoBuffer.front();
            this->readFifoBuffer.pop();
        }
        else if (timeout == 0)
        {
            if (this->readAvailable() > 0)
            {
                ret = this->readFifoBuffer.front();
                this->readFifoBuffer.pop();
            }
        }
        else
        {
            std::clock_t start = std::clock();
            double end = (double)(timeout * ((double)(CLOCKS_PER_SEC / 1000)));

            while ((std::clock() - start) < end)
            {
                //           if (evBreak && WaitForSingleObject(evBreak->m_hObject, 0) == WAIT_OBJECT_0)break;
                if (this->readAvailable() > 0)
                {
                    ret = this->readFifoBuffer.front();
                    this->readFifoBuffer.pop();
                    break;
                }
            }
        }
    }
    return ret;
}

// ########################################## COMMON HIDDEVICE CLASS FUNCTION DEFINITIONS END ######################################### //





// ###################################### OS SPECIFIC HIDDEVICE CLASS FUNCTION DEFINITIONS BEGIN ###################################### //

HidDevice::HidDevice()
{
    this->devHandle              = INVALID_HANDLE_VALUE;
    this->internalReadBufferSize     = DEFAULT_INTERNAL_BUFFER_SIZE;
    this->internalWriteBufferSize    = DEFAULT_INTERNAL_BUFFER_SIZE;
    this->opened                     = false;
    this->deviceErrorCallback        = NULL;
    this->backgroundReader = 0;
    evBreak = 0;
}

HidDevice::~HidDevice()
{
    this->close();
}

bool HidDevice::open(bool bBackgroundReader)
{
    if( this->isOpened() )
    {
        return true;
    }

    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_INITIALIZED));
        return false;
    }
    this->devHandle = CreateFile(this->descr.path.c_str(), (GENERIC_WRITE|GENERIC_READ), 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

    if(this->devHandle == INVALID_HANDLE_VALUE)
    {
        this->opened = false;
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_OPEN));
        return false;
    }

    HIDP_CAPS caps;
    void * pp_data = NULL;

    // TODO error handling operations will be done for these
    if(!HidDllFuncs->SetNumInputBuffers(this->devHandle, 64))
    {
    }

    if(!HidDllFuncs->GetPreparsedData(this->devHandle, &pp_data))
    {
    }

    if(HidDllFuncs->GetCaps(pp_data, &caps) != 0x110000 )
    {
    }

    internalReadBufferSize  = caps.InputReportByteLength;
    internalWriteBufferSize = caps.OutputReportByteLength;
    if (internalReadBufferSize == 0)internalReadBufferSize = 64;
    if (internalWriteBufferSize == 0)internalWriteBufferSize = 64;
    HidDllFuncs->FreePreparsedData(pp_data);
    this->opened          = true;
    if (bBackgroundReader)
    {
        if(!this->backgroundReader)this->backgroundReader = new HidDevice::HidDeviceReaderThread();
        this->backgroundReader->stop();
        this->backgroundReader->setParent(this);
        this->readFifoBuffer = std::queue<std::string>(); // flush
        this->backgroundReader->run();
    }
    return this->opened;
}

bool HidDevice::close() const
{
    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice *)this,HidError(HidError::DEVICE_INITIALIZED));
        return false;
    }

    if( this->isOpened() )
    {
        bool closed = (CloseHandle(this->devHandle) ? true : false);
        

        if( closed )
        {
            if( this->backgroundReader )
            {
                if( this->backgroundReader->isRunning() ) this->backgroundReader->stop();
            }
            this->opened = false;
        }
        else
        {
            if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice *)this,HidError(HidError::DEVICE_CLOSE));
        }
        return closed;
    }
    else
    {
        if( this->backgroundReader )
        {
            if( this->backgroundReader->isRunning() ) this->backgroundReader->stop();
        }
        return true;
    }
}

int HidDevice::write(BYTE* arr, UINT nSize) const
{
    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice *)this,HidError(HidError::DEVICE_INITIALIZED));
        return -1;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_OPENED));
        return -1;
    }

    DWORD       bytesWritten = -1;
    if (nSize > this->internalWriteBufferSize)nSize = this->internalWriteBufferSize;
    OVERLAPPED  ol;
    memset(&ol, 0, sizeof(ol));
    BYTE* send = new BYTE[this->internalWriteBufferSize];
    memset(send, 0, this->internalWriteBufferSize);
    memcpy(send, arr, nSize);
    if( !WriteFile(this->devHandle, send, this->internalWriteBufferSize, NULL, &ol) )
    {
        if( GetLastError() != ERROR_IO_PENDING )
        {
            delete[] send;
            bytesWritten = -1;
            if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_WRITE));
            return bytesWritten;
        }
    }
    std::clock_t start = std::clock();
    double end = (double)(5000 * ((double)(CLOCKS_PER_SEC / 1000)));
    while ((std::clock() - start) < end)
    {
        bool bResult = GetOverlappedResult(this->devHandle, &ol, &bytesWritten, FALSE/*wait*/);
        if (!bResult)
        {
            switch (GetLastError())
            {
            case ERROR_IO_INCOMPLETE:
            {
                Sleep(1);
                continue;
            }
            default:
            {
                delete[] send;
                bytesWritten = -1;
                if (this->deviceErrorCallback) (*(this->deviceErrorCallback))((HidDevice*)this, HidError(HidError::DEVICE_WRITE));
                return bytesWritten;
            }
            }
        }
        else break;
    }
    delete[] send;
    return bytesWritten;
}

bool HidDevice::flush()
{
    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_INITIALIZED));
        return false;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_OPENED));
        return false;
    }
    if( ! FlushFileBuffers(this->devHandle) )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))(this,HidError(HidError::DEVICE_FLUSH));
        return false;
    }
    return true;
}

std::wstring HidDevice::getIndexedString(int index)  const
{
    std::wstring retVal = L"";
    retVal.resize(this->internalReadBufferSize);

    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_INITIALIZED));
        return retVal;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_OPENED));
        return retVal;
    }
    if( !HidDllFuncs->GetIndexedString(this->devHandle, index, &(retVal[0]), retVal.size()) )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_GET_INDEXED));
        retVal = L"";
    }
    return retVal;
}

bool HidDevice::sendFeatureReport(std::string* data) const
{
    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_INITIALIZED));
        return false;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_OPENED));
        return false;
    }

        if( HidDllFuncs->SetFeature(this->devHandle, (PVOID)(&((*data)[0])), (*data).size()) )
        {
            if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_SEND_REPORT));
            return false;
        }
        return true;
}

bool HidDevice::recvFeatureReport(std::string* data) const
{
    if( !this->isInitialized() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_INITIALIZED));
        return false;
    }

    if( !this->isOpened() )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_OPENED));
        return false;
    }

    DWORD bytesReturned;
    OVERLAPPED ol;
    memset(&ol, 0, sizeof(ol));

    if( !DeviceIoControl(this->devHandle,
                            CTL_CODE(FILE_DEVICE_KEYBOARD, (100), METHOD_OUT_DIRECT, FILE_ANY_ACCESS),
                            NULL, 0, &((*data)[0]), (*data).size(), &bytesReturned, &ol))
    {
        if(GetLastError() != ERROR_IO_PENDING)
        {
            if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_RECV_REPORT));
            return false;
        }
    }

    if( !GetOverlappedResult(this->devHandle, &ol, &bytesReturned, TRUE) )
    {
        if( this->deviceErrorCallback ) (*(this->deviceErrorCallback))((HidDevice*)this,HidError(HidError::DEVICE_RECV_REPORT));
        return false;
    }

    bytesReturned++;
    (*data).resize(bytesReturned);
    return true;
}

bool HidDevice::isInitialized() const
{
     return ( HidDllFuncs != NULL );
}

// ###################################### OS SPECIFIC HIDDEVICE CLASS FUNCTION DEFINITIONS END ###################################### //





// ################################ HIDDEVICE::HIDDEVICEREADERTHREAD CLASS FUNCTION DEFINITIONS BEGIN ############################### //

HidDevice::HidDeviceReaderThread::HidDeviceReaderThread(HidDevice *dev)
{
    this->threadHandle  = INVALID_HANDLE_VALUE;
    this->runningMutex  = CreateMutex(NULL, FALSE, NULL);
    this->running           = false;
    this->device            = dev;
}

HidDevice::HidDeviceReaderThread::~HidDeviceReaderThread()
{
    this->stop();
    CloseHandle(this->runningMutex);
}

void HidDevice::HidDeviceReaderThread::setParent(HidDevice *dev)
{
    this->device = dev;
}

void HidDevice::HidDeviceReaderThread::run()
{
    if( this->device != NULL )
    {
        if( !this->device->isInitialized() )
        {
            if( this->device->deviceErrorCallback ) (*(this->device->deviceErrorCallback))((this->device),HidError(HidError::DEVICE_INITIALIZED));
            return;
        }

        if( !this->device->isOpened() )
        {
            if( this->device->deviceErrorCallback ) (*(this->device->deviceErrorCallback))((this->device),HidError(HidError::DEVICE_OPENED));
            return;
        }
        this->threadHandle = CreateThread( NULL,                                           // default security attributes
                                            0,                                              // use default stack size
                                            &HidDevice::HidDeviceReaderThread::threadFunc,  // thread function name
                                            (void*)this,                                    // argument to thread function
                                            0,                                              // use default creation flags
                                            0                                               // pointer of thread identifier
                                            );

        if( (this->threadHandle == INVALID_HANDLE_VALUE) && this->device->deviceErrorCallback )
        {
            (*(this->device->deviceErrorCallback))((this->device),HidError(HidError::DEVICE_READER_THREAD_CREATE));
        }
    }
    else
    {
        if( this->device->deviceErrorCallback )
        {
            (*(this->device->deviceErrorCallback))((this->device),HidError(HidError::DEVICE_READER_THREAD_SETUP));
        }
    }
}

void HidDevice::HidDeviceReaderThread::stop()
{
    WaitForSingleObject( this->runningMutex, INFINITE);
    this->running = false;
    ReleaseMutex(this->runningMutex);
    if(threadHandle!=INVALID_HANDLE_VALUE)WaitForSingleObject(threadHandle, INFINITE);
}

void HidDevice::HidDeviceReaderThread::onStartHandler()
{
    if( this->device != NULL )
    {
        std::string tempBuf;

            WaitForSingleObject( this->runningMutex, INFINITE);
            this->running = true;
            ReleaseMutex(this->runningMutex);

            DWORD       bytesRead;
            OVERLAPPED  readOL;

            while(this->running)
            {
                WaitForSingleObject( this->runningMutex, INFINITE);
                if( this->device->isInitialized() && this->device->isOpened() )
                {
                    memset(&(readOL), 0, sizeof(readOL));
                    readOL.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                    bytesRead     = 0;
                    tempBuf       = "";
                    tempBuf.resize(this->device->internalReadBufferSize, 0);

                    ResetEvent(readOL.hEvent);

                    if( !ReadFile(this->device->devHandle, &(tempBuf[0]), tempBuf.size(), &bytesRead, &readOL) )
                    {
                        DWORD nError = GetLastError();
                        if (nError != ERROR_IO_PENDING)
                        {
                            CancelIo(this->device->devHandle);
                            CloseHandle(readOL.hEvent);
                            ReleaseMutex(this->runningMutex);
                            /*if (nError == ERROR_DEVICE_NOT_CONNECTED)
                            {
                                this->running = false;
                                break;
                            }*/
                            msleep(1);
                            continue;
                        }
                    }

                    if( WaitForSingleObject(readOL.hEvent, POLLING_TIME_MS) != WAIT_OBJECT_0 )
                    {
                        CancelIo(this->device->devHandle);
                        CloseHandle(readOL.hEvent);
                        ReleaseMutex(this->runningMutex);
                        msleep(DEVICE_READ_INTERVAL_MS);
                        continue;
                    }

                    if( GetOverlappedResult(this->device->devHandle, &readOL, &bytesRead, TRUE) )
                    {
                        if( bytesRead > 0 ) this->device->readFifoBuffer.push(tempBuf);
                    }

                    CloseHandle(readOL.hEvent);
                }
                ReleaseMutex(this->runningMutex);
                msleep(DEVICE_READ_INTERVAL_MS);
            }
       
    }
    else
    {
        if( this->device->deviceErrorCallback )
        {
            (*(this->device->deviceErrorCallback))((this->device),HidError(HidError::DEVICE_READER_THREAD_SETUP));
        }
    }
}

bool HidDevice::HidDeviceReaderThread::isRunning()
{
    return this->running;
}

DWORD WINAPI HidDevice::HidDeviceReaderThread::threadFunc(void* classPtr)
{
    ((HidDevice::HidDeviceReaderThread*)classPtr)->onStartHandler();
    return 0;
}


// ################################# HIDDEVICE::HIDDEVICEREADERTHREAD CLASS FUNCTION DEFINITIONS END ################################ //





// ######################################### COMMON HIDAPI CLASS FUNCTION DEFINITIONS BEGIN ######################################### //
bool          HidApi::isInitialized()                                               { return this->initialized;          }
void          HidApi::registerDeviceAddCallback(void (*fptr)(HidDeviceDescr& descr))        { this->deviceAddCallback    = fptr; }
void          HidApi::registerDeviceRemoveCallback(void (*fptr)(std::string& path))     { this->deviceRemoveCallback = fptr; }
void          HidApi::registerApiErrorCallback(void (*fptr)(HidError))              { this->apiErrorCallback     = fptr; }
void          HidApi::registerDeviceErrorCallback(void (*fptr)(HidDevice *,HidError)) { this->deviceErrorCallback  = fptr; }


std::string HidApi::wcharArrayToString(const wchar_t* arr, int size)
{
    std::string result = "";
    if( !arr ) return result;

    if( size < 0 )
    {
        size = 0;
        while( arr[size] != 0 ){ ++size; }
    }

    result.resize(size);
    std::wcstombs(&result[0],arr,size);

    return result;
}

std::wstring HidApi::wcharArrayToWString(const wchar_t* arr, int size)
{
    if( !arr ) return L"";

    if( size < 0 )
    {
        size = 0;
        while( arr[size] != 0 ){ ++size; }
    }

    return std::wstring(arr,size);
}

std::wstring HidApi::stringToWString(std::string str)
{
    std::wstring wc;
    wc.resize( str.size()+1 );
    mbstowcs( &wc[0], &str[0], str.size()+1 );

    return wc;
}

std::wstring HidApi::charArrayToWString(const char *utf8)
{
    wchar_t *ret = NULL;

    if (utf8)
    {
        size_t wlen = mbstowcs(NULL, utf8, 0);
        if ((size_t) -1 == wlen) {
            return std::wstring( L"" );
        }
        ret = (wchar_t *)calloc(wlen+1, sizeof(wchar_t));
        mbstowcs(ret, utf8, wlen+1);
        ret[wlen] = 0x0000;
    }

    return HidApi::wcharArrayToWString(ret);
}

// ########################################## COMMON HIDAPI CLASS FUNCTION DEFINITIONS END ########################################## //





// ####################################### OS SPECIFIC HIDAPI CLASS FUNCTION DEFINITIONS BEGIN ###################################### //

HidApi::HidApi(void (*apiErrCb)(HidError), unsigned short _vendorId, unsigned short _productId)
{
    this->_vendorId = _vendorId;
    this->_productId = _productId;

    this->deviceAddCallback    = NULL;
    this->deviceRemoveCallback = NULL;
    this->deviceErrorCallback  = NULL;
    this->apiErrorCallback     = apiErrCb;

    this->libHandle    = LoadLibraryA("hid.dll");
    this->HidDllFuncs  = new struct HidDllFunctions;
    ::HidDllFuncs = this->HidDllFuncs;

    if (this->libHandle)
    {
        this->HidDllFuncs->GetAttributes         = (BOOLEAN  (WINAPI *)(HANDLE, PHIDD_ATTRIBUTES))    GetProcAddress(this->libHandle, "HidD_GetAttributes");
        this->HidDllFuncs->GetSerialNumberString = (BOOLEAN  (WINAPI *)(HANDLE, PVOID, ULONG))        GetProcAddress(this->libHandle, "HidD_GetSerialNumberString");
        this->HidDllFuncs->GetManufacturerString = (BOOLEAN  (WINAPI *)(HANDLE, PVOID, ULONG))        GetProcAddress(this->libHandle, "HidD_GetManufacturerString");
        this->HidDllFuncs->GetProductString      = (BOOLEAN  (WINAPI *)(HANDLE, PVOID, ULONG))        GetProcAddress(this->libHandle, "HidD_GetProductString");
        this->HidDllFuncs->SetFeature            = (BOOLEAN  (WINAPI *)(HANDLE, PVOID, ULONG))        GetProcAddress(this->libHandle, "HidD_SetFeature");
        this->HidDllFuncs->GetFeature            = (BOOLEAN  (WINAPI *)(HANDLE, PVOID, ULONG))        GetProcAddress(this->libHandle, "HidD_GetFeature");
        this->HidDllFuncs->GetIndexedString      = (BOOLEAN  (WINAPI *)(HANDLE, ULONG, PVOID, ULONG)) GetProcAddress(this->libHandle, "HidD_GetIndexedString");
        this->HidDllFuncs->GetPreparsedData      = (BOOLEAN  (WINAPI *)(HANDLE, void **))             GetProcAddress(this->libHandle, "HidD_GetPreparsedData");
        this->HidDllFuncs->FreePreparsedData     = (BOOLEAN  (WINAPI *)(void *))                      GetProcAddress(this->libHandle, "HidD_FreePreparsedData");
        this->HidDllFuncs->GetCaps               = (UINT (WINAPI *)(void *, HIDP_CAPS *))         GetProcAddress(this->libHandle, "HidP_GetCaps");
        this->HidDllFuncs->SetNumInputBuffers    = (BOOLEAN  (WINAPI *)(HANDLE, ULONG))               GetProcAddress(this->libHandle, "HidD_SetNumInputBuffers");
    }
    else
    {
        this->HidDllFuncs->GetAttributes         = NULL;
        this->HidDllFuncs->GetSerialNumberString = NULL;
        this->HidDllFuncs->GetManufacturerString = NULL;
        this->HidDllFuncs->GetProductString      = NULL;
        this->HidDllFuncs->SetFeature            = NULL;
        this->HidDllFuncs->GetFeature            = NULL;
        this->HidDllFuncs->GetIndexedString      = NULL;
        this->HidDllFuncs->GetPreparsedData      = NULL;
        this->HidDllFuncs->FreePreparsedData     = NULL;
        this->HidDllFuncs->GetCaps               = NULL;
        this->HidDllFuncs->SetNumInputBuffers    = NULL;
    }



    this->initialized = ((this->HidDllFuncs->GetAttributes)         &&
                            (this->HidDllFuncs->GetSerialNumberString) &&
                            (this->HidDllFuncs->GetManufacturerString) &&
                            (this->HidDllFuncs->GetProductString) &&
                            (this->HidDllFuncs->SetFeature) &&
                            (this->HidDllFuncs->GetFeature) &&
                            (this->HidDllFuncs->GetIndexedString) &&
                            (this->HidDllFuncs->GetPreparsedData) &&
                            (this->HidDllFuncs->FreePreparsedData) &&
                            (this->HidDllFuncs->GetCaps) &&
                            (this->HidDllFuncs->SetNumInputBuffers));


    if( this->initialized )
    {
        this->monitorThread = new HidApi::HidDeviceMonitoringThread(this);
        this->monitorThread->run();
    }

}

HidApi::~HidApi()
{
    this->monitorThread->stop();
    if( this->libHandle ) FreeLibrary(this->libHandle);
    this->libHandle = NULL;
    this->initialized = false;
}

HidDeviceList HidApi::scanDevices(unsigned short _vendorId,
                                  unsigned short _productId,
                                  const  wchar_t* _serial,
                                  const wchar_t* _manufacturer,
                                  const wchar_t* _product,
                                  unsigned short _release,
                                  unsigned short _usagePage,
                                  unsigned short _usage)
{
    if (_vendorId == ANY)_vendorId = this->_vendorId;
    if (_productId == ANY)_productId = this->_productId;
    HidDeviceList devices;
    if( !this->initialized )
    {
        if( this->apiErrorCallback ) (*(this->apiErrorCallback))(HidError(HidError::API_INITIALIZED));
        return devices;
    }

        BOOL                                res;
        int                                 deviceIndex = 0 ;
        SP_DEVINFO_DATA                     devinfoData;
        SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
        SP_DEVICE_INTERFACE_DETAIL_DATA * deviceInterfaceDetailData = NULL;
        GUID                                InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

        memset(&devinfoData,           0x0, sizeof(devinfoData));
        memset(&deviceInterfaceData,   0x0, sizeof(deviceInterfaceData));
        devinfoData.cbSize                = sizeof(SP_DEVINFO_DATA);
        deviceInterfaceData.cbSize        = sizeof(SP_DEVICE_INTERFACE_DATA);

        HDEVINFO deviceInfoSet            = SetupDiGetClassDevsA(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);



        while( SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &InterfaceClassGuid, deviceIndex, &deviceInterfaceData) )
        {
            HANDLE          writeHandle   = INVALID_HANDLE_VALUE;
            DWORD           requiredSize  = 0;
            HIDD_ATTRIBUTES attrib;
            char DeviceDescr[256]="";

            res = SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

            deviceInterfaceDetailData         = (SP_DEVICE_INTERFACE_DETAIL_DATA *) malloc(requiredSize);
            deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            res = SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, NULL, NULL);

            if (!res)
            {
                free(deviceInterfaceDetailData);
                deviceIndex++;
                continue;
            }

            for (int i = 0; ; i++)
            {
                char driverName[256];

                res = SetupDiEnumDeviceInfo(deviceInfoSet, i, &devinfoData);
                if(!res) break;

                res = SetupDiGetDeviceRegistryPropertyA(deviceInfoSet,
                                                        &devinfoData,
                                                        SPDRP_CLASS,
                                                        NULL,
                                                        (PBYTE)driverName,
                                                        sizeof(driverName),
                                                        NULL);
                if(!res) break;

                if(strcmp(driverName, "HIDClass") == 0)
                {
                    res = SetupDiGetDeviceRegistryPropertyA(deviceInfoSet,
                                                            &devinfoData,
                                                            SPDRP_DRIVER,
                                                            NULL,
                                                            (PBYTE)driverName,
                                                            sizeof(driverName),
                                                            NULL);
                    if (res)
                    {                        
                        DWORD buffersize = 256;
                        SetupDiGetDeviceRegistryProperty(deviceInfoSet, &devinfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)DeviceDescr, buffersize, &buffersize);
                        break;
                    }
                }
            }


            if( !res )
            {
                free(deviceInterfaceDetailData);
                deviceIndex++;
                continue;
            }


            writeHandle = CreateFile(deviceInterfaceDetailData->DevicePath,
                                      0,
                                      (FILE_SHARE_READ|FILE_SHARE_WRITE),
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_OVERLAPPED,
                                      0);


            if (writeHandle == INVALID_HANDLE_VALUE)
            {
                free(deviceInterfaceDetailData);
                deviceIndex++;
                continue;
            }
            attrib.Size = sizeof(HIDD_ATTRIBUTES);

            void      *ppData     = NULL;
            HIDP_CAPS caps;
            wchar_t   serial[256];
            wchar_t   manufa[256];
            wchar_t   produc[256];
            bool      isPreparsed = false;
            bool      isAttr      = false;
            bool      isSerial    = false;
            bool      isManufa    = false;
            bool      isProduc    = false;


            if( this->HidDllFuncs->GetPreparsedData(writeHandle, &ppData) )
            {
                if(this->HidDllFuncs->GetCaps(ppData, &caps) == 0x110000)
                {
                    isPreparsed   = true;
                }

                this->HidDllFuncs->FreePreparsedData(ppData);
            }

            if( this->HidDllFuncs->GetAttributes(writeHandle, &attrib) )                        { isAttr   = true; }
            if( this->HidDllFuncs->GetSerialNumberString(writeHandle, serial, sizeof(serial) )) { isSerial = true; }
            if( this->HidDllFuncs->GetManufacturerString(writeHandle, manufa, sizeof(manufa) )) { isManufa = true; }
            if( this->HidDllFuncs->GetProductString(writeHandle, produc, sizeof(produc) ))      { isProduc = true; }



            if( ((_vendorId      == ANY) || ( isAttr      && ( _vendorId  == attrib.VendorID )      )) &&
                ((_productId     == ANY) || ( isAttr && ( _productId == attrib.ProductID )     )) &&
                ((_serial        == ANY) || ( isSerial && ( wcscmp( _serial, serial)       == 0 ))) &&
                ((_manufacturer  == ANY) || ( isManufa && ( wcscmp( _manufacturer, manufa) == 0 ))) &&
                ((_product       == ANY) || ( isProduc && ( wcscmp( _product, produc)      == 0 ))) &&
                ((_release       == ANY) || ( isAttr && ( _release   == attrib.VersionNumber ) )) &&
                ((_usagePage     == ANY) || ( isPreparsed && ( _usagePage == caps.UsagePage )       )) &&
                ((_usage         == ANY) || ( isPreparsed && ( _usage     == caps.Usage )           ))
            )
            {
                HidDeviceDescr temp;


                temp.path = std::string(deviceInterfaceDetailData->DevicePath);
                std::transform(temp.path.begin(), temp.path.end(), temp.path.begin(), ::toupper);

                if( isAttr )
                {
                    temp.vendorId  = attrib.VendorID;
                    temp.productId = attrib.ProductID;
                    temp.release   = attrib.VersionNumber;
                }

                if( isPreparsed )
                {
                    temp.usagePage = caps.UsagePage;
                    temp.usage     = caps.Usage;
                }

                if( isSerial ) { temp.serial       = HidApi::wcharArrayToWString(serial); }
                if( isManufa ) { temp.manufacturer = HidApi::wcharArrayToWString(manufa); }
                if( isProduc ) { temp.product      = HidApi::wcharArrayToWString(produc); }
                temp.descr = DeviceDescr;


                temp.interfaceNumber = 0;
                size_t foundAt = temp.path.find(_T("&mi_"));
                if (foundAt != std::string::npos )
                {
                    foundAt += 4;
                    temp.interfaceNumber = strtol(&(temp.path[foundAt]), NULL, 16);
                }

                devices.push_back(temp);
            }
            CloseHandle(writeHandle);
            free(deviceInterfaceDetailData);
            deviceIndex++;
        }//main while loop

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return devices;
}

// ######################################## OS SPECIFIC HIDAPI CLASS FUNCTION DEFINITIONS END ####################################### //





// ################################ HIDAPI::HIDDEVICEMONITORINGTHREAD CLASS FUNCTION DEFINITIONS BEGIN ############################## //
HidApi::HidDeviceMonitoringThread::HidDeviceMonitoringThread(HidApi *pr)
{
    this->running               = false;
    this->parent                = pr;
    this->runningMutex      = CreateMutex(NULL, FALSE, NULL);
    this->threadHandle      = INVALID_HANDLE_VALUE;
}

HidApi::HidDeviceMonitoringThread::~HidDeviceMonitoringThread()
{
    this->stop();
    CloseHandle(this->runningMutex);
}

void HidApi::HidDeviceMonitoringThread::run()
{
    if( this->parent->initialized )
    {
        this->threadHandle = CreateThread( NULL,                                                       // default security attributes
                                            0,                                                          // use default stack size
                                            &HidApi::HidDeviceMonitoringThread::threadFunc,             // thread function name
                                            (void*)this,                                                // argument to thread function
                                            0,                                                          // use default creation flags
                                            0                                                           // pointer of thread identifier
                                            );

        if( (this->threadHandle == INVALID_HANDLE_VALUE) && this->parent->apiErrorCallback )
        {
            (*(this->parent->apiErrorCallback))(HidError(HidError::API_MONITOR_THREAD_CREATE));
        }

        
    }
    else
    {
        if( this->parent->apiErrorCallback )
        {
            (*(this->parent->apiErrorCallback))(HidError(HidError::API_INITIALIZED));
        }
    }
}

void HidApi::HidDeviceMonitoringThread::stop()
{
    WaitForSingleObject( this->runningMutex, INFINITE);
    this->running = false;
    ReleaseMutex(this->runningMutex);
    WaitForSingleObject(this->threadHandle, INFINITE);
 
}

void HidApi::HidDeviceMonitoringThread::onStartHandler()
{
    if( this->parent->initialized )
    {
            WaitForSingleObject( this->runningMutex, INFINITE);
            this->running = true;
            ReleaseMutex(this->runningMutex);

            WNDCLASSEX  wc;
            wc.cbSize        = sizeof(WNDCLASSEX);
            wc.style         = 0;
            wc.lpfnWndProc   = &HidApi::HidDeviceMonitoringThread::onMessageReceivedHandler;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = sizeof(HidApi::HidDeviceMonitoringThread*);
            wc.hInstance     = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
            wc.hIcon         = 0;
            wc.hCursor       = 0;
            wc.hbrBackground = 0;
            wc.lpszMenuName  = NULL;
            wc.lpszClassName = _T("MonitoringClassName");
            wc.hIconSm       = NULL;
            RegisterClassEx(&wc);


            HWND monitorWindow = CreateWindowEx( 0,                                               // extra style
                                                 _T("MonitoringClassName"),                           // classname
                                                 _T("MonitoringClassName"),                           // window name
                                                 0,                                               // style
                                                 0, 0, 0, 0,                                      // geometry
                                                 HWND_MESSAGE,                                    // parent (message-only window)
                                                 NULL,                                            // menu handle
                                                 reinterpret_cast<HINSTANCE>(GetModuleHandle(0)), // application handle
                                                 NULL);                                           // windows creation data

            if( monitorWindow )
            {
                SetWindowLongPtr(monitorWindow, 0, reinterpret_cast<LONG_PTR>(this));

                DEV_BROADCAST_DEVICEINTERFACE NotificationFilter ;
                memset(&NotificationFilter, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE)) ;
                NotificationFilter.dbcc_size        = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
                NotificationFilter.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
                NotificationFilter.dbcc_classguid   = { 0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
                HDEVNOTIFY monitorNotify            = RegisterDeviceNotification(monitorWindow, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

                if( monitorNotify )
                {
                    MSG msg;
                    while( this->running )
                    {
                        WaitForSingleObject( this->runningMutex, INFINITE);
                        if( PeekMessage(&msg, monitorWindow, 0, 0,PM_REMOVE) )
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        ReleaseMutex(this->runningMutex);
                        msleep(API_CHECK_DEVICES_INTERVAL_MS);
                    }
                }
                else
                {
                    if( this->parent->apiErrorCallback )
                    {
                        (*(this->parent->apiErrorCallback))(HidError(HidError::API_MONITOR_THREAD_SETUP));
                    }

                    WaitForSingleObject( this->runningMutex, INFINITE);
                    this->running = false;
                    ReleaseMutex(this->runningMutex);
                }
            }
            else
            {
                if( this->parent->apiErrorCallback )
                {
                    (*(this->parent->apiErrorCallback))(HidError(HidError::API_MONITOR_THREAD_SETUP));
                }

                WaitForSingleObject( this->runningMutex, INFINITE);
                this->running = false;
                ReleaseMutex(this->runningMutex);
            }


        if( this->parent->apiErrorCallback )
        {
            (*(this->parent->apiErrorCallback))(HidError(HidError::API_MONITOR_THREAD_STOP));
        }
    }
    else
    {
        if( this->parent->apiErrorCallback )
        {
            (*(this->parent->apiErrorCallback))(HidError(HidError::API_INITIALIZED));
        }
    }


}

LRESULT CALLBACK HidApi::HidDeviceMonitoringThread::onMessageReceivedHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HidApi::HidDeviceMonitoringThread* _this = reinterpret_cast<HidApi::HidDeviceMonitoringThread*>(GetWindowLongPtr(hwnd, 0));

    if( _this )
    {
        if( message == WM_DEVICECHANGE )
        {
            DEV_BROADCAST_HDR *lpdb = (DEV_BROADCAST_HDR *)lParam;
            if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
                std::string node(pDevInf->dbcc_name);
                std::transform(node.begin(), node.end(), node.begin(), ::toupper);

                if( wParam == DBT_DEVICEARRIVAL)
                {
                    HidDeviceList devices=_this->parent->scanDevices(_this->parent->_vendorId, _this->parent->_productId);

                    int addedToList = -1;
                    for( size_t i = 0 ; i < devices.size() ; i++)
                    {
                        if( devices[i].path == node )
                        {
                            addedToList = static_cast<int>(i);
                            break;
                        }
                    }
                    if( addedToList >= 0 )
                    {
                        if( _this->parent->deviceAddCallback )
                        {
                            (*(_this->parent->deviceAddCallback))(devices[addedToList]);
                        }
                    }
                    else
                    {
                        if( _this->parent->apiErrorCallback )
                        {
                            (*(_this->parent->apiErrorCallback))(HidError(HidError::API_ADD));
                        }
                    }
                }
                else if( wParam == DBT_DEVICEREMOVECOMPLETE)
                {                    
                    if( _this->parent->deviceRemoveCallback )
                    {
                        (*(_this->parent->deviceRemoveCallback))(node);
                    }                  
                }
            }
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

DWORD WINAPI HidApi::HidDeviceMonitoringThread::threadFunc(void* classPtr)
{
    ((HidApi::HidDeviceMonitoringThread*)classPtr)->onStartHandler();
    return 0;
}


// ################################# HIDAPI::HIDDEVICEMONITORINGTHREAD CLASS FUNCTION DEFINITIONS END ############################### //


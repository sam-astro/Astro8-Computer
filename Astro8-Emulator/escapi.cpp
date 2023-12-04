#include <windows.h>
#include "escapi.h"

countCaptureDevicesProc countCaptureDevices;
initCaptureProc initCapture;
deinitCaptureProc deinitCapture;
doCaptureProc doCapture;
isCaptureDoneProc isCaptureDone;
getCaptureDeviceNameProc getCaptureDeviceName;
ESCAPIVersionProc ESCAPIVersion;
getCapturePropertyValueProc getCapturePropertyValue;
getCapturePropertyAutoProc getCapturePropertyAuto;
setCapturePropertyProc setCaptureProperty;
getCaptureErrorLineProc getCaptureErrorLine;
getCaptureErrorCodeProc getCaptureErrorCode;


/* Internal: initialize COM */
typedef void (*initCOMProc)();
initCOMProc initCOM;

int setupESCAPI()
{
  /* Load DLL dynamically */
  HMODULE capdll = LoadLibraryA("escapi.dll");
  if (capdll == NULL)
    return 0;

  /* Fetch function entry points */
  countCaptureDevices = (countCaptureDevicesProc)GetProcAddress(capdll, "countCaptureDevices");
  initCapture = (initCaptureProc)GetProcAddress(capdll, "initCapture");
  deinitCapture = (deinitCaptureProc)GetProcAddress(capdll, "deinitCapture");
  doCapture = (doCaptureProc)GetProcAddress(capdll, "doCapture");
  isCaptureDone = (isCaptureDoneProc)GetProcAddress(capdll, "isCaptureDone");
  initCOM = (initCOMProc)GetProcAddress(capdll, "initCOM");
  getCaptureDeviceName = (getCaptureDeviceNameProc)GetProcAddress(capdll, "getCaptureDeviceName");
  ESCAPIVersion = (ESCAPIVersionProc)GetProcAddress(capdll, "ESCAPIVersion");
  getCapturePropertyValue = (getCapturePropertyValueProc)GetProcAddress(capdll, "getCapturePropertyValue");
  getCapturePropertyAuto = (getCapturePropertyAutoProc)GetProcAddress(capdll, "getCapturePropertyAuto");
  setCaptureProperty = (setCapturePropertyProc)GetProcAddress(capdll, "setCaptureProperty");
  getCaptureErrorLine = (getCaptureErrorLineProc)GetProcAddress(capdll, "getCaptureErrorLine");
  getCaptureErrorCode = (getCaptureErrorCodeProc)GetProcAddress(capdll, "getCaptureErrorCode");


  /* Check that we got all the entry points */
  if (initCOM == NULL ||
      ESCAPIVersion == NULL ||
      getCaptureDeviceName == NULL ||
      countCaptureDevices == NULL ||
      initCapture == NULL ||
      deinitCapture == NULL ||
      doCapture == NULL ||
      isCaptureDone == NULL ||
	  getCapturePropertyValue == NULL ||
	  getCapturePropertyAuto == NULL ||
	  setCaptureProperty == NULL ||
	  getCaptureErrorLine == NULL ||
	  getCaptureErrorCode == NULL)
      return 0;

  /* Verify DLL version is at least what we want */
  if (ESCAPIVersion() < 0x300)
    return 0;

  /* Initialize COM.. */
  initCOM();

  /* and return the number of capture devices found. */
  return countCaptureDevices();
}


// max-bright.cpp : A small Windows utility to quickly maximize the screen brightness.
//

#include "stdafx.h"
#include <windows.h>
#include <Psapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <iostream>
#include <string>

int listProcesses() {
	DWORD processes[1024], needed, processCount;
	unsigned int i;
	if (!EnumProcesses(processes, sizeof(processes), &needed)) {
		return 1;
	}

	// How many Process IDs were returned
	processCount = needed / sizeof(DWORD);
	std::cout << processCount;

	return 0;
}

int brightnessOps2() {
	DWORD minBrightness, curBrightness, maxBrightness;
	HWND curWin = GetConsoleWindow();
	if (curWin == NULL) {
		std::cout << "Problem getting a handle to the window." << std::endl;
		return 1;
	}

	// Call MonitorFromWindow to get the HMONITOR handle
	HMONITOR curMon = MonitorFromWindow(curWin, MONITOR_DEFAULTTONULL);
	if (curMon == NULL) {
		std::cout << "Problem getting the display monitor" << std::endl;
		return 1;
	}

	// Call GetNumberOfPhysicalMonitorsFromHMONITOR to get the needed array size
	DWORD monitorCount;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(curMon, &monitorCount)) {
		std::cout << "Problem getting the number of physical monitors" << std::endl;
		return 1;
	}

	// Call GetPhysicalMonitorsFromHMONITOR to get a handle to the physical monitor 
	LPPHYSICAL_MONITOR physicalMonitors = (LPPHYSICAL_MONITOR)malloc(monitorCount*sizeof(PHYSICAL_MONITOR));
	if (physicalMonitors == NULL) {
		std::cout << "Unable to malloc the physical monitor array." << std::endl;
		return 1;
	}
	if (!GetPhysicalMonitorsFromHMONITOR(curMon, monitorCount, physicalMonitors)) {
		std::cout << "Problem getting the physical monitors." << std::endl;
		return 1;
	}
	std::cout << "Num Monitors: " << monitorCount << std::endl;
	wprintf(L"%s\n", physicalMonitors[0].szPhysicalMonitorDescription);

	// From here start using the "Low-level Monitor Configuration Functions
	// https://msdn.microsoft.com/en-us/library/dd692982(v=vs.85).aspx 
	DWORD monCapLen;
	if (!GetCapabilitiesStringLength(physicalMonitors[0].hPhysicalMonitor, &monCapLen)) {
		std::cout << "Problem getting the monitor's capabilities' string length." << std::endl;
		DWORD errNum = GetLastError();
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		LPVOID buffer;
		FormatMessage(flags, NULL, errNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);
		wprintf(L"%s\n", buffer);
		return 1;
	}
}

int brightnessOps() {
	// Method: Work backwards. I found "Get/Set MonitorBrightness" functions, then looked at what parameters they needed and read the docs
	// Continued up the chain...
	// Or you could look here for the overview: https://msdn.microsoft.com/en-us/library/dd692981(v=vs.85).aspx

	// Note: I had linker errors. I had to manually go to "Project->max-bright properties" then "Linker" "Input" "Additional Dependencies" and add "dxva2.lib".

	DWORD minBrightness, curBrightness, maxBrightness;
	HWND curWin = GetConsoleWindow();
	if (curWin == NULL) {
		std::cout << "Problem getting a handle to the window." << std::endl;
		return 1;
	}

	// Call MonitorFromWindow to get the HMONITOR handle
	HMONITOR curMon = MonitorFromWindow(curWin, MONITOR_DEFAULTTONULL);
	if (curMon == NULL) {
		std::cout << "Problem getting the display monitor" << std::endl;
		return 1;
	}

	// Call GetNumberOfPhysicalMonitorsFromHMONITOR to get the needed array size
	DWORD monitorCount;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(curMon, &monitorCount)) {
		std::cout << "Problem getting the number of physical monitors" << std::endl;
		return 1;
	}

	// Call GetPhysicalMonitorsFromHMONITOR to get a handle to the physical monitor
	PHYSICAL_MONITOR* physicalMonitors = new PHYSICAL_MONITOR[monitorCount];
	if (physicalMonitors == NULL) {
		std::cout << "Unable to malloc the physical monitor array." << std::endl;
		return 1;
	}
	if (!GetPhysicalMonitorsFromHMONITOR(curMon, monitorCount, physicalMonitors)) {
		std::cout << "Problem getting the physical monitors." << std::endl;
		return 1;
	}
	std::cout << "Num Monitors: " << monitorCount << std::endl;
	// These next two lines took some googling. I initially used this line
	//std::cout << physicalMonitors[0].szPhysicalMonitorDescription << std::endl;
	// and it printed 002471FC. Then I saw an example online using wprintf. I was more familiar with printf, so I tried the following line:
	//printf("%s\n", physicalMonitors[0].szPhysicalMonitorDescription);
	// and it printed "G". Then I tried the wprintf function
	//wprintf("%s\n", physicalMonitors[0].szPhysicalMonitorDescription);
	// but got a syntax error: "Error: argument of type "const char*" is incompatible with parameter of type "const wchar_t*".
	// Turns out I have to do the following to get it to work. Need to research what "L" does and wprintf vs printf.
	// My initial hunch is it has to do with 16 bit characters vs the 8-bit chars that printf expects.
	// That would explain printf only printing the first letter, etc. Need to look into this. (deep rabbit hole)
	// See: https://msdn.microsoft.com/en-us/library/aa272863(v=vs.60).aspx
	// And: http://stackoverflow.com/questions/6300804/wchars-encodings-standards-and-portability
	wprintf(L"%s\n", physicalMonitors[0].szPhysicalMonitorDescription);
	printf("Physical Monitor Handle: %x\n", physicalMonitors[0].hPhysicalMonitor);


	// Call GetMonitorCapabilities to find out which functions it supports (For now assume it is the first monitor)
	DWORD monCaps;
	DWORD monColorTemps;
	if (!GetMonitorCapabilities(physicalMonitors[0].hPhysicalMonitor, &monCaps, &monColorTemps)) {
		// This function call was failing, but I can't tell why. Documentation said to use "GetLastError" function.
		// This lead me to this: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
		// And the FormatMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351(v=vs.85).aspx
		std::cout << "Problem getting the monitor's capabilities." << std::endl;
		DWORD errNum = GetLastError();
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		LPVOID buffer;
		FormatMessage(flags, NULL, errNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);
		wprintf(L"%s\n", buffer);
		// I got the error printing to work! This is the error: "An error occurred while transmitting data to the device on the I2C bus."
		// Google...
		// Actually not much help... I've been stuck here for a while. I may try another, somewhat similar method...
		return 1;
	}
	if ((monCaps & MC_CAPS_BRIGHTNESS) != MC_CAPS_BRIGHTNESS) {
		std::cout << "Monitor does not support brightness operations." << std::endl;
		return 1;
	}
	// First get the current brightness
	if (!GetMonitorBrightness(physicalMonitors[0].hPhysicalMonitor, &minBrightness, &curBrightness, &maxBrightness)) {
		std::cout << "Problem querying the brightness options." << std::endl;
		return 1;
	}
	std::cout << "Minimum brightness: " << minBrightness << std::endl;
	std::cout << "Maximum brightness: " << maxBrightness << std::endl;
	std::cout << "Current brightness: " << curBrightness << std::endl;

	// Finally call get brightness, then call set brightness accordingly
	//GetMonitorBrightness();

	// Close the monitor handles

	// Free the allocated array
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	brightnessOps();
	//brightnessOps2();

	std::cout << "Hello, Windows" << std::endl;

	std::string temp;
	std::cin >> temp;
	return 0;
}


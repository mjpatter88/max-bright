// max-bright.cpp : A small Windows utility to quickly maximize the screen brightness.
//

#include "stdafx.h"
#include <windows.h>
#include <Psapi.h>
#include <highlevelmonitorconfigurationapi.h>
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

int _tmain(int argc, _TCHAR* argv[])
{
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
	LPPHYSICAL_MONITOR physicalMonitors = (LPPHYSICAL_MONITOR)malloc(monitorCount*sizeof(PHYSICAL_MONITOR));
	if (physicalMonitors == NULL) {
		std::cout << "Unable to malloc the physical monitor array." << std::endl;
		return 1;
	}
	if (!GetPhysicalMonitorsFromHMONITOR(curMon, monitorCount, physicalMonitors)) {
		std::cout << "Problem getting the physical monitors." << std::endl;
		return 1;
	}

	// Call GetMonitorCapabilities to find out which functions it supports
	std::cout << "Num Monitors: " << monitorCount << std::endl;

	// Finally call get brightness, then call set brightness accordingly
	//GetMonitorBrightness();

	std::cout << "Hello, Windows" << std::endl;

	std::string temp;
	std::cin >> temp;
	return 0;
}


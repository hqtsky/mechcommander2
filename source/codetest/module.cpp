/*******************************************************************************
 module.cpp - implementation of DLL entrypoint and DLL exports

 codetest

 2018-09-03 Jerker Back, created

 $LastChangedBy$

================================================================================
 RcsID = $Id$ */

#include "stdinc.h"
#include "module.h"

int
testresource();

/// <summary>
/// <c>WinMain</c> application entry-point function
/// </summary>
/// <remarks>override _AtlModule.WinMain and _AtlModule.Run for custom implementations
/// </remarks>
/// <param name="hinstance">application handle</param>
/// <param name="hPrevInstance">reserved</param>
/// <param name="commandline">pointer to command line arguments</param>
/// <param name="nShowCmd">initial window state</param>
/// <returns>application exit code</returns>
extern "C" int WINAPI
wWinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ const std::wstring_view& commandline, _In_ int nShowCmd)
{
	(void)hinstance;
	(void)hPrevInstance;
	(void)commandline;
	(void)commandline;
	(void)nShowCmd;

	int result = EXIT_SUCCESS;
	{
		result = testresource();
	}

	return result;
}

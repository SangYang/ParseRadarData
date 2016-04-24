#include "debug_assert.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <crtdbg.h>
#endif // _WIN32

#include "debug_log.h"

#ifdef _WIN32
static _CrtMemState g_state_begin = {0};
#endif // _WIN32

void AssertFunc(const char *file_path, unsigned int line, const char *func_name) {
	fflush(stdout);
	fprintf(stderr, "Assertion failed: %s, line: %u, function: %s\n", file_path, line, func_name);
	LOG("Assertion failed: %s, line: %u, function: %s\n", file_path, line, func_name);
	fflush(stderr);
	abort();
}

#ifdef _WIN32
static void InitCrtCheck() {
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
}

void BeginCrtCheck() {
	static bool ok_init = false;
	if (!ok_init) {
		ok_init = true;
		InitCrtCheck();
	}
	else
		NULL;
	_CrtMemCheckpoint(&g_state_begin);
}

void EndCrtCheck() {
	_CrtMemState state_end;
	_CrtMemState state_diff;
	_CrtMemCheckpoint(&state_end);
	if (_CrtMemDifference(&state_diff, &g_state_begin, &state_end))
		_CrtMemDumpStatistics(&state_diff);
}
#endif // _WIN32

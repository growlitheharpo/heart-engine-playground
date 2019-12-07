#include <heart/debug/message_box.h>

#include <heart/stl/vector.h>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

#include <CommCtrl.h>
#include <debugapi.h>
#include <shellapi.h>
#include <stdio.h>

#pragma comment(linker,                                                                                                \
	"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

typedef WINCOMMCTRLAPI HRESULT(WINAPI* TaskDialogProc)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);

static thread_local bool s_inside = false;
static SRWLOCK s_lock = SRWLOCK_INIT;

class TaskDialogProcWrapper
{
private:
	HMODULE comctrl32_ = nullptr;
	ULONG_PTR ulp_activation_cookie = 0;

public:
	TaskDialogProc procaddr = nullptr;

	bool Load()
	{
		ACTCTXW actCtx = {
			sizeof(actCtx),
			ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID,
			nullptr,
			0,
			0,
			nullptr,
			(LPCTSTR)124,
			nullptr,
			GetModuleHandleW(L"SHELL32.dll")
		};
		ActivateActCtx(CreateActCtxW(&actCtx), &ulp_activation_cookie);

		comctrl32_ = LoadLibraryW(L"COMCTL32.DLL");
		if (comctrl32_)
			procaddr = (TaskDialogProc)GetProcAddress(comctrl32_, "TaskDialogIndirect");

		return procaddr != nullptr;
	}

	~TaskDialogProcWrapper()
	{
		if (comctrl32_ != nullptr)
			FreeLibrary(comctrl32_);

		DeactivateActCtx(0, ulp_activation_cookie);
	}
};

void ConvertToWChar(const char* in, wchar_t** outBuf, size_t* outCount)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
	*outCount = size_t(size);

	*outBuf = reinterpret_cast<wchar_t*>(::operator new(sizeof(wchar_t) * (*outCount)));

	MultiByteToWideChar(CP_UTF8, 0, in, -1, *outBuf, size);
}

void FreeWChar(wchar_t** buf, size_t* count)
{
	::operator delete(*buf, sizeof(wchar_t) * (*count));
	*buf = nullptr;
	*count = 0;
}

//--------------------------------------------------------------------------------------------------
static bool s_DisplayErrorTaskDialog(TaskDialogProc dialogProc, const char* title, const char* expr, const char* msg,
	const char* file, uint32_t line, bool* ignoreAlways)
{
	wchar_t *titleW, *exprW, *msgW;
	size_t titleWC, exprWC, msgWC;

	ConvertToWChar(title, &titleW, &titleWC);
	ConvertToWChar(expr, &exprW, &exprWC);
	ConvertToWChar(msg, &msgW, &msgWC);

	// Just use a local buffer to combine file + line number
	wchar_t fileLineBufW[MAX_PATH + 10];
	int fileLineBufWC = swprintf_s(fileLineBufW, L"%S (%u)", file, line);

	// Assemble our message box content (file + msg)
	hrt::vector<wchar_t> contentW(size_t(fileLineBufWC) + msgWC + 2, 0);
	swprintf_s(contentW.data(), contentW.size(), L"%s\n\n%s", msgW, fileLineBufW);

	// Only show extra Debug button when a debugger is attached
	hrt::vector<TASKDIALOG_BUTTON> buttons;
	buttons.push_back({IDOK, L"Quit"});
	buttons.push_back({IDIGNORE, L"Ignore"});

	if (IsDebuggerPresent())
		buttons.push_back({IDRETRY, L"Debug"});

	TASKDIALOGCONFIG taskConfig = {};
	taskConfig.cbSize = sizeof(taskConfig);
	taskConfig.hwndParent = GetDesktopWindow();
	taskConfig.dwFlags = TDF_SIZE_TO_CONTENT;
	taskConfig.pszMainIcon = TD_ERROR_ICON;
	taskConfig.pszWindowTitle = titleW;
	taskConfig.pszMainInstruction = exprW;
	taskConfig.pszContent = contentW.data();
	taskConfig.cButtons = uint32_t(buttons.size());
	taskConfig.pButtons = buttons.data();
	taskConfig.nDefaultButton = IDOK;

	if (ignoreAlways != nullptr)
	{
		taskConfig.pszVerificationText = L"Don't show again";
	}

	int button;
	BOOL ignoreAlwaysChecked;
	(*dialogProc)(&taskConfig, &button, nullptr, &ignoreAlwaysChecked);

	FreeWChar(&titleW, &titleWC);
	FreeWChar(&exprW, &exprWC);
	FreeWChar(&msgW, &msgWC);

	// If a debugger is attached, Esc should break to the debugger, otherwise Esc should Ignore.
	if (button == IDCANCEL)
		button = IsDebuggerPresent() ? IDRETRY : IDIGNORE;

	if (button == IDOK)
		TerminateProcess(GetCurrentProcess(), 1);

	if (ignoreAlways != nullptr && ignoreAlwaysChecked)
		*ignoreAlways = true;

	return button == IDRETRY;
}

//--------------------------------------------------------------------------------------------------
bool DisplayAssertError(
	const char* title, const char* expr, const char* msg, const char* file, uint32_t line, bool* ignoreAlways)
{
	if (ignoreAlways && *ignoreAlways)
		return false;

	if (s_inside)
		return false;

	title = title ? title : "";
	expr = expr ? expr : "";
	msg = msg ? msg : "";
	file = file ? file : "";

	bool result = false;

	s_inside = true;
	AcquireSRWLockExclusive(&s_lock);

	{
		TaskDialogProcWrapper proc;
		if (proc.Load())
		{
			result = s_DisplayErrorTaskDialog(proc.procaddr, title, expr, msg, file, line, ignoreAlways);
		}
	}

	s_inside = false;
	ReleaseSRWLockExclusive(&s_lock);

	return result;
}

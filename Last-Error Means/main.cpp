#include <Windows.h>
#include <commctrl.h>

#include <sstream>

#ifdef UNICODE
#define _stringstream std::wstringstream
#else
#define _stringstream std::stringstream
#endif


DWORD g_error_code{};


LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	constexpr INT TEXT_BUFFER_SIZE{ 500 };


	switch (uMsg) {

	case WM_COMMAND: {
		WORD notifCode{ HIWORD(wParam) };
		HWND ctrlWnd{
			reinterpret_cast<HWND>(lParam)
		};

		if (notifCode == EN_CHANGE) {
			TCHAR textBuffer[TEXT_BUFFER_SIZE]{};
			GetWindowText(ctrlWnd, textBuffer, sizeof(textBuffer) / sizeof(TCHAR));
			_stringstream ss{ textBuffer };
			ss >> g_error_code;

			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}

		return 0;
	}

	case WM_CREATE: {
		RECT rc{};
		GetClientRect(hWnd, &rc);

		CreateWindow(
			TEXT("EDIT"),
			TEXT("0"),
			WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | WS_BORDER,
			rc.right / 3, rc.bottom / 9,
			rc.right / 3, rc.bottom / 8,
			hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);

		return 0;
	}

	case WM_PAINT: {
		RECT rc{};
		GetClientRect(hWnd, &rc);

		PAINTSTRUCT ps{};
		BeginPaint(hWnd, &ps);


		SetBkMode(ps.hdc, TRANSPARENT);

		CONST TCHAR szPrompt[]{ TEXT("Error Code: ") };
		TextOut(ps.hdc, rc.right / 13, rc.bottom / 8.5, szPrompt, sizeof(szPrompt) / sizeof(TCHAR));

		TCHAR textBuffer[TEXT_BUFFER_SIZE]{};
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			g_error_code,
			NULL,
			textBuffer,
			sizeof(textBuffer) / sizeof(TCHAR),
			NULL
		);

		RECT rcOutput{ rc.right / 12, rc.bottom / 2.8, rc.right - rc.right / 12, rc.bottom / 1.1 };
		Rectangle(ps.hdc, rcOutput.left, rcOutput.top, rcOutput.right, rcOutput.bottom);
		DrawText(ps.hdc, textBuffer, -1, &rcOutput, DT_LEFT | DT_WORDBREAK);


		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT) {
	SetProcessDPIAware();


	INITCOMMONCONTROLSEX icctrlex{};
	icctrlex.dwSize = sizeof(icctrlex);
	icctrlex.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icctrlex);


	CONST constexpr TCHAR* MAIN_CLASS{ TEXT("MAIN") };

	WNDCLASS wc{};
	wc.hInstance = hInstance;
	wc.lpszClassName = MAIN_CLASS;
	wc.lpfnWndProc = MainProc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_ERROR);

	RegisterClass(&wc);


	CreateWindow(
		MAIN_CLASS,
		TEXT("Last-Error Means"),
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		GetSystemMetrics(SM_CXSCREEN) / 5, GetSystemMetrics(SM_CYSCREEN) / 6,
		NULL,
		NULL,
		hInstance,
		NULL
	);


	MSG msg{};
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<INT>(msg.wParam);
}
#include <Windows.h> // 윈도우 API 함수들을 위한 정의를 포함하는 윈도우의 C 및 C++ 헤더파일

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

namespace Pipeline
{ LRESULT CALLBACK Procedure(HWND const, UINT const, WPARAM const, LPARAM const); }

int APIENTRY WinMain                                // 윈도우 프로그램의 시작점 WinMain , APIENTRY 지정자는 __stdcall 호출규약을 따름
(
	_In_	 HINSTANCE const hInstance,             // 현재 인스턴스의 핸들
	_In_opt_ HINSTANCE const prevhInstance,         // 이전 인스턴스의 핸들
	_In_	 LPSTR	   const lpCmdLine,             // 명령행으로 입력된 프로그램 인수 
	_In_	 int	   const nShowCmd               // 프로그램이 실행될 형태이며 최소화, 보통모양 등이 전달
)
{
    HWND hWnd = nullptr;

    {
        WNDCLASSEX wndClass = WNDCLASSEX();                          // 윈도우 클래스 

        wndClass.cbSize = sizeof(WNDCLASSEX);                        // 윈도우 클래스 구조체의 크기
        wndClass.lpfnWndProc = Pipeline::Procedure;                  // 윈도우의 메시지 처리 함수를 지정, 메시지가 발생할 때마다 여기서 지정한 함수가 호출 되며 이 함수가 모든 메시지 처리
        wndClass.hInstance = hInstance;                              // 윈도우 클래스를 사용하는 프로그램의 번호, 인스턴스에 대한 핸들
        wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);         // 윈도우가 사용할 아이콘 지정
        wndClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);       // 윈도우가 최소화 됬을 때 사용할 아이콘 지정
        wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);           // 윈도우가 사용할 마우스 커서 지정
        wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)); // 윈도의 배경 색상을 채색할 브러시를 지정하는 멤버
        wndClass.lpszClassName = "Window";                           // 윈도우의 클래스 이름을 정의  

        RegisterClassEx(&wndClass);                                  // 윈도우의 클래스 등록, 성공하면 등록된 윈도우 클래스를 나타내는 아톰값 리턴 
    }

    {
        CREATESTRUCT window = CREATESTRUCT();                           // 

        window.lpszClass = "Window";                                    // 윈도우 클래스 이름 지정
        window.lpszName  = "Game";                                      // 윈도우 이름 지정
        window.style     = WS_CAPTION | WS_SYSMENU;                     // 윈도우 스타일 지정
        window.hInstance = hInstance;                                   // 인스턴스 핸들
        window.cx         = 1280;                                       // 윈도우의 x축 사이즈
        window.cy         = 720;                                        // 윈도우의 y축 사이즈

        {
            RECT rect = RECT();                                         // 상하좌우값을 가지는 구조체
                            
            rect.right = window.cx;                                     // 우측끝 값을 1280으로 지정
            rect.bottom = window.cy;                                    // 하단 값을 720으로 지정

            AdjustWindowRectEx(&rect, window.style, static_cast<bool>(window.hMenu), window.dwExStyle); //

            window.cx = rect.right - rect.left;
            window.cy = rect.bottom - rect.top;

            window.x = (GetSystemMetrics(SM_CXSCREEN) - window.cx) / 2;
            window.y = (GetSystemMetrics(SM_CYSCREEN) - window.cy) / 2;
        }

        hWnd = CreateWindowEx                                   // 윈도우 할당
        (
            window.dwExStyle,                                   // 생성되는 윈도우의 스타일 지정
            window.lpszClass,                                   // 
            window.lpszName,                                    // 생성되는 윈도우의 클래스명
            window.style,
            window.x,                               
            window.y,
            window.cx,
            window.cy,
            window.hwndParent,
            window.hMenu,
            window.hInstance,
            window.lpCreateParams
        );
        
        ShowWindow(hWnd, SW_RESTORE);                               
    }

    {
        MSG msg;

        while (true)
        {
            if (PeekMessage(&msg, HWND(), WM_NULL, WM_NULL, PM_REMOVE)) // PeekMessage : GetMessage에서 메시지를 받기전에 리턴을 안하는 문제점이 있기때문에 대기하지 않는 PeekMessage를 사용, TRUE 면 메시지 있음, FALSE 면 없음.
            {
                if (msg.message == WM_QUIT)                             // WM_QUIT : 0x0012 로 define된 QUIT함수로 응용프로그램을 종료하라는 신호, PeekMessage 함수가 0 을 리턴하도록 함으로써 메시지 루프 종료
                    return static_cast<int>(msg.wParam);                // wParam : 핸들 또는 정수를 받아들일 때 사용, lParam : 포인터 값을 전달할 때 사용

                DispatchMessage(&msg);                                  // DispatchMessage : 메시지를 윈도우 프로시저로 보냄
            }
            else
            {
                SendMessage(
                    hWnd,                                               // 이 메시지를 받을 윈도우 핸들
                    WM_APP,                                             // 사용자 정의 메시지
                    0,                                                  // ?    
                    0                                                   // ?                
                );                        
            }
        }
    }
}
#include <windows.h>

#define WIDTH 200
#define HEIGHT 200

int c= 0;
char * data = NULL;
HDC hCompatibleDC = NULL;
HBITMAP hCustomBmp = NULL;

void releaseResources()
{
	if(data != NULL)delete[] data;
	if(hCompatibleDC != NULL)DeleteObject(hCompatibleDC);
	if(hCustomBmp != NULL )DeleteObject(hCustomBmp);
}

void render(HWND hwnd)
{
	HDC hdc;
	hdc = GetDC(hwnd);
	if( hCompatibleDC == NULL)
	{
		hCompatibleDC = CreateCompatibleDC(NULL);
		hCustomBmp = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT); //创建一副与当前DC兼容的位图
		SelectObject(hCompatibleDC, hCustomBmp);
	}

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = WIDTH;
	bmpInfo.bmiHeader.biHeight = -HEIGHT;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biBitCount = 32;

	if(data == NULL)
	{
		data = new char[WIDTH * HEIGHT * 4 + 1];
	}
	memset(data,0,WIDTH*HEIGHT*4); //初始化位图
	for(int i = 0 ; i < HEIGHT; i++)
	{
		c = (c+2) % 255;
		for(int j = 0; j <  WIDTH; j++)
		{
			if(i < 20 && j >= 30 && j < 60)
			{
				continue;// 全透
			}
			char * pdata = data + (i*WIDTH+j)*4;
			*pdata = 0;   //Blue
			*(pdata+1)= c; //Ggeen
			*(pdata+2)= 0; //Red
			*(pdata+3)= 150; //Alpha
			if(i < 20 && j < 30)
			{
				*(pdata+3)= 255; //Alpha,不透
			}
		}
	}
	SetDIBits(hdc, hCustomBmp, 0, HEIGHT, data, &bmpInfo, DIB_RGB_COLORS); //使用指定的DIB颜色数据来设置位图中的像素

	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;//按通道混合
	POINT	pSrc = { 0, 0 };
	SIZE	sizeWnd = { WIDTH, HEIGHT };
	UpdateLayeredWindow(hwnd, hdc, NULL, &sizeWnd, hCompatibleDC, &pSrc, NULL, &blend, ULW_ALPHA);//更新分层窗口

	ReleaseDC(hwnd, hdc);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int
	nCmdShow)
{
	TCHAR		szAppName[] = TEXT("layerwindowTest");
	WNDCLASSEX	wndClass;
	MSG			msg;
	HWND		hwnd;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground =  (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hIconSm = NULL;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = TEXT("AppTest");
	wndClass.lpszMenuName = NULL;
	wndClass.lpfnWndProc = WndProc; 
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_IME | CS_DBLCLKS;


	RegisterClassEx(&wndClass);
	hwnd = CreateWindowEx(WS_EX_LAYERED, TEXT("AppTest"), szAppName, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP/*无边框风格*/
		, 300, 500, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);
	//设置本窗口为分层窗口支持透明
	//分层窗口没有WM_PAINT消息
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if( msg.message != WM_QUIT)
		{
			render(hwnd);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL		ldown;
	static POINT	TheFirstPoint;

	switch (msg)
	{
	case WM_LBUTTONDOWN:
		ldown = TRUE;
		SetCapture(hwnd);
		TheFirstPoint.x = LOWORD(lParam);
		TheFirstPoint.y = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
		ldown = FALSE;
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		if (ldown)
		{
			POINT	pt;
			GetCursorPos(&pt);
			pt.x -= TheFirstPoint.x;
			pt.y -= TheFirstPoint.y;
			SetWindowPos(hwnd, NULL, pt.x, pt.y, NULL, NULL, SWP_NOREDRAW |
				SWP_NOSIZE | SWP_NOZORDER);
		}
		break;
	case WM_LBUTTONDBLCLK:
		DestroyWindow(hwnd);
		releaseResources();
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
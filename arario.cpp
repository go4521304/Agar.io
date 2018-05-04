#include <Windows.h>
#include <tchar.h>

#define nWitdth 800
#define nHeight 800

#define MinSize 10;	// ���ΰ��� / ���̷��� �ּ� ������
#define FoodSize 5;	// ���� ������

typedef struct PlayerCircle {

	PlayerCircle* Parent;	// �θ���

	int size;	// ������

	int x, y;	// �߽� ��ǥ

	PlayerCircle* Child1;
	PlayerCircle* Child2;	// �ڽĳ���

} Player;

Player* First;

HBRUSH	hBrush, oldBrush;
HPEN hPen, oldPen;
HDC memDC;
HBITMAP hBitmap;

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void Circle(Player*);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	// ������ Ŭ���� ����ü �� ����
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// ������ Ŭ���� ���
	RegisterClassEx(&WndClass);

	// ������ ����
	hWnd = CreateWindow
	(lpszClass,
		_T("Agar.io"),
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
		10,
		10,
		nWitdth,
		nHeight,
		NULL,
		(HMENU)NULL,
		hInstance,
		NULL);

	// ������ ���
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// �̺�Ʈ ���� ó��
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

void Circle(Player* p)
{
	if (p == NULL)
		return;

	Ellipse(memDC, p->x - p->size, p->y - p->size, p->x + p->size, p->y + p->size);

	Circle(p->Child1);
	Circle(p->Child2);
}
		
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	static int W, H;

	int i;

	switch (iMessage)
	{
	case WM_CREATE:
		W = nWitdth - 15;
		H = nHeight - 20;
		First = (Player*)malloc(sizeof(Player));
		First->Parent = NULL;
		First->Child1 = NULL;
		First->Child2 = NULL;

		First->x = W / 2 + MinSize;
		First->y = H / 2 + MinSize;
		First->size = MinSize;
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		memDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, W, H);

		SelectObject(memDC, hBitmap);
		
		hPen = CreatePen(PS_SOLID, 2, RGB(50,205,50));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(144, 238, 144));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

		Rectangle(memDC, 0, 0, W, H);

		for (i = 1; i < 10; ++i)
		{
			MoveToEx(memDC, 800 / 10 * i, 0, NULL);
			LineTo(memDC, 800 / 10 * i, H);

			MoveToEx(memDC,0, 800 / 10 * i, NULL);
			LineTo(memDC, W, 800 / 10 * i);
		}
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);
		DeleteObject(oldPen);
		DeleteObject(oldBrush);	// ��

		hPen = CreatePen(PS_SOLID, 2, RGB(255, 102, 0));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(255, 153, 0));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		Circle(First);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);
		DeleteObject(oldPen);
		DeleteObject(oldBrush);	// ���ΰ� �� ����

		BitBlt(hDC, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
		DeleteDC(memDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
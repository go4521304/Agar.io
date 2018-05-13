#include <Windows.h>
#include <time.h>
#include <tchar.h>
#include <math.h>
///dsfadsfdasfadsfadsfads

#define nWitdth 800
#define nHeight 800

#define MinSize 10	// ���ΰ��� / ���̷��� �ּ� ������
#define FoodSize 5	// ���� ������
#define FoodMax 500	// ���� �ִ� ����


/**********************
���ΰ��� / ���̷��� ����
**********************/
typedef struct PlayerCircle {

	Player* Prev;	// �������
	Player* Next;	// �������


	int size;	// ������

	int x, y;	// �߽� ��ǥ
} Player;


/************
���̷��� ����
************/
typedef struct Virus {
	Player virus;	// ���̷��� ����

	Player* Prev;	// �� ���̷���
	Player* Next;	// ���� ���̷���

} Virus;

/********
���� ����
********/
typedef struct Food {

	int x, y;	// ���� ��ǥ
	int active;	// ���� Ȱ��ȭ ����

	HPEN penColor;	// ���� �׵θ� ��
	HBRUSH brushColor;	// ���� ��

} Food;


HBRUSH	hBrush, oldBrush;
HPEN hPen, oldPen;
HDC memDC;
HBITMAP hBitmap;

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");

void Circle(Player*);	// ���ΰ� �� ���
bool EatFood(Player*, Food);	// ���̿� ���� �浹üũ

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

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
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
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


void Circle(Player* p)	// �� �׷��ֱ�
{
	if (p == NULL)
		return;

	Ellipse(memDC, p->x - p->size, p->y - p->size, p->x + p->size, p->y + p->size);

	Circle(p->Next);
}

bool EatFood(Player* p, Food f)	// ���� �ȳ�
{
	if (p == NULL)
		return false;

	int X, Y;
	double length;

	bool Check;

	X = p->x - (f.x);
	Y = p->y - (f.y);

	length = sqrt((X*X) + (Y*Y));


	if (length < p->size + FoodSize)
	{
		p->size += 2;
		return true;
	}

	Check = EatFood(p->Next, f);

	return Check;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned int)time(NULL));
	PAINTSTRUCT ps;
	HDC hDC;

	static int W, H;

	static Player* First;	// ���ΰ��� ����
	static Food food[FoodMax];	// ����
	int i;
	int r, g, b;

	switch (iMessage)
	{
	case WM_CREATE:
		W = nWitdth - 15;
		H = nHeight - 20;
		First = (Player*)malloc(sizeof(Player));
		First->Prev = NULL;
		First->Next = NULL;

		First->x = W / 2 + MinSize;
		First->y = H / 2 + MinSize;
		First->size = MinSize;

		for (i = 0; i < FoodMax; ++i)
		{
			food[i].x = rand() % W;
			food[i].y = rand() % H;

			food[i].active = (rand() % 1000)*(rand() % 100);

			r = rand() % 256;
			g = rand() % 256;
			b = rand() % 256;

			food[i].penColor = CreatePen(PS_SOLID, 1, RGB(r, g, b));
			food[i].brushColor = CreateSolidBrush(RGB(r, g, b));
		}	// ���̵��� ��ǥ, ���� ī����, ���� ����

		SetTimer(hWnd, 100, 10, NULL);

		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		memDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, W, H);

		SelectObject(memDC, hBitmap);

		hPen = CreatePen(PS_SOLID, 2, RGB(50, 205, 50));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(144, 238, 144));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		Rectangle(memDC, 0, 0, W, H);
		for (i = 1; i < 10; ++i)
		{
			MoveToEx(memDC, 800 / 10 * i, 0, NULL);
			LineTo(memDC, 800 / 10 * i, H);

			MoveToEx(memDC, 0, 800 / 10 * i, NULL);
			LineTo(memDC, W, 800 / 10 * i);
		}
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// �� ����


		for (i = 0; i < FoodMax; ++i)
		{
			if (food[i].active == 0)
			{
				oldPen = (HPEN)SelectObject(memDC, food[i].penColor);
				oldBrush = (HBRUSH)SelectObject(memDC, food[i].brushColor);
				Ellipse(memDC, food[i].x - FoodSize, food[i].y - FoodSize, food[i].x + FoodSize, food[i].y + FoodSize);
				SelectObject(memDC, oldPen);
				SelectObject(memDC, oldBrush);
			}
		}	// ���� ����


		hPen = CreatePen(PS_SOLID, 2, RGB(255, 102, 0));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(255, 153, 0));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		Circle(First);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// ���ΰ� �� ����

		BitBlt(hDC, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hBitmap);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);	// ����Ѱ͵� ����
		break;

	case WM_TIMER:
		if (wParam == 100)
		{
			for (i = 0; i < FoodMax; ++i)
			{
				if (food[i].active != 0)	// ���̰� Ȱ��ȭ�� �ȵ� ���¸� Ȱ��ȭ ī��Ʈ -1
					food[i].active--;

				else if (EatFood(First, food[i]))	// Ȱ��ȭ�� �����̰� �� ���� ������ �ʱ�ȭ
				{
					food[i].x = rand() % W;
					food[i].y = rand() % H;

					food[i].active = (rand() % 1000)*(rand() % 100);

					r = rand() % 256;
					g = rand() % 256;
					b = rand() % 256;

					food[i].penColor = CreatePen(PS_SOLID, 1, RGB(r, g, b));
					food[i].brushColor = CreateSolidBrush(RGB(r, g, b));
				}
			}
			InvalidateRect(hWnd, NULL, false);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
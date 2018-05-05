#include <Windows.h>
#include <time.h>
#include <tchar.h>
#include <math.h>

#define nWitdth 800
#define nHeight 800

#define MinSize 10	// 주인공원 / 바이러스 최소 사이즈
#define FoodSize 5	// 먹이 사이즈
#define FoodMax 500	// 먹이 최대 갯수


/**********************
주인공원 / 바이러스 정보
**********************/
typedef struct PlayerCircle {

	PlayerCircle* Parent;	// 부모노드

	int size;	// 반지름

	int x, y;	// 중심 좌표
	int xTo, yTo;	// 중심으로부터 멀어진 거리

	int Dead;	// 중심원으로 다시 돌아가야 할 시간

	PlayerCircle* Child1;
	PlayerCircle* Child2;	// 자식노드들

} Player;


/************
바이러스 관리
************/
typedef struct Virus {
	Player virus;	// 바이러스 정보

	Player* Prev;	// 전 바이러스
	Player* Next;	// 다음 바이러스

} Virus;

/********
먹이 정보
********/
typedef struct Food {

	int x, y;	// 먹이 좌표
	int active;	// 먹이 활성화 여부

	HPEN penColor;	// 먹이 테두리 색
	HBRUSH brushColor;	// 먹이 색

} Food;


HBRUSH	hBrush, oldBrush;
HPEN hPen, oldPen;
HDC memDC;
HBITMAP hBitmap;

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");

void Circle(Player*);	// 주인공 원 출력
void CircleMove(Player*, int, int);		// 주인공원 이동
bool EatFood(Player*, Food);	// 먹이를 먹었는지 확인

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	// 윈도우 클래스 구조체 값 설정
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

	// 윈도우 클래스 등록
	RegisterClassEx(&WndClass);

	// 윈도우 생성
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

	// 윈도우 출력
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 이벤트 루프 처리
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

	Ellipse(memDC, p->x - p->size + p->xTo, p->y - p->size + p->yTo, p->x + p->size + p->xTo, p->y + p->size + p->yTo);

	Circle(p->Child1);
	Circle(p->Child2);
}

void CircleMove(Player* p, int x, int y)	// ★좌표에 관해서 수정이 필요
{
	if (p == NULL)
		return;

	int tmp;
	tmp = x - (p->x);
	tmp = tmp / ((p->size)+2);
	p->x += tmp;

	tmp = y - (p->y);
	tmp = tmp / ((p->size)+2);
	p->y += tmp;

	CircleMove(p->Child1, x, y);
	CircleMove(p->Child2, x, y);
}

bool EatFood(Player* p, Food f)
{
	if (p == NULL)
		return false;

	int X, Y;
	double length;

	bool child1;
	bool child2;

	X = (p->x + p->xTo) - (f.x);
	Y = (p->y + p->yTo) - (f.y);

	length = sqrt((X*X) + (Y*Y));


	if (length < p->size + FoodSize)
	{
		p->size += 2;
		return true;
	}

	child1 = EatFood(p->Child1, f);
	child2 = EatFood(p->Child2, f);

	return (child1 || child2);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned int)time(NULL));
	PAINTSTRUCT ps;
	HDC hDC;

	static int W, H;

	static Player* First;	// 주인공원 생성
	static Food food[FoodMax];	// 먹이

	int i;
	int r, g, b;
	int x, y;

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
		First->xTo = 0;
		First->yTo = 0;

		First->Dead = -1;	// 첫번째원은 불멸

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
		}	// 먹이들의 좌표, 생성 카운터, 색상 결정

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
		SelectObject(memDC, oldBrush);	// 판 생성


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
		}	// 먹이 생성


		hPen = CreatePen(PS_SOLID, 2, RGB(255, 102, 0));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(255, 153, 0));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		Circle(First);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// 주인공 원 생성

		BitBlt(hDC, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hBitmap);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);	// 사용한것들 삭제
		break;

	case WM_TIMER:
		if (wParam == 100)
		{
			for (i = 0; i < FoodMax; ++i)
			{
				if (food[i].active != 0)
					food[i].active--;

				else if (EatFood(First, food[i]))	// 먹이를 먹었으면 먹이 초기화
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

	case WM_MOUSEMOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);

		CircleMove(First, x, y);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
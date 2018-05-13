#include <Windows.h>
#include <time.h>
#include <tchar.h>
#include <math.h>
///dsfadsfdasfadsfadsfads

#define nWidth 800
#define nHeight 800

#define MinSize 10	// 주인공원 / 바이러스 최소 사이즈
#define FoodSize 5	// 먹이 사이즈
#define FoodMax 500	// 먹이 최대 갯수
#define LifeTime 70	// 살아있는 시간


/**********************
주인공원 / 바이러스 정보
**********************/
typedef struct PlayerCircle {

	PlayerCircle* Prev;	// 이전노드
	PlayerCircle* Next;	// 다음노드

	int size;	// 반지름

	int x, y;	// 중심 좌표

	int count;	// 살아있는 시간

} Player;


/************
바이러스 관리
************/
typedef struct Virus {

	Player* virus;	// 바이러스 정보

	int count;		// 바이러스 생성 카운터

	int index;				// 타켓의 번호
	int TargetX, TargetY;	// 타겟의 X, Y

	Virus* Prev;	// 전 바이러스
	Virus* Next;	// 다음 바이러스

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
		WS_POPUPWINDOW,//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		0,
		0,
		nWidth,
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

void Circle(Player*);				// 주인공 원 출력
bool EatFood(Player*, Food);		// 먹이와 원의 충돌체크
void CircleMove(Player*, int, int);	// 원들의 이동
void CircleBreak(Player*, int);		// 원 분열
void CircleCombine(Player*);		// 원 합쳐짐

void VCircle(Virus*);				// 바이러스 출력
void VirusMake(Virus*);				// 바이러스 만들기
void VirusMove(Virus*);				// 바이러스 움직이기
bool VEatFood(Virus*, Food);

void FindTarget(Virus*, Food, int);			// 바이러스 목적지 찾기

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned int)time(NULL));
	PAINTSTRUCT ps;
	HDC hDC;

	static int W, H;

	static Player* First;	// 주인공원 생성
	static Food food[FoodMax];	// 먹이

	static Virus* VFirst;	// 바이러스 생성

	int i;
	int r, g, b;
	static int x, y;

	static RECT rect;
	GetClientRect(hWnd, &rect);
	rect.left += 2;
	rect.top += 2;
	ClipCursor(&rect);

	switch (iMessage)
	{
	case WM_CREATE:
		W = nWidth;
		H = nHeight;
		First = (Player*)malloc(sizeof(Player));
		First->Prev = NULL;
		First->Next = NULL;
		First->x = W / 2 + MinSize;
		First->y = H / 2 + MinSize;
		First->size = MinSize;
		First->count = 0;

		VFirst = (Virus*)malloc(sizeof(Virus));
		VFirst->Prev = NULL;
		VFirst->Next = NULL;
		VFirst->virus = NULL;
		VFirst->count = rand() % 50;

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
		DeleteObject(hPen);
		DeleteObject(hBrush);
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
		DeleteObject(hPen);
		DeleteObject(hBrush);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// 주인공 원 생성

		hPen = CreatePen(PS_SOLID, 2, RGB(255-50, 255-205, 255-50));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(255-144, 255-238, 255-144));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		VCircle(VFirst);
		DeleteObject(hPen);
		DeleteObject(hBrush);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// 바이러스 생성


		SetBkMode(memDC, 1);
		TextOut(memDC, 0, 0, _T("Press ESC is Exit"), _tcsclen(_T("Press ESC is Exit")));

		BitBlt(hDC, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

		DeleteObject(hBitmap);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);	// 사용한것들 삭제
		break;

	case WM_TIMER:
		if (wParam == 100)
		{
			CircleCombine(First);
			VirusMake(VFirst);

			for (i = 0; i < FoodMax; ++i)
			{
				if (food[i].active != 0)	// 먹이가 활성화가 안된 상태면 활성화 카운트 -1
					food[i].active--;

				else if (EatFood(First, food[i]))	// 활성화된 먹이이고 원 한태 먹히면 초기화
				{
					food[i].x = rand() % W;
					food[i].y = rand() % H;

					food[i].active = (rand() % 1000)*(rand() % 100);

					DeleteObject(food[i].brushColor);
					DeleteObject(food[i].penColor);

					r = rand() % 256;
					g = rand() % 256;
					b = rand() % 256;

					food[i].penColor = CreatePen(PS_SOLID, 1, RGB(r, g, b));
					food[i].brushColor = CreateSolidBrush(RGB(r, g, b));
				}

				if (food[i].active == 0)	// 바이러스가 먹으러갈 먹이 찾기
				{
					if (VEatFood(VFirst, food[i]))
					{
						food[i].x = rand() % W;
						food[i].y = rand() % H;

						food[i].active = (rand() % 1000)*(rand() % 100);

						DeleteObject(food[i].brushColor);
						DeleteObject(food[i].penColor);

						r = rand() % 256;
						g = rand() % 256;
						b = rand() % 256;

						food[i].penColor = CreatePen(PS_SOLID, 1, RGB(r, g, b));
						food[i].brushColor = CreateSolidBrush(RGB(r, g, b));
					}
					FindTarget(VFirst, food[i], i);
				}
			}

			CircleMove(First, x, y);
			VirusMove(VFirst);

			InvalidateRect(hWnd, NULL, false);
		}
		break;

	case WM_MOUSEMOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;

	case WM_LBUTTONDOWN:
		if (First->size < MinSize*2)
			break;
		CircleBreak(First, 0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			ClipCursor(NULL);
			PostQuitMessage(0);
			return 0;
		}
		break;

	case WM_DESTROY:
		ClipCursor(NULL);
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}


void Circle(Player* p)		// 원 그려주기
{
	if (p == NULL)
		return;

	Ellipse(memDC, p->x - p->size, p->y - p->size, p->x + p->size, p->y + p->size);

	Circle(p->Next);
}

bool EatFood(Player* p, Food f)		// 먹이 냠냠
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

void CircleMove(Player* p, int x, int y)	// 원 슈슈슉~
{
	if (p == NULL)
		return;

	srand((unsigned int)time(NULL));
	int tmp1, tmp2;

	tmp1 = x - p->x;
	tmp2 = y - p->y;

	p->x = p->x + (tmp1 / p->size);
	p->y = p->y + (tmp2 / p->size);

	CircleMove(p->Next, x, y);
}

void CircleBreak(Player* p, int active)		// 원 쨍그랑!
{
	if (p == NULL)
		return;

	srand((unsigned int)time(NULL));
	bool finish;

	if (active == 0 && p->size > MinSize*2)
	{
		Player* node = (Player*)malloc(sizeof(Player));

		if (p->Next == NULL)
		{
			finish = true;
			node->Next = NULL;
		}
		else
		{
			finish = false;
			node->Next = p->Next;
		}
		p->Next = node;

		node->Prev = p;
		node->size = p->size / 2;
		node->x = p->x;
		node->y = p->y;
		node->count = LifeTime;


		p->size /= 2;
		if (rand() % 2 == 0)
			p->x = p->x + (rand() % (p->size * 2) + (p->size * 2));
		else
			p->x = p->x - (rand() % (p->size * 2) + (p->size * 2));
		
		if (rand() % 2 == 1)
			p->y = p->y + (rand() % (p->size * 2) + (p->size * 2));
		else
			p->y = p->y - (rand() % (p->size * 2) + (p->size * 2));


		p->count += LifeTime;

		if (finish == false)
			CircleBreak(p->Next, 1);
	}

	else
		CircleBreak(p->Next, 0);
}

void CircleCombine(Player* p)
{
	if (p == NULL)
		return;

	Player* tmp = p;

	if (p->Prev == NULL);

	else if (p->count == 0)
	{
		tmp = p->Prev;

		p->Prev->size += p->size;
		p->Prev->Next = p->Next;
		free(p);
	}

	else
		p->count--;

	CircleCombine(tmp->Next);
}

void VCircle(Virus* vp)
{
	if (vp == NULL || vp->count != 0 || vp->virus == NULL)
		return;

	if (vp->count == 0)
	{
		Circle(vp->virus);
	}

	VCircle(vp->Next);
}

void VirusMake(Virus* vp)
{
	if (vp == NULL)
		return;

	if (vp->count == 0)
	{
		if (vp->virus == NULL)
		{
			vp->virus = (Player*)malloc(sizeof(Player));
			vp->virus->Prev = NULL;
			vp->virus->Next = NULL;
			vp->virus->count = 0;
			vp->virus->size = MinSize;
			vp->virus->x = rand() % nWidth;
			vp->virus->y = rand() % nHeight;
			vp->index = -1;
			vp->TargetX = 0;
			vp->TargetY = 0;

			vp->Next = (Virus*)malloc(sizeof(Virus));
			vp->Next->Prev = vp;
			vp->Next->Next = NULL;
			vp->Next->virus = NULL;
			vp->Next->count = (rand() % 100)*(rand() % 100);
		}
	}
	else
		vp->count--;

	VirusMake(vp->Next);
}

void VirusMove(Virus* vp)
{
	if (vp == NULL || vp->count != 0 || vp->virus == NULL)
		return;

	CircleMove(vp->virus, vp->TargetX, vp->TargetY);

	VirusMove(vp->Next);
}

void FindTarget(Virus* vp, Food f, int i)
{
	if (vp == NULL || vp->count != 0 || vp->virus == NULL)
		return;

	else if (vp->index == -1)
	{
		vp->TargetX = f.x;
		vp->TargetY = f.y;
		vp->index = i;
	}

	else if (vp->index == i && (vp->TargetX != f.x || vp->TargetY != f.y))
	{
		vp->TargetX = f.x;
		vp->TargetY = f.y;
		vp->index = i;
	}

	else
	{
		if (sqrt((abs((vp->virus->x) - (vp->TargetX))*abs((vp->virus->x) - (vp->TargetX))) + (abs((vp->virus->y) - (vp->TargetY))*abs((vp->virus->y) - (vp->TargetY))))
			 > sqrt((abs((vp->virus->x) - (f.x))*abs((vp->virus->x) - (f.x))) + (abs((vp->virus->y) - (f.y))*abs((vp->virus->y) - (f.y)))))
		{
			vp->TargetX = f.x;
			vp->TargetY = f.y;
			vp->index = i;
		}
	}

	FindTarget(vp->Next, f, i);
}

bool VEatFood(Virus* vp, Food f)
{
	bool check;

	if (vp == NULL || vp->count != 0 || vp->virus == NULL)
		return false;

	else if (EatFood(vp->virus, f))
	{
		vp->virus->size += 2;
		return true;
	}

	check = VEatFood(vp->Next, f);
	return check;
}
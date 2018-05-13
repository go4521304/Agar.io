#include <Windows.h>
#include <time.h>
#include <tchar.h>
#include <math.h>
///dsfadsfdasfadsfadsfads

#define nWidth 800
#define nHeight 800

#define MinSize 10	// ���ΰ��� / ���̷��� �ּ� ������
#define FoodSize 5	// ���� ������
#define FoodMax 500	// ���� �ִ� ����
#define LifeTime 70	// ����ִ� �ð�


/**********************
���ΰ��� / ���̷��� ����
**********************/
typedef struct PlayerCircle {

	PlayerCircle* Prev;	// �������
	PlayerCircle* Next;	// �������

	int size;	// ������

	int x, y;	// �߽� ��ǥ

	int count;	// ����ִ� �ð�

} Player;


/************
���̷��� ����
************/
typedef struct Virus {

	Player* virus;	// ���̷��� ����

	int count;		// ���̷��� ���� ī����

	int index;				// Ÿ���� ��ȣ
	int TargetX, TargetY;	// Ÿ���� X, Y

	Virus* Prev;	// �� ���̷���
	Virus* Next;	// ���� ���̷���

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
		WS_POPUPWINDOW,//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		0,
		0,
		nWidth,
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

void Circle(Player*);				// ���ΰ� �� ���
bool EatFood(Player*, Food);		// ���̿� ���� �浹üũ
void CircleMove(Player*, int, int);	// ������ �̵�
void CircleBreak(Player*, int);		// �� �п�
void CircleCombine(Player*);		// �� ������

void VCircle(Virus*);				// ���̷��� ���
void VirusMake(Virus*);				// ���̷��� �����
void VirusMove(Virus*);				// ���̷��� �����̱�
bool VEatFood(Virus*, Food);

void FindTarget(Virus*, Food, int);			// ���̷��� ������ ã��

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned int)time(NULL));
	PAINTSTRUCT ps;
	HDC hDC;

	static int W, H;

	static Player* First;	// ���ΰ��� ����
	static Food food[FoodMax];	// ����

	static Virus* VFirst;	// ���̷��� ����

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
		DeleteObject(hPen);
		DeleteObject(hBrush);
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
		DeleteObject(hPen);
		DeleteObject(hBrush);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// ���ΰ� �� ����

		hPen = CreatePen(PS_SOLID, 2, RGB(255-50, 255-205, 255-50));
		oldPen = (HPEN)SelectObject(memDC, hPen);
		hBrush = CreateSolidBrush(RGB(255-144, 255-238, 255-144));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		VCircle(VFirst);
		DeleteObject(hPen);
		DeleteObject(hBrush);
		SelectObject(memDC, oldPen);
		SelectObject(memDC, oldBrush);	// ���̷��� ����


		SetBkMode(memDC, 1);
		TextOut(memDC, 0, 0, _T("Press ESC is Exit"), _tcsclen(_T("Press ESC is Exit")));

		BitBlt(hDC, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

		DeleteObject(hBitmap);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);	// ����Ѱ͵� ����
		break;

	case WM_TIMER:
		if (wParam == 100)
		{
			CircleCombine(First);
			VirusMake(VFirst);

			for (i = 0; i < FoodMax; ++i)
			{
				if (food[i].active != 0)	// ���̰� Ȱ��ȭ�� �ȵ� ���¸� Ȱ��ȭ ī��Ʈ -1
					food[i].active--;

				else if (EatFood(First, food[i]))	// Ȱ��ȭ�� �����̰� �� ���� ������ �ʱ�ȭ
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

				if (food[i].active == 0)	// ���̷����� �������� ���� ã��
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


void Circle(Player* p)		// �� �׷��ֱ�
{
	if (p == NULL)
		return;

	Ellipse(memDC, p->x - p->size, p->y - p->size, p->x + p->size, p->y + p->size);

	Circle(p->Next);
}

bool EatFood(Player* p, Food f)		// ���� �ȳ�
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

void CircleMove(Player* p, int x, int y)	// �� ������~
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

void CircleBreak(Player* p, int active)		// �� ¸�׶�!
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
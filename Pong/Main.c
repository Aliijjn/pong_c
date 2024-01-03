#include <Windows.h>
#include <stdio.h>
#include <math.h>
#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#define BAT_WIDTH 48
#define BAT_SPACING (BAT_WIDTH / 2 + 64)
#define BAT_HEIGHT 256
#define BAT_VEL 3
#define BALL_SIZE 48
#define LIVES 3
#define FRIC 0.9
#define CARRYOVER 0.5
#define BACKCOLOUR (pixel){ 220, 240, 30 }

BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER),SCR_WIDTH,SCR_HEIGHT,1,24,BI_RGB };
HDC windowDC;

typedef struct {
    unsigned char b, g, r;
}pixel;

pixel* grid;

HWND window;

pixel vram[SCR_HEIGHT][SCR_WIDTH];
int windowMessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSA window_class = { .lpfnWndProc = windowMessageHandler,.lpszClassName = "class",.lpszMenuName = "class" };

int ballposx;
int ballposy;
int balldirx;
int balldiry;
int batpos[] = { SCR_HEIGHT / 2, SCR_HEIGHT / 2 };
unsigned int score[] = { 0, 0 };

int windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CLOSE:
	case WM_QUIT:
		ExitProcess(0);
	}
	return DefWindowProcA(window, msg, wParam, lParam);
}

int tRand() {
	unsigned int x = __rdtsc();
	x >>= 3;
	x ^= 7;
	x <<= 4;
	x %= 5;
	return x;
}

void ballinit()
{
	ballposx = SCR_WIDTH / 2;
	ballposy = SCR_HEIGHT / 2;
	balldirx = tRand() + 5;
	balldiry = 12 - balldirx;
	balldirx *= -1;
}

void startgame(int* batpos, pixel *grid)
{
	batpos[0] = SCR_HEIGHT / 2;
	batpos[1] = SCR_HEIGHT / 2;
	for (int i = 0; i < SCR_HEIGHT * SCR_WIDTH; i++)
	{
		grid[i] = BACKCOLOUR;
	}
	ballinit();
}

void drawrectangle(int x, int y, int width, int height, pixel colour)
{
	x -= width / 2;
	y -= height / 2;
	int tempheight = height;
	width += x;
	height += y;
	for (; x < width; x++)
	{
		for (; y < height; y++)
		{
			grid[x + y * SCR_WIDTH] = colour;
		}
		y -= tempheight;
	}
}

void drawcircle(int x, int y, int width, int height, pixel colour)
{
	float radius;
	int tempx = x;
	int tempy = y;
	x -= width / 2;
	y -= height / 2;
	int tempheight = height;
	width += x;
	height += y;
	for (; x < width; x++)
	{
		for (; y < height; y++)
		{
			radius = sqrt((x - tempx) * (x - tempx) + (y - tempy) * (y - tempy));
			if (radius < BALL_SIZE / 2)
			{
				grid[x + y * SCR_WIDTH] = colour;
			}
		}
		y -= tempheight;
	}
}

void logic()
{
	float batleftspeed = 0.0f;
	float batrightspeed = 0.0f;
	startgame(batpos, grid);
	while (score[0] < LIVES && score[1] < LIVES)
	{
		//game logic here
		if (GetKeyState('W') & 128)
		{
			batleftspeed += BAT_VEL;
		}
		if (GetKeyState('S') & 128)
		{
			batleftspeed -= BAT_VEL;
		}
		if (GetKeyState(VK_UP) & 128)
		{
			batrightspeed += BAT_VEL;
		}
		if (GetKeyState(VK_DOWN) & 128)
		{
			batrightspeed -= BAT_VEL;
		}
		batleftspeed *= FRIC;
		batrightspeed *= FRIC;
		batpos[0] += batleftspeed;
		batpos[1] += batrightspeed;
		if (batpos[0] < BAT_HEIGHT / 2)
			batpos[0] = BAT_HEIGHT / 2;
		if (batpos[0] > SCR_HEIGHT - BAT_HEIGHT / 2)
			batpos[0] = SCR_HEIGHT - BAT_HEIGHT / 2;
		if (batpos[1] < BAT_HEIGHT / 2)
			batpos[1] = BAT_HEIGHT / 2;
		if (batpos[1] > SCR_HEIGHT - BAT_HEIGHT / 2)
			batpos[1] = SCR_HEIGHT - BAT_HEIGHT / 2;

		ballposx += balldirx;
		ballposy += balldiry;
		if (ballposy < BALL_SIZE / 2)
		{
			balldiry *= -1;
			ballposy = BALL_SIZE / 2;
			balldiry *= 1 - CARRYOVER / 5;
		}
		else if (ballposy > SCR_HEIGHT - BALL_SIZE / 2)
		{
			balldiry *= -1;
			ballposy = SCR_HEIGHT - BALL_SIZE / 2;
			balldiry *=  1 - CARRYOVER / 5;
		}

		if (ballposx < BALL_SIZE / 2)
		{
			score[1]++;
			ballinit();
		}
		else if (ballposx > SCR_WIDTH - BALL_SIZE / 2)
		{
			score[0]++;
			ballinit();
		}

		if (ballposx < BAT_SPACING + BAT_WIDTH / 2 + BALL_SIZE / 2
			&& ballposx > BAT_SPACING
			&& ballposy - BALL_SIZE / 2 > batpos[0] - BAT_HEIGHT
			&& ballposy + BALL_SIZE / 2 < batpos[0] + BAT_HEIGHT)
		{
			ballposx = BAT_SPACING + BAT_WIDTH / 2 + BALL_SIZE / 2;
			balldirx *= -1;
			balldirx++;
			balldiry += batleftspeed * CARRYOVER;
		}
		else if (ballposx > SCR_WIDTH - BAT_SPACING - BAT_WIDTH / 2 - BALL_SIZE / 2
			&& ballposx < SCR_WIDTH - BAT_SPACING
			&& ballposy - BALL_SIZE / 2 > batpos[1] - BAT_HEIGHT
			&& ballposy + BALL_SIZE / 2 < batpos[1] + BAT_HEIGHT)
		{
			ballposx = SCR_WIDTH - BAT_SPACING - BAT_WIDTH / 2 - BALL_SIZE / 2;
			balldirx *= -1;
			balldirx--;
			balldiry += batrightspeed * CARRYOVER;
		}
		Sleep(16);
	}
	Sleep(1500);
	ExitProcess(0);
}

void draw()
{
	StretchDIBits(windowDC, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, grid, &bmi, 0, SRCCOPY);

	while (1)
	{
		for (int heartleft = LIVES - score[1]; heartleft > 0; heartleft--)
		{
			drawrectangle(SCR_WIDTH / 8 + heartleft * BALL_SIZE * 3, SCR_HEIGHT - BAT_SPACING - BALL_SIZE, BALL_SIZE * 2, BALL_SIZE * 2, (pixel) { 0, 0, 255 });
		}
		for (int heartleft = LIVES - score[0]; heartleft > 0; heartleft--)
		{
			drawrectangle(SCR_WIDTH / 8 * 7 - heartleft * BALL_SIZE * 3, SCR_HEIGHT - BAT_SPACING - BALL_SIZE, BALL_SIZE * 2, BALL_SIZE * 2, (pixel) { 0, 0, 255 });
		}
		drawrectangle(BAT_SPACING,				batpos[0], BAT_WIDTH, BAT_HEIGHT, (pixel) { 30, 100, 230 });
		drawrectangle(SCR_WIDTH - BAT_SPACING,	batpos[1], BAT_WIDTH, BAT_HEIGHT, (pixel) { 0, 190, 0 });
		drawcircle(ballposx, ballposy, BALL_SIZE, BALL_SIZE, (pixel) { 255, 255, 255 }); //use __rdtsc() for rainbow ball
		StretchDIBits(windowDC, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, grid, &bmi, 0, SRCCOPY);
		
		for (int i = 0; i < SCR_HEIGHT * SCR_WIDTH; i++)
		{
			grid[i] = BACKCOLOUR;
		}
		Sleep(2);
	}
}

void main() {
    RegisterClassA(&window_class);
    window = CreateWindowExA(0, "class", "Pong", WS_VISIBLE | WS_POPUP, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, window_class.hInstance, 0);
#pragma comment(lib,"Winmm.lib")
	timeBeginPeriod(1);
	grid = malloc(SCR_WIDTH * SCR_HEIGHT * sizeof(pixel));
	windowDC = GetDC(window);
	CreateThread(0, 0, draw, 0, 0, 0);
	CreateThread(0, 0, logic, 0, 0, 0);
    MSG message;

    while (GetMessageA(&message, window, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);		
    }
}
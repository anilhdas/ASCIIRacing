#include <windows.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <vector>

using namespace std;

enum class GameObject
{
	None,
	Road,
	Hurdle,
	Player
};

const WORD COLOR_ROAD = FOREGROUND_RED | FOREGROUND_GREEN;
const WORD COLOR_HURDLE = FOREGROUND_RED | FOREGROUND_INTENSITY;
const WORD COLOR_PLAYER = FOREGROUND_BLUE | FOREGROUND_GREEN;
const WORD COLOR_TEXT = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const WORD COLOR_WHITE = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

const static short ROAD_CHAR_ASCII = 124;
const static short GAMELOOP_DELAY_IN_MS = 75;
const static short ROAD_WIDTH = 20, ROAD_LENGTH = 20;
const static short ROAD_X_OFFSET = 15, ROAD_Y_OFFSET = 3;

bool isGameOver = FALSE;
HANDLE OutStdConsole;

GameObject GameScreen[ROAD_WIDTH][ROAD_LENGTH] = { GameObject::None };

short HurdleDelay = 250;
short Input;
COORD PlayerPos;
COORD Hurdle1Pos, Hurdle2Pos;
long long Score;

void Hidecursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void Reset()
{
	// Reset Road
	for (int i = 0; i < ROAD_WIDTH; ++i)
	{
		for (int j = 0; j < ROAD_LENGTH; ++j)
		{
			if (i == 0 || i == ROAD_WIDTH - 1)
				GameScreen[i][j] = GameObject::Road;
			else
				GameScreen[i][j] = GameObject::None;
		}
	}

	// Reset Player
	PlayerPos.X = 8;
	PlayerPos.Y = (ROAD_LENGTH - 2);
	GameScreen[PlayerPos.X][PlayerPos.Y] = GameObject::Player;

	// Reset Hurdles
	Hurdle1Pos.X = 1;
	Hurdle1Pos.Y = 0;

	Hurdle2Pos.X = 11;
	Hurdle2Pos.Y = 10;

	// Reset Input
	Input = 0;

	OutStdConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

void PrintInstructions()
{
	SetConsoleTextAttribute(OutStdConsole, COLOR_WHITE);
	cout << endl << "!! CONSOLE CAR DASH !!" << endl << endl << endl;

	cout << "How to Play.." << endl << endl;

	cout << "1. Car (";
	SetConsoleTextAttribute(OutStdConsole, COLOR_PLAYER);
	cout << " # ";
	SetConsoleTextAttribute(OutStdConsole, COLOR_WHITE);
	cout << ") will automatically move forward." << endl;
	cout << "2. Use <- arrow -> keys to steer the car." << endl;
	cout << "3. Do not hit the boundary (";
	SetConsoleTextAttribute(OutStdConsole, COLOR_ROAD);
	cout << " | ";
	SetConsoleTextAttribute(OutStdConsole, COLOR_WHITE);
	cout << ") or hurdles (";
	SetConsoleTextAttribute(OutStdConsole, COLOR_HURDLE);
	cout << " === ";
	SetConsoleTextAttribute(OutStdConsole, COLOR_WHITE);
	cout << ") while driving." << endl;
	cout << "4. Car will progressively become faster." << endl;
	cout << "5. Have fun !" << endl << endl;
}

void PrintInfo()
{
	TCHAR Username[50];
	DWORD Username_len = 50;
	GetUserName(Username, &Username_len);

	COORD c;
	c.X = 0;
	c.Y = 0;
	SetConsoleCursorPosition(OutStdConsole, c);
	SetConsoleTextAttribute(OutStdConsole, COLOR_TEXT);
	cout << "Username: " << Username << endl;
	cout << "Score: " << Score << endl;
}

void UpdateInput()
{
	while (!isGameOver)
	{
		if (GetAsyncKeyState(VK_LEFT) & (0x8000 != 0))
			Input = -1;
		else if (GetAsyncKeyState(VK_RIGHT) & (0x8000 != 0))
			Input = 1;
	}
}

bool HasCollidedWithRoad()
{
	COORD pos;
	pos.X = PlayerPos.X + ROAD_X_OFFSET;
	pos.Y = PlayerPos.Y + ROAD_Y_OFFSET;

	TCHAR c;
	DWORD dwChars;
	ReadConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), &c, 1, pos, &dwChars);
	return (c == ROAD_CHAR_ASCII);
}

bool HasCollidedWithHurdle()
{
	if (Hurdle1Pos.Y == PlayerPos.Y)
	{
		for (short i = 0; i < 3; ++i)
			if (Hurdle1Pos.X + i == PlayerPos.X)
				isGameOver = true;
	}
	else if (Hurdle2Pos.Y == PlayerPos.Y)
	{
		for (short i = 0; i < 3; ++i)
			if (Hurdle2Pos.X + i == PlayerPos.X)
				isGameOver = true;
	}

	return isGameOver;
}

void UpdateRoad()
{
	// Fix road update to be sin wave function

	const short maxRoadCurve = 5;

	for (short i = 0; i < ROAD_LENGTH; ++i)
	{
		for (short j = 0; j < ROAD_WIDTH; ++j)
		{
			if (GameScreen[i][j] == GameObject::Road)
			{
				GameScreen[i][j] = GameObject::None;

				short newX = (i + 1) % maxRoadCurve;
				short newY = (j + 1) % ROAD_WIDTH;
				GameScreen[i][newY] = GameObject::Road;
			}
		}
	}
}

void SwitchHurdles()
{
	if (Hurdle1Pos.Y == 1)
		Hurdle1Pos.X = (Hurdle1Pos.X + 3) % (ROAD_WIDTH - 4) + 1;
	else if (Hurdle2Pos.Y == 1)
		Hurdle2Pos.X = (PlayerPos.X - 1) % (ROAD_WIDTH - 4) + 1;
}

void UpdateHurdles()
{
	while (!isGameOver)
	{
		// Erase hurdles from last frame
		for (short i = 0; i < 3; ++i)
		{
			GameScreen[Hurdle1Pos.X + i][Hurdle1Pos.Y] = GameObject::None;
			GameScreen[Hurdle2Pos.X + i][Hurdle2Pos.Y] = GameObject::None;
		}

		Hurdle1Pos.Y = (Hurdle1Pos.Y++) % (ROAD_LENGTH - 1);
		Hurdle2Pos.Y = (Hurdle2Pos.Y++) % (ROAD_LENGTH - 1);

		SwitchHurdles();

		// Draw hurdles for this frame
		for (short i = 0; i < 3; ++i)
		{
			GameScreen[Hurdle1Pos.X + i][Hurdle1Pos.Y] = GameObject::Hurdle;
			GameScreen[Hurdle2Pos.X + i][Hurdle2Pos.Y] = GameObject::Hurdle;
		}

		Sleep(HurdleDelay);

		if (++Score % 10 == 0 && HurdleDelay > 50)
			HurdleDelay -= 5;
	}
}

void UpdatePlayer()
{
	if (Input != 0)
	{
		// Remove old player character
		GameScreen[PlayerPos.X][PlayerPos.Y] = GameObject::None;

		// Update position
		PlayerPos.X += Input;
		Input = 0;

		// Check Collision with Road
		if (HasCollidedWithRoad() || HasCollidedWithHurdle())
		{
			isGameOver = true;
		}

		// Add player to the new position
		GameScreen[PlayerPos.X][PlayerPos.Y] = GameObject::Player;
	}
}

void PrintObj(GameObject go)
{
	switch (go)
	{
	case GameObject::None:
		cout << " ";
		break;
	case GameObject::Road:
		SetConsoleTextAttribute(OutStdConsole, COLOR_ROAD);
		cout << "|";
		break;
	case GameObject::Hurdle:
		SetConsoleTextAttribute(OutStdConsole, COLOR_HURDLE);
		cout << "=";
		break;
	case GameObject::Player:
		SetConsoleTextAttribute(OutStdConsole, COLOR_PLAYER);
		cout << "#";
		break;
	}
	cout << flush;
}

void UpdateRender()
{
	COORD c;
	for (int i = 0; i < ROAD_WIDTH; ++i)
	{
		for (int j = 0; j < ROAD_LENGTH; ++j)
		{
			c.X = i + ROAD_X_OFFSET;
			c.Y = j + ROAD_Y_OFFSET;
			SetConsoleCursorPosition(OutStdConsole, c);
			PrintObj(GameScreen[i][j]);
		}
	}
}

void UpdateGame()
{
	while (!isGameOver)
	{
		// UpdateRoad();
		UpdatePlayer();
		UpdateRender();
		PrintInfo();
		isGameOver = HasCollidedWithHurdle();
		Sleep(GAMELOOP_DELAY_IN_MS);
	}
}

int main()
{
	Hidecursor();
	Reset();

	PrintInstructions();
	system("pause");

	// Game Start
	system("cls");
	thread inputThread(UpdateInput);
	thread aiThread(UpdateHurdles);
	UpdateGame();

	// Game over
	COORD c;
	c.X = 0;
	c.Y = ROAD_Y_OFFSET + ROAD_LENGTH + 2;
	SetConsoleCursorPosition(OutStdConsole, c);
	SetConsoleTextAttribute(OutStdConsole, COLOR_TEXT);
	cout << "GAME OVER !!" << endl;
	inputThread.detach();
	aiThread.detach();
	system("pause");

	return 0;
}

/*
void TakeInput()
{
	INPUT_RECORD InRec;
	DWORD NumRead;

	ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &InRec, 1, &NumRead);

	if (InRec.EventType == KEY_EVENT)
	{
		switch (InRec.Event.KeyEvent.uChar.AsciiChar)
		{
		case 'a':
			input = -1;
			break;
		case 'd':
			input = 1;
			break;
		}
	}
}
*/
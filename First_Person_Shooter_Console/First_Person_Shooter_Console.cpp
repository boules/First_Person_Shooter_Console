#include <chrono>
#include <math.h>
#include <iostream>
using namespace std;

#include <Windows.h>
int nScreenWidth = 120;			//120 columns
int nScreenHeight = 40;			//40 rows


float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

// field of view
float fFOV = 3.14159 / 4.0; 			// pi/4  it means quarter of a circle in rad
float fDepth = 16.0f;

/**
 * # for a wall
 * . for empty Space
 *
 * interesting words:-
 * field of view
 */


int main() {

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..........#...#";
	map += L"#..........#...#";
	map += L"#..........#...#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#........#######";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	// time points
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	// Game Loop
	while (1) {

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		// contols the Character
		// Handles CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerA -= (0.8f) * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerA += (0.8f) * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			// this condition to stop the player from getting through walls
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			// this condition to stop the player from getting through walls
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}


		//loop on width of the screen wich we choose to 120 col to calc the field of view
		for (int x = 0; x < nScreenWidth; x++) {
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;

			float fEyeX = sinf(fRayAngle);		// Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {

					bHitWall = true;			// Just set distance to mazimum depth
					fDistanceToWall = fDepth;

				}
				else {

					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;
					}

				}

			}

			// Calc distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';
			// Extended ASCII table for shaded walls
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	/* close to the wall*/
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 1.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;	/* to far away (less filled so it seems darker)*/
			else										nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++) {

				if (y < nCeiling) {
					screen[y * nScreenWidth + x] = ' ';

				}
				else if (y > nCeiling && y <= nFloor)
				{
					screen[y * nScreenWidth + x] = nShade;
				}
				else
				{	// the floor
					short nFloorShade = ' ';
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)	   nFloorShade = '#';
					else if (b < 0.50) nFloorShade = 'x';
					else if (b < 0.75) nFloorShade = '.';
					else if (b < 0.90) nFloorShade = '-';
					else			   nFloorShade = ' ';



					screen[y * nScreenWidth + x] = nFloorShade;
				}

			}

		}



		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, (nScreenWidth * nScreenHeight), { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}
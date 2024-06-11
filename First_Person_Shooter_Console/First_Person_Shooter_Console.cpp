#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;
#include <math.h>
#include <utility>
#include <stdio.h>

#include <Windows.h>
int nScreenWidth = 120;			//120 columns
int nScreenHeight = 40;			//40 rows


float fPlayerX = 14.7f;			// player initial position
float fPlayerY = 1.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;			// the floor dimentions of the map
int nMapWidth = 16;

// field of view
float fFOV = 3.14159f / 4.0f; 			// pi/4  it means quarter of a circle in rad

float fDepth = 16.0f;
float fSpeed = 5.0f;

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
			fPlayerA -= (fSpeed * 0.15f) * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerA += (fSpeed * 0.15f) * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;

			// this condition to stop the player from getting through walls
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;

			// this condition to stop the player from getting through walls
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}


		//loop on width of the screen wich we choose to 120 col to calc the field of view
		for (int x = 0; x < nScreenWidth; x++) {
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0.0f;
			float fStepSize = 0.1f;

			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle);		// Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += fStepSize;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {

					bHitWall = true;			// Just set distance to mazimum depth
					fDistanceToWall = fDepth;

				}
				else {

					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') {
						bHitWall = true;

						vector<pair<float, float>> p; // distance, dot product (angle between two vectors)
						
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {

								// Angle of corner to eye
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;

								float d = sqrt(vx * vx + vy * vy); // magnetiude of the vector
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d); // the dot product

								p.push_back(make_pair(d, dot));
							}
						}

						// sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });
						
						float fBound = 0.005;	// edge size
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
						
					}

				}

			}

			// Calc distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';
			// Extended ASCII table for shaded walls
			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588;	/* close to the wall*/
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;	/* to far away (less filled so it seems darker)*/
			else										nShade = ' ';

			if (bBoundary)		nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++) {

				if (y <= nCeiling) {
					screen[y * nScreenWidth + x] = ' ';

				}
				else if (y > nCeiling && y <= nFloor)
				{
					screen[y * nScreenWidth + x] = nShade;
				}
				else
				{	// the floor
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)	   nShade = '#';
					else if (b < 0.50) nShade = 'x';
					else if (b < 0.75) nShade = '.';
					else if (b < 0.90) nShade = '-';
					else			   nShade = ' ';


					screen[y * nScreenWidth + x] = nShade;
				}

			}

		}

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {

				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';


		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, (nScreenWidth * nScreenHeight), { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>

/*TO-DO
-fix stuf uh

-change color values to ease some processes DONE!

-clean up the code ig

-remove pawn forward movement if piece on front DONE!

-disable piece pickup if no moves are possible FIXED!

-don't display moves that go through friendly pieces DONE! and/or multiple pieces DONE!

-add roque and pawn promotion selection DONE!
*/

/*
0 pawn
1 rook
2 knight
3 bishop
4 queen
5 king
*/


int scl = 2;//scale
int nSz = 11 * scl;//board cell size
int order[8] = { 1,2,3,4,5,3,2,1 };//order to create non-pawn pieces in a simpler way
int gridx;//x value for mouse swapping
int gridy;//y value for mouse swapping
int nTurn = 1;// black/white turn value
int nWin = 0;
int nRoqueL = 0;
int nRoqueR = 0;
bool bMoving = false;// mouse is holding a piece if true
bool bEnPassant = false;
bool bPromotion = false;//value to detect en passant i na easier way
olc::vi2d currMouse;
olc::vi2d prevMouse;
std::unique_ptr<olc::Sprite> sprSheet;

class Piece {
public:
	int x, y;
	int value;
	int color;
	bool moved = false;
	bool bValid[8][8];
	void draw(olc::PixelGameEngine* pge) {
		pge->DrawPartialSprite(x,y, sprSheet.get(), 10 * value, 5 + color*5, 10, 10,scl);
	}
	void drawMoves(olc::PixelGameEngine* pge) {
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				if(bValid[x][y]){
					pge->FillCircle(nSz * x - 1 + nSz / 2, nSz * y - 1 + nSz / 2, nSz / 4, olc::Pixel(128,128,128,128));
				}
			}
		}
	}
};

Piece board[8][8];//piece values
Piece mousePiece;//piece the mouse is holding
Piece swapPiece;//dummy piece for swap

void showMoves(olc::PixelGameEngine* pge, Piece &pc) {
	int x = (pc.x - scl) / nSz;
	int y = (pc.y - scl) / nSz;
	//calculate valid positions for every bValid bool
	for (int px = -x; px < 8 - x; px++)
	{
		for (int py = -y; py < 8 - y; py++)
		{
			int nx = x + px;
			int ny = y + py;
			switch (pc.value) {
			case 0:// valid pawn move if forward move, or first move and 2 forward, or en-passant
				pc.bValid[nx][ny] = 
					((px==0&&py==0) ||
					(px == 0 && py == -pc.color && board[nx][ny].value==-1) || /*cell is 1 forward and empty OR*/
					((abs(px) == 1 && py == -pc.color) && (board[nx][ny].value!=-1 && board[nx][ny].color!=pc.color)) || /*cell is 1 forward and 1 sideways and is enemy piece OR*/
					(px==0 && py == 2 * -pc.color && (pc.moved == 0) && board[nx][ny].value==-1) || /*cell hasn't moved and is 2 forward OR*/
					((py == -pc.color && abs(px) == 1) && (board[nx][ny + pc.color].value!=-1 && board[nx][ny + pc.color].color!=pc.color)));/*location is 1 diagonally and neighbor cell is enemy*/
				bEnPassant = ((py == -pc.color && abs(px) == 1) && (board[nx][ny + pc.color].value != -1 && board[nx][ny + pc.color].color != pc.color)) ? 1 : bEnPassant;
				break;
			case 1:// valid rook move if horizontal or vertical or enemy
				pc.bValid[nx][ny] = 
					((px == 0 && py == 0) || (px == 0 || py == 0) && (board[nx][ny].color!=pc.color));
				break;
			case 2:// valid knight move if "L"
				pc.bValid[nx][ny] = ((px == 0 && py == 0) || (((abs(px) + abs(py)) == 3) && (abs(px) < 3 && abs(py) < 3)) && (board[nx][ny].color != pc.color));
				break;
			case 3://valid bishop move if diagonal or enemy
				pc.bValid[nx][ny] = ((px == 0 && py == 0) || (abs(px) == abs(py)) && (board[nx][ny].color != pc.color));
				break;
			case 4://valid queen move if horizontal, vertical or diagonal or enemy
				pc.bValid[nx][ny] = ((px == 0 && py == 0) || ((abs(px) == abs(py)) || (px == 0 || py == 0)) && (board[nx][ny].color != pc.color));
				break;
			case 5://valid king move if withn 1 ortogonal square or enemy
				pc.bValid[nx][ny] = (
					/*if itself*/
					(px == 0 && py == 0) ||
					/*if 1 orthogonally*/
					((-1 <= px && px <= 1) && (-1 <= py && py <= 1)) && (board[nx][ny].value ==-1 || board[nx][ny].color != pc.color) ||
					/*if roque(2 sideways AND rook on the sides AND rook is friendly AND rook hasnt moved)*/
					((py == 0 && abs(px) == 2) && pc.moved == 0 && board[0 + 7 * (px == 2)][0 + 7 * (pc.color == 1)].value == 1 && board[0 + 7 * (px == 2)][0 + 7 * (pc.color == 1)].color == pc.color && board[0 + 7 * (px == 2)][0 + 7 * (pc.color == 1)].moved == 0)
					);
				nRoqueL = ((py == 0 && px == -2) && pc.moved == 0 && board[0][0 + 7 * (pc.color == 1)].value == 1 && board[0][0 + 7 * (pc.color == 1)].color == pc.color && board[0][0 + 7 * (pc.color == 1)].moved == 0 && board[3][0 + 7 * (pc.color == 1)].value == -1) ? 1 : nRoqueL;
				nRoqueR = ((py == 0 && px == 2) && pc.moved == 0 && board[7][0 + 7 * (pc.color == 1)].value == 1 && board[7][0 + 7 * (pc.color == 1)].color == pc.color && board[7][0 + 7 * (pc.color == 1)].moved == 0 && board[5][0 + 7 * (pc.color == 1)].value == -1) ? 1 : nRoqueR;
				break;
			}
		}
	}
	//check horizontal/vertical clamps
	{
		if (pc.value == 1 || pc.value == 4 || pc.value == 5) {
			bool enemyN = 0;
			bool enemyE = 0;
			bool enemyS = 0;
			bool enemyW = 0;
			for (int n = x; n < 8; n++)
			{
				pc.bValid[n][y] = (enemyE) ? 0 : pc.bValid[n][y];
				enemyE = (board[n][y].value != -1) ? 1 : enemyE;
			}
			for (int n = y; n < 8; n++)
			{
				pc.bValid[x][n] = (enemyS) ? 0 : pc.bValid[x][n];
				enemyS = (board[x][n].value != -1) ? 1 : enemyS;
			}
			for (int n = x; n >= 0; n--)
			{
				pc.bValid[n][y] = (enemyW) ? 0 : pc.bValid[n][y];
				enemyW = (board[n][y].value != -1) ? 1 : enemyW;
			}
			for (int n = y; n >= 0; n--)
			{
				pc.bValid[x][n] = (enemyN) ? 0 : pc.bValid[x][n];
				enemyN = (board[x][n].value != -1) ? 1 : enemyN;
			}
		}
	}
	//check diagonal clamps
	{
		if (pc.value == 3 || pc.value == 4) {
			bool enemyNE = 0;
			bool enemySE = 0;
			bool enemySW = 0;
			bool enemyNW = 0;
			//northeast
			for (int n = 0; n < 1 + std::min(7 - x, y); n++)
			{
				int dx = x + n;
				int dy = y - n;
				pc.bValid[dx][dy] = (enemyNE) ? 0 : pc.bValid[dx][dy];
				enemyNE = (board[dx][dy].value != -1) ? 1 : enemyNE;
			}
			//southeast
			for (int n = 0; n < 1 + std::min(7 - x, 7 - y); n++)
			{
				int dx = x + n;
				int dy = y + n;
				pc.bValid[dx][dy] = (enemySE) ? 0 : pc.bValid[dx][dy];
				enemySE = (board[dx][dy].value != -1) ? 1 : enemySE;
			}
			//southwest
			for (int n = 0; n < 1 + std::min(x, 7 - y); n++)
			{
				int dx = x - n;
				int dy = y + n;
				int newn = (n < 0) ? 0 : (n > 7) ? 7 : n;
				pc.bValid[dx][dy] = (enemySW) ? 0 : pc.bValid[dx][dy];
				enemySW = (board[dx][dy].value != -1) ? 1 : enemySW;
			}
			//northwest
			for (int n = 0; n < 1 + std::min(x, y); n++)
			{
				int dx = x - n;
				int dy = y - n;
				int newn = (n < 0) ? 0 : (n > 7) ? 7 : n;
				pc.bValid[dx][dy] = (enemyNW) ? 0 : pc.bValid[dx][dy];
				enemyNW = (board[dx][dy].value != -1) ? 1 : enemyNW;
			}
		}
	}
}

void newBoard() {
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			board[x][y].value = -1;
			board[x][y].x = nSz * x + scl;
			board[x][y].y = nSz * y + scl;
			board[x][y].moved = false;
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					board[x][y].bValid[i][j] = false;
				}
			}
		}
		//black pawns
		board[x][1].value = 0;
		board[x][1].color = -1;
		//white pawns
		board[x][6].value = 0;
		board[x][6].color = 1;
		//black pieces
		board[x][0].value = order[x];
		board[x][0].color = -1;
		//white pieces
		board[x][7].value = order[x];
		board[x][7].color = 1;

	}
}

void drawBoard(olc::PixelGameEngine* pge){
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			//draw checkerboard
			if ((x + y) % 2 == 0) {
				pge->FillRect(nSz * x, nSz * y, nSz, nSz, olc::WHITE);
			}
			else {
				pge->FillRect(nSz * x, nSz * y, nSz, nSz, olc::BLACK);
			}
			//draw piece
			board[x][y].draw(pge);
		}
	}
}

class Basics : public olc::PixelGameEngine
{
public:
	Basics()
	{
		sAppName = "Basic()";
	}
public:
	bool OnUserCreate() override
	{
		sprSheet.reset(new olc::Sprite("Chess.png"));
		newBoard();
		mousePiece = { 0,0,-1,0 };
		swapPiece = { 0,0,-1,0 };

		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::GREY);
		SetPixelMode(olc::Pixel::Mode::ALPHA);

		if(GetKey(olc::R).bPressed){
			newBoard();
			mousePiece = { 0,0,-1,0 };
			swapPiece = { 0,0,-1,0 };
		}

		currMouse = GetMousePos();
		mousePiece.x = currMouse.x - nSz/2;
		mousePiece.y = currMouse.y - nSz/2;
		
		drawBoard(this);

		//if click on piece
		if (GetMouse(0).bPressed) {
			gridx = currMouse.x / nSz;
			gridy = currMouse.y / nSz;
			if (!bMoving && board[gridx][gridy].color==nTurn) {
				swapPiece = mousePiece;
				mousePiece = board[gridx][gridy];
				board[gridx][gridy] = swapPiece;

				board[gridx][gridy].x = ((board[gridx][gridy].x + (nSz / 2)) / nSz) * nSz + scl;
				board[gridx][gridy].y = ((board[gridx][gridy].y + (nSz / 2)) / nSz) * nSz + scl;

				showMoves(this, mousePiece);
				bMoving = true;
			}
			else if(bMoving){
				//if piece is pawn and reached enemy backline, promotion
				if (mousePiece.value==0 && gridy== 0 + 7 * (mousePiece.color == -1)) {
					swapPiece = mousePiece;
					swapPiece.x /= nSz;
					swapPiece.y /= nSz;

					bPromotion = true;
				}
				//if en passant, remove enemy piece
				if (bEnPassant) {
					board[gridx][gridy + mousePiece.color] = { 0,0,-1,0 };
					bEnPassant = false;
				}
				//if left roque
				if (nRoqueL && ((gridx == -2 + (swapPiece.x + nSz / 2) / nSz || gridy != (swapPiece.y + nSz / 2) / nSz))) {
					//clear rook
					board[0][0 + 7 * (mousePiece.color == 1)].value = -1;
					board[0][0 + 7 * (mousePiece.color == 1)].color = 0;
					//place rook
					board[3][0 + 7 * (mousePiece.color == 1)].value = 1;
					board[3][0 + 7 * (mousePiece.color == 1)].color = mousePiece.color;
					board[3][0 + 7 * (mousePiece.color == 1)].moved = 1;

					nRoqueL = 0;
				}
				//if right roque
				if (nRoqueR && ((gridx == 2 + (swapPiece.x + nSz / 2) / nSz || gridy != (swapPiece.y + nSz / 2) / nSz))) {
					//clear rook
					board[7][0 + 7 * (mousePiece.color == 1)].value = -1;
					board[7][0 + 7 * (mousePiece.color == 1)].color = 0;
					//place rook
					board[5][0 + 7 * (mousePiece.color == 1)].value = 1;
					board[5][0 + 7 * (mousePiece.color == 1)].color = mousePiece.color;
					board[5][0 + 7 * (mousePiece.color == 1)].moved = 1;

					nRoqueR = 0;
				}
				//if valid move
				if (mousePiece.bValid[gridx][gridy]) {
					//if king dies
					if (board[gridx][gridy].color != nTurn && board[gridx][gridy].value == 5) {
						nWin = mousePiece.color;
					}
					board[gridx][gridy] = mousePiece;
					mousePiece = { 0,0,-1,0 };

					board[gridx][gridy].x = ((board[gridx][gridy].x + (nSz / 2)) / nSz) * nSz + scl;
					board[gridx][gridy].y = ((board[gridx][gridy].y + (nSz / 2)) / nSz) * nSz + scl;

					bMoving = false;
					if ((gridx != (swapPiece.x + nSz / 2) / nSz || gridy != (swapPiece.y + nSz / 2) / nSz)) {
						board[gridx][gridy].moved = true;
						nTurn = (nTurn == 1) ? -1 : 1;
					}
				}
			}
		}
		if (bMoving) {
			mousePiece.drawMoves(this);
		}

		if (mousePiece.value >= 0) {
			mousePiece.draw(this);
		}

		if(bPromotion){
			//promotion menu
			for (int n = 0; n < 5; n++)
			{
				DrawPartialSprite(nSz * 8, 0 + nSz * n, sprSheet.get(), 10 * n, 5 - 5 * nTurn, 10, 10, scl);
				if ((nSz * 8 < currMouse.x && currMouse.x <= nSz * 9) && (0 < currMouse.y && currMouse.y <= nSz * 5)) {
					if(GetMouse(0).bPressed){
						board[swapPiece.x][swapPiece.y].value = currMouse.y / nSz;
						bPromotion = false;
					}
				}
			}
		}

		if (nWin != 0) {
			DrawString(20, 180, "Winner!", (nWin == 1) ? olc::WHITE : olc::BLACK);
		}

		FillRect(180, 170, 20, 20, (nTurn==1) ? olc::WHITE : olc::BLACK);
		prevMouse = currMouse;

		return true;
	}
};

int main()
{
	Basics demo;
	if (demo.Construct(scl * 100, scl * 100, 4, 4))
	{
		demo.Start();
	}
	return 0;
}
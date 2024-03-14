#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <list> 

#define WIDTH  50
#define HEIGHT 30

#define TILE_SIZE 8

#define EAST 0
#define SOUTH 1
#define WEST 2
#define	NORTH 3

class MazePrims
{
public:
	MazePrims(int Width, int Height) : Width(Width), Height(Height)
	{
		srand(time(NULL));
		grid.resize(Height + 2);
		for (auto& row : grid)
		{
			row.resize(Width + 2);
		}

		for (int x = 1; x < Width + 1; x++)
		{
			grid[0][x].edge[SOUTH] = true;
			grid[0][x].state = Outside;
			grid[Height][x].edge[SOUTH] = true;
			grid[Height + 1][x].state = Outside;
		}
		for (int y = 1; y < Height + 1; y++)
		{
			grid[y][0].edge[EAST] = true;
			grid[y][0].state = Outside;
			grid[y][Width].edge[EAST] = true;
			grid[y][Width + 1].state = Outside;
		}
		initMaze();
	}

	~MazePrims() {}

	bool MazeGenStep()
	{
		if (frontier.size() == 0)
			return false;

		auto randomFrontierCell = std::next(frontier.begin(), rand() % frontier.size());
		vi2 newMazeCell = *randomFrontierCell;
		frontier.erase(randomFrontierCell);
		grid[newMazeCell.y][newMazeCell.x].state = InMaze;

		std::vector<vi2> mazeNeighbor;
		for (int i = 0; i < 4; i++)
		{
			vi2 Nposition = { newMazeCell.x + orthoNeighbor[i].x, newMazeCell.y + orthoNeighbor[i].y };

			switch (grid[Nposition.y][Nposition.x].state)
			{
			case InMaze:
				mazeNeighbor.push_back(Nposition);
				break;
			case Open:
				grid[Nposition.y][Nposition.x].state = Frontier;
				frontier.push_back(Nposition);                    // Lack of break statement is intentional
			case Frontier:
				toggleBorder(newMazeCell, Nposition);
				break;
			}
		}

		vi2 toDelBorder = mazeNeighbor[rand() % mazeNeighbor.size()];
		toggleBorder(newMazeCell, toDelBorder);

		return true;
	}

	void initMaze()
	{
		for (int y = 1; y < Height + 1; y++)
			for (int x = 1; x < Width + 1; x++)
			{
				grid[y][x].state = Open;
				if (y < Height)
					grid[y][x].edge[SOUTH] = false;
				if (x < Width)
					grid[y][x].edge[EAST] = false;
			}

		vi2 position = { rand() % Width + 1, rand() % Height + 1 };
		grid[position.y][position.x].state = InMaze;

		frontier.clear();
		for (int i = 0; i < 4; i++)
		{
			vi2 Nposition = { position.x + orthoNeighbor[i].x, position.y + orthoNeighbor[i].y };

			if (grid[Nposition.y][Nposition.x].state == Open)
			{
				toggleBorder(position, Nposition);

				grid[Nposition.y][Nposition.x].state = Frontier;
				frontier.push_back(vi2(Nposition.x, Nposition.y));
			}
		}
	}

public:
	enum CellState {
		Outside,
		Open,     // Cells yet to be in the maze
		InMaze,
		Frontier  // Open cells but they're bordered by InMaze cells (except walls), meaning they're eligible to become InMaze themselves
	};

private:
	struct vi2
	{
		int x, y;
		vi2(int X, int Y)
		{
			x = X;
			y = Y;
		}
	};
	struct Cell
	{
		CellState state;
		bool edge[2]; // southern and eastern edge
	};
	std::list<vi2> frontier;

	const vi2 orthoNeighbor[4] =
	{
		{ 0, -1},
		{ 1,  0},
		{ 0,  1},
		{-1,  0}
	};

private:
	void toggleBorder(vi2 aPos, vi2 bPos)
	{
		Cell& a = grid[aPos.y][aPos.x];
		Cell& b = grid[bPos.y][bPos.x];
		if (aPos.x == bPos.x - 1)
		{
			a.edge[EAST] = !a.edge[EAST];
		}
		else if (bPos.x == aPos.x - 1)
		{
			b.edge[EAST] = !b.edge[EAST];
		}
		else if (aPos.y == bPos.y - 1)
		{
			a.edge[SOUTH] = !a.edge[SOUTH];
		}
		else if (bPos.y == aPos.y - 1)
		{
			b.edge[SOUTH] = !b.edge[SOUTH];
		}
	}

public:
	std::vector<std::vector<Cell>> grid;
	const int Width;
	const int Height;
};

class olcClass : public olc::PixelGameEngine
{
public:
	olcClass() : maze(WIDTH, HEIGHT)
	{
		sAppName = "Maze generation - Prim's algorithm";
	}

private:
	MazePrims maze;

	int xPos = 1;
	int yPos = 1;

	bool runMazeGen = false;
	bool playMode = false;
	bool won = false;

	void drawMaze()
	{
		for (int x = 0; x < maze.Width + 2; x++)
			for (int y = 0; y < maze.Height + 2; y++)
			{
				olc::Pixel p;
				switch (maze.grid[y][x].state) /*(grid[y+1][x+1].state)*/
				{
				case MazePrims::Open:
					p = olc::Pixel(230, 120, 120);
					break;
				case MazePrims::InMaze:
					p = olc::Pixel(255, 255, 255);
					break;
				case MazePrims::Frontier:
					p = olc::Pixel(240, 175, 175);
					break;
				case MazePrims::Outside:
					p = olc::Pixel(200, 170, 255);
					break;
				}

				FillRect(TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE, p);

				if (maze.grid[y][x].edge[SOUTH]/*grid[y + 1][x + 1].edge[SOUTH]*/)
					DrawLine(TILE_SIZE * x - 1, TILE_SIZE * (y + 1) - 1, TILE_SIZE * (x + 1), TILE_SIZE * (y + 1) - 1, olc::BLACK);
				if (maze.grid[y][x].edge[EAST]/*grid[y + 1][x + 1].edge[EAST]*/)
					DrawLine(TILE_SIZE * (x + 1) - 1, TILE_SIZE * y, TILE_SIZE * (x + 1) - 1, TILE_SIZE * (y + 1), olc::BLACK);
			
				if (x == xPos && y == yPos && playMode)
					FillCircle(int(float(TILE_SIZE) * (float(x) + 0.4f)), int(float(TILE_SIZE) * (float(y) + 0.4f)), TILE_SIZE/2 - 2, olc::Pixel(0, 175, 250));
			}

		if (won && ScreenWidth() >= 88)
		{
			static int size = ScreenWidth() / 88;
			DrawString(ScreenWidth() / 2 - 44*size, ScreenHeight() / 2 - 4*size, std::string("You've won!"), olc::Pixel(235, 30, 35), size);
		}
	}

	bool isWall(int direction)
	{
		switch (direction)
		{
		case NORTH:
			return maze.grid[yPos - 1][xPos].edge[SOUTH];
		case EAST:
			return maze.grid[yPos][xPos].edge[EAST];
		case SOUTH:
			return maze.grid[yPos][xPos].edge[SOUTH];
		case WEST:
			return maze.grid[yPos][xPos - 1].edge[EAST];
		}
	}

	bool movePlayer(olc::Key input)
	{
		return (GetKey(input).bPressed/* || (GetKey(input).bHeld && GetKey(olc::Key::SHIFT).bHeld)*/);
	}

public:
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (!playMode)
		{
			if (GetKey(olc::Key::G).bReleased)
				runMazeGen = !runMazeGen;
			if (runMazeGen || GetKey(olc::Key::S).bReleased)
			{
				playMode = !maze.MazeGenStep();
			}
		}
		else
		{
			//std::cout << xPos << " " << maze.Width << "\n";
			if (!won)
			{
				if (movePlayer(olc::Key::LEFT) && !isWall(WEST))
				{
					do {
						xPos--;
					} while (!isWall(WEST) && isWall(NORTH) && isWall(SOUTH));
				}
				if (movePlayer(olc::Key::RIGHT) && !isWall(EAST))
				{
					do {
						xPos++;
					} while (!isWall(EAST) && isWall(NORTH) && isWall(SOUTH));
					
				}
				if (movePlayer(olc::Key::UP) && !isWall(NORTH))
				{
					do {
						yPos--;
					} while (!isWall(NORTH) && isWall(EAST) && isWall(WEST));
					
				}
				if (movePlayer(olc::Key::DOWN) && !isWall(SOUTH))
				{
					do {
						yPos++;
					} while (!isWall(SOUTH) && isWall(EAST) && isWall(WEST));
				}
			}

			won = (xPos == maze.Width && yPos == maze.Height);
		}
		
		if (GetKey(olc::Key::R).bReleased)
		{
			maze.initMaze();
			runMazeGen = false;
			playMode = false;
			won = false;
			xPos = yPos = 1;
		}
		
		static int drawMazeFrame = 0;
		if(drawMazeFrame % 20 == 0)
			drawMaze();
		drawMazeFrame++;

		return true;
	}
};

int main()
{
	olcClass demo;
	if (demo.Construct((WIDTH+2) * TILE_SIZE, (HEIGHT+2) * TILE_SIZE, 2, 2))
		demo.Start();
	return 0;
}
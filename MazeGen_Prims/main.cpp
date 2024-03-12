#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <list> 

#define WIDTH  40
#define HEIGHT 40

#define TILE_SIZE 16

#define EAST 0
#define SOUTH 1

class MazePrims : public olc::PixelGameEngine
{
public:
	MazePrims()
	{
		sAppName = "Maze generation - Prim's algorithm";
	}

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
	enum CellState {
		Open,     // Cells yet to be in the maze
		InMaze,
		Frontier,  // Open cells but they're bordered by InMaze cells (except walls), meaning they're eligible to become InMaze themselves
		Outside
	};
	struct Cell
	{
		CellState state;
		bool edge[2]; // southern and eastern edge
	};
	Cell grid[WIDTH+2][HEIGHT+2]; // +2 because it includes a bounding box that prevents illegal memory access
	std::list<vi2> frontier;

	const vi2 orthoNeighbor[4] =
	{
		{ 0, -1},
		{ 1,  0},
		{ 0,  1},
		{-1,  0}
	};

	bool runMazeGen = false;

private:
	void initMaze()
	{
		for (int y = 1; y < HEIGHT + 1; y++)
			for (int x = 1; x < WIDTH + 1; x++)
			{
				grid[y][x].state = Open;
				if(y < HEIGHT)
					grid[y][x].edge[SOUTH] = false;
				if(x < WIDTH)
					grid[y][x].edge[EAST] = false;
			}

		vi2 position = { rand() % HEIGHT + 1, rand() % WIDTH + 1 };
		grid[position.y][position.x].state = InMaze;
		grid[position.y][position.x].edge[SOUTH] = TRUE;
		grid[position.y][position.x].edge[EAST] = TRUE;
		grid[position.y - 1][position.x].edge[SOUTH] = TRUE;
		grid[position.y][position.x - 1].edge[EAST] = TRUE;

		frontier.clear();
		for (int i = 0; i < 4; i++)
		{
			vi2 Nposition = { position.x + orthoNeighbor[i].x, position.y + orthoNeighbor[i].y };
			if (grid[Nposition.y][Nposition.x].state == Open)
			{
				grid[Nposition.y][Nposition.x].state = Frontier;
				frontier.push_back(vi2(Nposition.x, Nposition.y));
			}
		}

		runMazeGen = false;
	}

	void toggleBorder(vi2 aPos, vi2 bPos)
	{
		Cell& a = grid[aPos.y][aPos.x];
		Cell& b = grid[bPos.y][bPos.x];
		if (aPos.x == bPos.x - 1)
		{
			a.edge[EAST] = !a.edge[EAST];
		}
		else if(bPos.x == aPos.x - 1)
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

	void MazeGenStep()
	{
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
	
	}

	void drawMaze()
	{
		for (int x = 0; x < WIDTH + 2; x++)
			for (int y = 0; y < HEIGHT + 2; y++)
			{
				olc::Pixel p;
				switch /*(grid[y+1][x+1].state)*/ (grid[y][x].state)
				{
				case Open:
					p = olc::Pixel(255, 255, 255);
					break;
				case InMaze:
					p = olc::Pixel(230, 120, 120);
					break;
				case Frontier:
					p = olc::Pixel(200, 170, 170);
					break;
				case Outside:
					p = olc::Pixel(200, 170, 255);
					break;
				}

				FillRect(TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE, p);

				if (/*grid[y + 1][x + 1].edge[SOUTH]*/grid[y][x].edge[SOUTH])
					DrawLine(TILE_SIZE * x, TILE_SIZE * (y + 1) - 1, TILE_SIZE * (x + 1), TILE_SIZE * (y + 1) - 1, olc::BLACK);
				if (/*grid[y + 1][x + 1].edge[EAST]*/grid[y][x].edge[EAST])
					DrawLine(TILE_SIZE * (x + 1) - 1, TILE_SIZE * y, TILE_SIZE * (x + 1) - 1, TILE_SIZE * (y + 1), olc::BLACK);
			}
	}

public:
	bool OnUserCreate() override
	{
		srand(time(NULL));
		for (int x = 1; x < WIDTH+1; x++)
		{
			grid[0][x].edge[SOUTH] = true;
			grid[0][x].state = Outside;
			grid[HEIGHT][x].edge[SOUTH] = true;
			grid[HEIGHT+1][x].state = Outside;
		}
		for (int y = 1; y < HEIGHT + 1; y++)
		{
			grid[y][0].edge[EAST] = true;
			grid[y][0].state = Outside;
			grid[y][WIDTH].edge[EAST] = true;
			grid[y][WIDTH+1].state = Outside;
		}

		initMaze();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::G).bReleased)
			runMazeGen = !runMazeGen;
		if (runMazeGen || GetKey(olc::Key::S).bReleased)
		{
			if (frontier.size() > 0)
			{
				MazeGenStep();
			}
			else
				runMazeGen = false;
		}
		if (GetKey(olc::Key::R).bReleased)
			initMaze();
		
		static int drawMazeFrame = 0;
		if(drawMazeFrame % 10 == 0)
			drawMaze();
		drawMazeFrame++;

		return true;
	}
};

int main()
{
	MazePrims demo;
	if (demo.Construct((WIDTH+2) * TILE_SIZE, (HEIGHT+2) * TILE_SIZE, 1, 1))
		demo.Start();
	return 0;
}
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <list> 

#define WIDTH  5
#define HEIGHT 5

#define TILE_SIZE 32

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
		Frontier  // Open cells but they're bordered by InMaze cells (except walls), meaning they're eligible to become InMaze themselves
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

private:
	void MazeGenStep()
	{
		auto randomFrontierCell = std::next(frontier.begin(), rand() % frontier.size());
		vi2 newMazeCell = *randomFrontierCell;
		std::cout << newMazeCell.x << "   " << newMazeCell.y << "\n";
		frontier.erase(randomFrontierCell);
		grid[newMazeCell.y][newMazeCell.x].state = InMaze;
	}

public:
	bool OnUserCreate() override
	{
		srand(time(NULL));
		for (int x = 1; x < WIDTH+1; x++)
		{
			grid[0][x].edge[SOUTH] = true;
			grid[0][x].state = InMaze;
			grid[HEIGHT][x].edge[SOUTH] = true;
			grid[HEIGHT+1][x].state = InMaze;
		}
		for (int y = 1; y < HEIGHT + 1; y++)
		{
			grid[y][0].edge[EAST] = true;
			grid[y][0].state = InMaze;
			grid[y][WIDTH].edge[EAST] = true;
			grid[y][WIDTH+1].state = InMaze;
		}
		for(int y = 1; y < HEIGHT+1; y++)
			for (int x = 1; x < WIDTH+1; x++)
			{
				grid[y][x].state = Open;
			}
		
		vi2 position = { rand() % HEIGHT + 1, rand() % WIDTH + 1 };
		grid[position.y][position.x].state = InMaze;
		grid[position.y][position.x].edge[SOUTH] = TRUE;
		grid[position.y][position.x].edge[EAST] = TRUE;
		grid[position.y-1][position.x].edge[SOUTH] = TRUE;
		grid[position.y][position.x-1].edge[EAST] = TRUE;

		for (int i = 0; i < 4; i++)
		{
			vi2 Nposition = { position.x + orthoNeighbor[i].x, position.y + orthoNeighbor[i].y };
			if (grid[Nposition.y][Nposition.x].state == Open)
			{
				grid[Nposition.y][Nposition.x].state = Frontier;
				frontier.push_back(vi2(Nposition.x, Nposition.y));
			}
		}

		for (const auto& frontierCell : frontier)
		{
			std::cout << frontierCell.x << " " << frontierCell.y << "\n";
		}

		MazeGenStep();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		for (int x = 0; x < WIDTH+2; x++)
			for (int y = 0; y < HEIGHT+2; y++)
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
				}

				FillRect(TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE, p);

				if (/*grid[y + 1][x + 1]*/grid[y][x].edge[SOUTH])
					DrawLine(TILE_SIZE * x, TILE_SIZE * (y + 1) - 1, TILE_SIZE * (x + 1), TILE_SIZE * (y + 1) - 1, olc::BLACK);
				if (/*grid[y + 1][x + 1]*/grid[y][x].edge[EAST])
					DrawLine(TILE_SIZE * (x + 1) - 1 , TILE_SIZE * y, TILE_SIZE * (x + 1) - 1, TILE_SIZE * (y + 1), olc::BLACK);
				
				
			}
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
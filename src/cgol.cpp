#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include <stdexcept>
#include <chrono>
#undef main
#define PERF_ITERATIONS 10000

class CGOL
{
	int size_x;
	int size_y;
	int render_scale;
	bool* matrix_cells = nullptr;
	bool* matrix_newcells = nullptr;
	int* matrix_neighbors;
	int* matrix_newneighbors;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event event{};
	bool clicked = false;
	int mouseX = 0;
	int mouseY = 0;
	bool step = false;
	bool continuous = true;
	int sleep = 0;
	SDL_Color gridColor = { 255, 255, 255, 255 };
	SDL_Color cellColor = { 255, 0, 0, 255 };
	SDL_Color bgColor = { 0, 0, 0, 255 };


	bool _get_cell(int x, int y, bool* mat)
	{
		if (x < 0 || y < 0 || x >= size_x || y >= size_y)
		{
			return false;
		}
		return mat[x + size_x * y];
	}

	bool _get_mcell(int x, int y)
	{
		return _get_cell(x, y, matrix_cells);
	}

	bool _get_newmcell(int x, int y)
	{
		return _get_cell(x, y, matrix_newcells);
	}

	void _set_newmcell(int x, int y, bool value)
	{
		matrix_newcells[x + size_x * y] = value;
	}

	void _switch_newcell(int x, int y)
	{
		bool& cellstate = matrix_newcells[x + size_x * y];
		if (cellstate)
		{
			_dec_neighbor(x, y);
		}
		else
		{
			_inc_neighbor(x, y);
		}
		cellstate = !cellstate;
	}

	void _change_neighbor(int x, int y, bool increase)
	{
		for (int k = y - 1; k <= y + 1; k++)
		{
			for (int l = x - 1; l <= x + 1; l++)
			{
				if (k < 0 || k >= size_y || l < 0 || l >= size_x || (y == k && x == l))
				{
					continue;
				}
				if (increase)
				{
					matrix_newneighbors[l + size_x * k]++;
				}
				else
				{
					matrix_newneighbors[l + size_x * k]--;
				}
			}
		}
	}

	void _inc_neighbor(int x, int y)
	{
		_change_neighbor(x, y, true);
	}

	void _dec_neighbor(int x, int y)
	{
		_change_neighbor(x, y, false);
	}

	void _count_neighbors()
	{
		std::fill_n(matrix_neighbors, size_x * size_y, 0);
		for (int i = 0; i < size_y; i++)
		{
			for (int j = 0; j < size_x; j++)
			{
				if (_get_newmcell(j, i))
				{
					_inc_neighbor(j, i);
				}
			}
		}
	}

	void _swap_cellmatrices()
	{
		bool* tmp_matrix = matrix_cells;
		matrix_cells = matrix_newcells;
		matrix_newcells = tmp_matrix;
	}

	void _swap_neighbormatrices()
	{
		int* tmp_matrix = matrix_neighbors;
		matrix_neighbors = matrix_newneighbors;
		matrix_newneighbors = tmp_matrix;
	}

	void _advance_cell(int x, int y)
	{
		int neighbor = matrix_neighbors[x + size_x * y];
		bool alive = false;
		if (_get_mcell(x, y))
		{
			if (neighbor == 2 || neighbor == 3)
			{
				alive = true;
			}
		}
		else
		{
			if (neighbor == 3)
			{
				alive = true;
			}
		}
		_set_newmcell(x, y, alive);
		if (alive)
		{
			_inc_neighbor(x, y);
		}
	}

	void _advance()
	{
		_swap_neighbormatrices();
		std::fill_n(matrix_newneighbors, size_x * size_y, 0);
		_swap_cellmatrices();
		for (int i = 0; i < size_y; i++)
		{
			for (int j = 0; j < size_x; j++)
			{
				_advance_cell(j, i);
			}
		}
	}

	void _changeColor(SDL_Color& color)
	{
		color.r = rand() % 256;
		color.g = rand() % 256;
		color.b = rand() % 256;
	}

	void _changeAllColor()
	{
		_changeColor(gridColor);
		_changeColor(cellColor);
		_changeColor(bgColor);
	}

public:
	bool quit = false;
	bool render_grid = false;

	CGOL(int x, int y, int scale)
	{
		size_x = x;
		size_y = y;
		render_scale = scale;
		matrix_cells = new bool[x * y]{};
		matrix_newcells = new bool[x * y]{};
		matrix_neighbors = new int[x * y]{};
		matrix_newneighbors = new int[x * y]{};
	}

	CGOL(bool* inpmatrix, int x, int y, int scale) : CGOL(x, y, scale)
	{
		if (matrix_newcells)
		{
			memcpy(matrix_newcells, inpmatrix, x * y * sizeof(bool));
		}
		else
		{
			throw std::runtime_error("*matrix_newcells = '0'");
		}
		_count_neighbors();
	}

	void sdlInit()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			printf("Error initializing SDL: %s\n", SDL_GetError());
		}
		window = SDL_CreateWindow("CGOL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size_x * render_scale,
			size_y * render_scale,
			SDL_WINDOW_SHOWN);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}

	void randomize()
	{
		for (int i = 0; i < size_x * size_y; i++)
		{
			matrix_newcells[i] = (rand() > (RAND_MAX / 2));
		}
		_count_neighbors();
	}

	void getInput()
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			default:
				break;
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				clicked = SDL_GetMouseState(&mouseX, &mouseY);
				break;
			case SDL_KEYDOWN:
				SDL_Keycode key = event.key.keysym.sym;
				switch (key)
				{
				default:
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_SPACE:
					step = !step;
					continuous = false;
					break;
				case SDLK_x:
					sleep -= 20;
					if (sleep < 0)
					{
						sleep = 0;
					}
					break;
				case SDLK_z:
					sleep += 20;
					if (sleep > 150)
					{
						sleep = 150;
					}
					break;
				case SDLK_l:
					randomize();
					break;
				case SDLK_g:
					render_grid = !render_grid;
					break;
				case SDLK_c:
					continuous = !continuous;
					break;
				case SDLK_q:
					_changeColor(bgColor);
					break;
				case SDLK_w:
					_changeColor(cellColor);
					break;
				case SDLK_e:
					_changeColor(gridColor);
					break;
				case SDLK_r:
					_changeAllColor();
					break;
				}
			}
		}
	}

	void update()
	{
		if (clicked)
		{
			_switch_newcell(mouseX / render_scale, mouseY / render_scale);
			clicked = false;
		}
		if (step || continuous)
		{
			_advance();
			if (!continuous)
			{
				step = !step;
			}
			else
			{
				SDL_Delay(sleep);
			}
		}
	}

	void render()
	{
		SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
		SDL_RenderClear(renderer);
		for (int i = 0; i < size_y; i++)
		{
			for (int j = 0; j < size_x; j++)
			{
				SDL_Rect rect = { j * render_scale, i * render_scale, render_scale, render_scale };
				if (_get_newmcell(j, i))
				{
					SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, cellColor.a);
					SDL_RenderFillRect(renderer, &rect);
				}
				if (render_grid)
				{
					SDL_SetRenderDrawColor(renderer, gridColor.r, gridColor.b, gridColor.g, gridColor.a);
					SDL_RenderDrawRect(renderer, &rect);
				}
			}
		}
		SDL_RenderPresent(renderer);
	}

	~CGOL()
	{
		delete[] matrix_cells;
		delete[] matrix_newcells;
		delete[] matrix_neighbors;
		delete[] matrix_newneighbors;
		if (renderer)
		{
			SDL_DestroyRenderer(renderer);
		}
		if (window)
		{
			SDL_DestroyWindow(window);
		}
	}
};

int main()
{
	srand(time(NULL));
	CGOL game = CGOL(1920, 1080, 1);
	game.randomize();
	game.sdlInit();
	while (!game.quit)
	{
		game.getInput();
		game.update();
		game.render();
	}
}

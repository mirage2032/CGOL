#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <stdexcept>
#undef main

class CGOL
{
	int size_x;
	int size_y;
	int render_scale;
	bool* matrix = nullptr;
	bool* newmatrix = nullptr;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event event;
	bool clicked = false;
	int mouseX, mouseY;
	bool step = false;
	bool continuous = false;
	SDL_Color gridColor = {255, 255, 255, 255};
	SDL_Color cellColor = {255, 0, 0, 255};
	SDL_Color bgColor = {0, 0, 0, 255};


	void _initData(int x, int y, int scale)
	{
		size_x = x;
		size_y = y;
		render_scale = scale;
		matrix = new bool[x * y]{};
		newmatrix = new bool[x * y]{};
	}

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
		return _get_cell(x, y, matrix);
	}

	bool _get_newmcell(int x, int y)
	{
		return _get_cell(x, y, newmatrix);
	}

	void _set_newmcell(int x, int y, bool value)
	{
		newmatrix[x + size_x * y] = value;
	}

	void _switch_newcell(int x, int y)
	{
		newmatrix[x + size_x * y] = !newmatrix[x + size_x * y];
	}

	int _count_neighbors(int x, int y)
	{
		int neighbors = 0;
		for (int i = y - 1; i <= y + 1; i++)
		{
			for (int j = x - 1; j <= x + 1; j++)
			{
				if (i == y && j == x)
				{
					continue;
				}
				if (_get_mcell(j, i))
				{
					neighbors += 1;
				}
			}
		}
		return neighbors;
	}

	void _swap_matrices()
	{
		bool* tmp_matrix = matrix;
		matrix = newmatrix;
		newmatrix = tmp_matrix;
	}

	void _advance_cell(int x, int y)
	{
		int neighbor = _count_neighbors(x, y);
		if (_get_mcell(x, y))
		{
			if (neighbor == 2 || neighbor == 3)
			{
				_set_newmcell(x, y, true);
			}
			else
			{
				_set_newmcell(x, y, false);
			}
		}
		else
		{
			if (neighbor == 3)
			{
				_set_newmcell(x, y, true);
			}
			else
			{
				_set_newmcell(x, y, false);
			}
		}
	}

	void _advance()
	{
		_swap_matrices();
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
	int sleep = 100;

	CGOL(int x, int y, int scale)
	{
		_initData(x, y, scale);
	}

	CGOL(bool* inpmatrix, int x, int y, int scale)
	{
		_initData(x, y, scale);
		if (newmatrix)
		{
			memcpy(newmatrix, inpmatrix, x * y * sizeof(bool));
		}
		else
		{
			throw std::runtime_error("*newmatrix = '0'");
		}
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
			newmatrix[i] = rand() % 2;
		}
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
				case SDLK_SPACE:
					step = !step;
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
					if (sleep>150)
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
				SDL_Rect rect = {j * render_scale, i * render_scale, render_scale, render_scale};
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
		free(matrix);
		free(newmatrix);
	}
};

int main()
{
	srand(time(NULL));
	CGOL game = CGOL(1920, 1080,1);
	game.sdlInit();
	while (!game.quit)
	{
		game.getInput();
		game.update();
		game.render();
	}
}

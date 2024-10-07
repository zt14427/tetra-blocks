#define VERSION "1.0"
#define WINDOW_TITLE "Tetris"
#define SCREEN_WIDTH_TILES 12
#define SCREEN_HEIGHT_TILES 22
#define TILE_SIZE 32
#define FPS 60
#include "raylib.h"
#include "pieces.h"

enum TileType {
	I,
	O,
	T,
	J,
	L,
	S,
	Z,
	EMPTY,
	BORDER,
};

typedef struct State {
	bool paused;
	int frame;
	int tick;
} State;

typedef struct Tile {
	int x;
	int y;
	int type;
} Tile;

typedef struct Tetromino {
	int x;
	int y;
	int type;
	int state;
	Color TileColor[9];
} Tetromino;

void Setup();
void GameLoop();
void InitGrid(Tile grid[][SCREEN_HEIGHT_TILES]);
void Update(Tile grid[][SCREEN_HEIGHT_TILES], State* state, Tetromino* tetromino);
void Draw(Tile grid[][SCREEN_HEIGHT_TILES], State* state, Tetromino* tetromino);
void DrawShadowedRectangle(int x, int y, Color c);
void CollisionCheck(Tile grid[][SCREEN_HEIGHT_TILES], Tetromino* tetromino);
bool CollisionCheckBool(Tile grid[][SCREEN_HEIGHT_TILES], Tetromino* tetromino);
void ClearLineCheck(Tile grid[][SCREEN_HEIGHT_TILES]);
bool LoseCheck(grid, tetromino);
void Unload();

int main() {
	Setup();
	GameLoop();
	Unload();
	return 0;
}

void Setup() {
	InitWindow(SCREEN_WIDTH_TILES * TILE_SIZE, SCREEN_HEIGHT_TILES * TILE_SIZE, "Tetris");
	SetTargetFPS(FPS);
}

void Unload() {
	CloseWindow();
}

void GameLoop() {
	State state = {.paused=false, .frame=0, .tick=(int)(FPS*0.5)};
	Tile grid[SCREEN_WIDTH_TILES][SCREEN_HEIGHT_TILES];
	Tetromino tetromino = {
		.x = (int)(SCREEN_WIDTH_TILES / 2),
		.y = 0,
		.type = GetRandomValue(0,6),
		.state = 0,
	};
	InitGrid(grid);
	while (!WindowShouldClose()) {
		Draw(grid, &state, &tetromino);
		Update(grid, &state, &tetromino);
	}
}

void InitGrid(Tile grid[][SCREEN_HEIGHT_TILES]) {
	// Initialize the grid with borders and empty tiles
	for (int i = 0; i < SCREEN_WIDTH_TILES; i++) {
		grid[i][0].type = BORDER;
		grid[i][SCREEN_HEIGHT_TILES - 1].type = BORDER;
	}
	for (int i = 0; i < SCREEN_HEIGHT_TILES; i++) {
		grid[0][i].type = BORDER;
		grid[SCREEN_WIDTH_TILES - 1][i].type = BORDER;
	}
	for (int i = 1; i < SCREEN_WIDTH_TILES - 1; i++) for (int j = 1; j < SCREEN_HEIGHT_TILES - 1; j++) grid[i][j].type = EMPTY;
}

void Draw(Tile grid[][SCREEN_HEIGHT_TILES], State* state, Tetromino* tetromino) {
	// Colors for the tetromino and the grid

	const static Color TileColor[9] = {
		{102, 191, 255, 255}, // I-cyan
		{253, 249,   0, 255}, // O-yellow
		{200, 122, 255, 255}, // T-purple
		{0,   121, 241, 255}, // J-blue
		{255, 161,   0, 255}, // L-orange
		{0,   228,  48, 255}, // S-green
		{230,  41,  55, 255}, // Z-red
		{  0,   0,   0, 255}, // EMPTY
		{130, 130, 130, 255}, //BORDER
	};

	
	BeginDrawing();
	ClearBackground(BLANK);

	// Draw the tetromino
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) {
		if (tet_map[tetromino->type][tetromino->state][i][j]) {
			DrawShadowedRectangle((tetromino->x + i) * TILE_SIZE, (tetromino->y + j) * TILE_SIZE, TileColor[tetromino->type]);
		}
	}

	for (int i = 0; i < SCREEN_WIDTH_TILES; i++) for (int j = 0; j < SCREEN_HEIGHT_TILES; j++) {
		switch (grid[i][j].type) {
		case EMPTY:
			break;
		case BORDER:
			DrawShadowedRectangle(i * TILE_SIZE, j * TILE_SIZE, GRAY);
			break;
		default:
			DrawShadowedRectangle(i * TILE_SIZE, j * TILE_SIZE, TileColor[grid[i][j].type]);
			break;
		};
	};

	if (state->paused) {
		DrawRectangle(0, 0, SCREEN_WIDTH_TILES * TILE_SIZE, SCREEN_HEIGHT_TILES * TILE_SIZE, (Color) {.r=0, .g=0, .b=0, .a=128 });
	};

	EndDrawing();
};

void Update(Tile grid[][SCREEN_HEIGHT_TILES], State* state, Tetromino* tetromino) {

	// Pause the game
	if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
	if (state->paused) return;
	state->frame++;
	
	bool collision = false;
	// Rotate the tetromino
	if (IsKeyPressed(KEY_W)) tetromino->state++;
	if (tetromino->state > 3) tetromino->state = 0;
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) {
		if (tet_map[tetromino->type][tetromino->state][i][j] &&
			grid[tetromino->x + i][tetromino->y + j].type != EMPTY) {
			collision = true;
		}
	}
	if (collision) tetromino->state--;
	if (tetromino->state < 0) tetromino->state = 3;

	// Move the tetromino left or right if possible
	if (IsKeyDown(KEY_S)) state->frame += state->tick;
	if (IsKeyPressed(KEY_A)) {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (tet_map[tetromino->type][tetromino->state][i][j] &&
					grid[tetromino->x + i - 1][tetromino->y + j].type != EMPTY)
					collision = true;
		if (!collision) tetromino->x--;
	}
	if (IsKeyPressed(KEY_D)) {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (tet_map[tetromino->type][tetromino->state][i][j] &&
					grid[tetromino->x + i + 1][tetromino->y + j].type != EMPTY)
					collision = true;
		if (!collision) tetromino->x++;
	}
	
	// Move the tetromino down
	if (state->frame >= state->tick) {
		state->frame = 0;
		if (LoseCheck(grid, tetromino)) InitGrid(grid);
		CollisionCheck(grid, tetromino);
		ClearLineCheck(grid);
		tetromino->y++;
	};
}

bool LoseCheck(Tile grid[][SCREEN_HEIGHT_TILES], Tetromino* tetromino) {
	// Check if the tetromino is colliding with the top border
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) 
		if (tet_map[tetromino->type][tetromino->state][i][j]) 
			if (grid[tetromino->x + i][tetromino->y + j].type != EMPTY) return true;
	return false;
};

void CollisionCheck(Tile grid[][SCREEN_HEIGHT_TILES], Tetromino* tetromino) {
	// Check if the tetromino is colliding with the bottom border or another tetromino
	bool respawn = false;
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
		if (tet_map[tetromino->type][tetromino->state][i][j])
			if (grid[tetromino->x + i][tetromino->y + j + 1].type != EMPTY) {
				respawn = true;
			};
	if (respawn) {
		for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) 
			if (tet_map[tetromino->type][tetromino->state][i][j])
				grid[tetromino->x + i][tetromino->y + j].type = tetromino->type;
		tetromino->x = (int)(SCREEN_WIDTH_TILES / 2);
		tetromino->y = 0;
		tetromino->type = GetRandomValue(0, 6);
		tetromino->state = 0;
	}
}

bool CollisionCheckBool(Tile grid[][SCREEN_HEIGHT_TILES], Tetromino* tetromino) {
	// Check if the tetromino is colliding with the bottom border or another tetromino
	bool collision = false;
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
		if (tet_map[tetromino->type][tetromino->state][i][j])
			if (grid[tetromino->x + i][tetromino->y + j + 1].type != EMPTY) {
				collision = true;
			};
	return collision;
}

void ClearLineCheck(Tile grid[][SCREEN_HEIGHT_TILES]) {
	// Check if a line is full and clear it
	for (int j = 1; j < SCREEN_HEIGHT_TILES - 1; j++) {
		bool clear = true;
		for (int i = 1; i < SCREEN_WIDTH_TILES - 1; i++) {
			if (grid[i][j].type == EMPTY) {
				clear = false;
			}
		}
		if (clear) {
			for (int k = j; k > 1; k--)
				for (int i = 1; i < SCREEN_WIDTH_TILES - 1; i++)
					grid[i][k].type = grid[i][k - 1].type;
			clear = false;
		}
	}
}

void DrawShadowedRectangle(int x, int y, Color c) {
	// Draw a rectangle with a shadow
	const static Color SHADOW = { .r = 0, .g = 0, .b = 0, .a = 32 };
	const static float SHADOW_OFFSET = 0.1f;
	const static float SHADOW_OFFSET_INV = 0.9f;
	DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, c);
	// Draw a shadow offset by 0.1 times the tile size
	DrawRectangle(
		x + (int)(TILE_SIZE * SHADOW_OFFSET),
		y + (int)(TILE_SIZE * SHADOW_OFFSET),
		TILE_SIZE - (int)(TILE_SIZE * SHADOW_OFFSET),
		TILE_SIZE - (int)(TILE_SIZE * SHADOW_OFFSET),
		SHADOW);
	// Draw a shadow offset by 0.9 times the tile size
	DrawRectangle(
		x + (int)(TILE_SIZE * SHADOW_OFFSET_INV),
		y,
		(int)(TILE_SIZE * SHADOW_OFFSET),
		TILE_SIZE,
		SHADOW);
	DrawRectangle(
		x,
		y + (int)(TILE_SIZE * SHADOW_OFFSET_INV),
		TILE_SIZE - (int)(TILE_SIZE * SHADOW_OFFSET + 1), // 1 to prevent overlap; scuffed
		(int)(TILE_SIZE * SHADOW_OFFSET),
		SHADOW);
};

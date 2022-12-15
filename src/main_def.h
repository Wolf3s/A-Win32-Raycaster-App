#pragma once
#include <windows.h>
#include <stdint.h>
#include <algorithm>
#include <vector>

// self-exp
const int map_width = 16;
const int map_height = 16;

// array storing the walls in the map
const int map[map_width][map_height] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,3,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,4,0,0,0,0,5,5,5,5,5,5,5,5,1},
  {1,0,5,0,0,0,0,5,0,5,0,5,0,5,0,1},
  {1,0,6,0,0,0,0,5,0,0,0,0,0,0,0,1},
  {1,0,7,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,8,0,0,0,0,5,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,5,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,5,5,5,5,0,5,5,5,1},
  {1,6,6,0,0,6,6,6,6,6,6,0,6,6,6,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,6,6,6,6,6,0,6,6,6,6,0,6,6,6,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// key code
#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44

#define WIN_WIDTH 640
#define WIN_HEIGHT 480

// texture max size
#define TILE_WIDTH 64
#define TILE_HEIGHT 64

#define MAX_TEX 128
#define MAX_SPRITES 1

int backbuffer_width = 640;
int backbuffer_height = 480;

BOOL bRunning = TRUE;
BOOL start_game = FALSE;

// struct where we store the bitmap from our "assets" folder
typedef struct game_bitmap 
{
	void *memory;
	BITMAPINFO bitmap_info;
}game_bitmap;


////////////////////////
//  SPRITES/ENTITIES  //
////////////////////////

enum ai_state_t {
	IDLE,
	FOLLOW,
	ATTACK
};

typedef struct entity_t
{
	float x, y;
	float block_dist = 1.0f;	// distance between sprite and entity for when they collide
	float speed = 3.0f;
	ai_state_t AI_STATE;
}entity_t;

typedef struct player_t
{
	float x, y;
	int health;
	float walk_vel;
	float rot_vel;
	int damage;
}player_t;

player_t player = { 5.0f, 5.0f, 100, 10.0f, 3.0f, 10 };

float game_time = 0.0f;

const float speed = 10.0f;

float dir_X = -1.0f;
float dir_Y = 0.0f;

float plane_X = 0.0f;
float plane_Y = 0.66f;

entity_t entities[MAX_SPRITES] = {
	{5.0f, 6.0f}/*
	{4.0f, 6.0f},
	{3.0f, 6.0f},
	{2.0f, 6.0f}*/
};

float z_buffer[WIN_WIDTH];

int sprite_order[MAX_SPRITES];
double sprite_distance[MAX_SPRITES];

// TODO: understand this function deeply on how it works
void sort_sprites(int* order, double* dist, int amount)
{
	std::vector<std::pair<double, int>> sprites(amount);
	for (int i = 0; i < amount; i++) {
		sprites[i].first = dist[i];
		sprites[i].second = order[i];
	}
	std::sort(sprites.begin(), sprites.end());
	// restore in reverse order to go from farthest to nearest
	for (int i = 0; i < amount; i++) {
		dist[i] = sprites[amount - i - 1].first;
		order[i] = sprites[amount - i - 1].second;
	}
}

// if the game is running and click the second time
// the game executable, it will close the second window
bool already_running(void)
{
	HANDLE mutex;

	mutex = CreateMutex(NULL, FALSE, L"GAME_MUTEX");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return true;
	}

	else
	{
		return false;
	}
}

// loads a .bmpx file on memory
int LoadBMP_FromDisk(const char* file_path, game_bitmap* bitmap_target)
{
	HANDLE file_hnd = INVALID_HANDLE_VALUE;

	// check if file actually exists
	if ((file_hnd = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Invalid file handle!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	DWORD num_bytes_read = 2;

	WORD bitmap_header = 0;

	int pixel_data_offset = 0x36;

	int header_data_offset = 0xE;

	if (ReadFile(file_hnd, &bitmap_header, num_bytes_read, &num_bytes_read, NULL) == false)
	{
		MessageBoxA(NULL, "Failed to read from file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// in every bitmap file (specifically the .bmpx extension)
	// the first two bytes hold the number 0x4d42 ("BM" spelt backwards in ASCII)
	if (bitmap_header != 0x4d42)
	{
		MessageBoxA(NULL, "Invalid file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// now we move to where the bitmap header data is and the we read from it and save in the bitmap
	if(SetFilePointer(file_hnd, header_data_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return -1;
	}

	if (ReadFile(file_hnd, &bitmap_target->bitmap_info.bmiHeader, sizeof(BITMAPINFOHEADER), &num_bytes_read, NULL) == false)
	{
		MessageBoxA(NULL, "Failed to read from file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// we allocate on memory the amount of the pixel data
	if ((bitmap_target->memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bitmap_target->bitmap_info.bmiHeader.biSizeImage)) == NULL)
	{
		MessageBoxA(NULL, "Failed to allocate memory!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// we move to where the pixel are actually store and read from there
	if (SetFilePointer(file_hnd, pixel_data_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return -1;

	if (ReadFile(file_hnd, bitmap_target->memory, bitmap_target->bitmap_info.bmiHeader.biSizeImage, &num_bytes_read, NULL) == false)
	{
		MessageBoxA(NULL, "Failed to read from file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	CloseHandle(file_hnd);

	return 0;
}

// loads bitmap data into a buffer
bool LoadTex_ToBuff(game_bitmap* bitmap, uint32_t* texture)
{
	int32_t bitmap_offset = 0;
	int32_t bitmap_pixel = 0;
	int32_t start_bitmap = bitmap->bitmap_info.bmiHeader.biWidth * bitmap->bitmap_info.bmiHeader.biHeight - bitmap->bitmap_info.bmiHeader.biWidth;

	for (int32_t pixel_y = 0; pixel_y < bitmap->bitmap_info.bmiHeader.biHeight; pixel_y++)
	{
		for (int32_t pixel_x = 0; pixel_x < bitmap->bitmap_info.bmiHeader.biWidth; pixel_x++)
		{
			bitmap_offset = start_bitmap + pixel_x - pixel_y * bitmap->bitmap_info.bmiHeader.biWidth;

			memcpy_s(&bitmap_pixel, sizeof(uint32_t), (uint32_t*)bitmap->memory + bitmap_offset, sizeof(uint32_t));

			texture[pixel_x + pixel_y * TILE_WIDTH] = bitmap_pixel;
		}
	}

	return true;
}

// copies pixel data into the color variable, given the bitmap portion we want to copy from
int LoadTextureIndex(uint32_t* color, game_bitmap bitmap, int index_x, int index_y, int tex_x, int tex_y)	//	bitmap.bitmap_info.bmiHeader.biWidth * (1 - TILE_HEIGHT * index_y) <- why i can't do this way :(
{
	// position relative to the address of the memory holding the bitmap
	uint32_t start_bitmap = (bitmap.bitmap_info.bmiHeader.biWidth * bitmap.bitmap_info.bmiHeader.biHeight) - bitmap.bitmap_info.bmiHeader.biWidth - TILE_HEIGHT * (index_y - 1) * bitmap.bitmap_info.bmiHeader.biWidth + (index_x - 1) * TILE_HEIGHT;
	
	if (start_bitmap > bitmap.bitmap_info.bmiHeader.biSizeImage || start_bitmap < 0)
	{
		MessageBoxA(NULL, "Invalid index!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// position relative to the texture we chose
	int32_t bitmap_offset = start_bitmap + tex_x - tex_y * bitmap.bitmap_info.bmiHeader.biWidth;

	// copy the color from the address of the bitmap to the variable
	memcpy_s(color, sizeof(uint32_t), (uint32_t*)bitmap.memory + bitmap_offset, sizeof(uint32_t));
	return true;
}
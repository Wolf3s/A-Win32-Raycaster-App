#pragma once
#include <chrono>
#include <vector>
#include "main_def.h"

// global backbuffer
game_bitmap gbackbuffer;

class Draw
{
public:
	Draw()
	{
	
	
	}

	~Draw()
	{
		
	}

	Draw(int x, int y, uint32_t color)
	{
		uint32_t* pixel = (uint32_t*)gbackbuffer.memory;

		// clip any pixel outside client window
		if (y > backbuffer_height - 1 || x > backbuffer_width - 1 || x < 0 || y < 0) {
			// do nothing
		}
		else
			pixel = pixel + y * backbuffer_width + x;
		*pixel = color;
	}
	void Clear(uint32_t color)
	{
		uint32_t* pixel = (uint32_t*)gbackbuffer.memory;

		for (int i = 0; i < backbuffer_width * backbuffer_height; i++)
		{
			*pixel++ = color;
		}
	}
	
	void FillRectangle(int x, int y, int width, int height, uint32_t color)
	{
		for (int j = y; j < y + height; j++) {

			for (int i = x; i < x + width; i++) {

				Draw(i, j, color);

			}
		}
	}
	void Blit32BMP(game_bitmap* bitmap, int x, int y, int scale)
	{
		int32_t bitmap_offset = 0;
		int32_t bitmap_pixel = 0;
		int32_t start_bitmap = bitmap->bitmap_info.bmiHeader.biWidth * bitmap->bitmap_info.bmiHeader.biHeight - bitmap->bitmap_info.bmiHeader.biWidth;

		for (int16_t pixel_y = 0; pixel_y < bitmap->bitmap_info.bmiHeader.biHeight * scale; pixel_y++)
		{
			for (int16_t pixel_x = 0; pixel_x < bitmap->bitmap_info.bmiHeader.biWidth * scale; pixel_x++)
			{
				bitmap_offset = start_bitmap + int((float)pixel_x / scale) - int((float)pixel_y / scale) * bitmap->bitmap_info.bmiHeader.biWidth;

				memcpy_s(&bitmap_pixel, sizeof(uint32_t), (uint32_t*)bitmap->memory + bitmap_offset, sizeof(uint32_t));

				if (bitmap_pixel != -6815608)
				{
					Draw(x + pixel_x, y + pixel_y, bitmap_pixel);
				}
			}
		}


	}
	void DrawSprite(game_bitmap* bitmap, int x, int y, double distance, float scale)
	{
		int32_t bitmap_offset = 0;
		int32_t color = 0;
		int32_t start_bitmap = bitmap->bitmap_info.bmiHeader.biWidth * bitmap->bitmap_info.bmiHeader.biHeight - bitmap->bitmap_info.bmiHeader.biWidth;

		for (int pixel_x = 0; pixel_x < bitmap->bitmap_info.bmiHeader.biWidth * scale; pixel_x++)
		{
			int wich_dist = x + pixel_x;
			if (wich_dist > WIN_WIDTH) wich_dist = WIN_WIDTH;

			if (distance > 0.0 && x + pixel_x > 0 && x < WIN_WIDTH && distance < z_buffer[wich_dist] && y > 0)
			{
				for (int pixel_y = 0; pixel_y < bitmap->bitmap_info.bmiHeader.biHeight * scale; pixel_y++)
				{

					bitmap_offset = start_bitmap + int((float)pixel_x / scale) - int((float)pixel_y / scale) * bitmap->bitmap_info.bmiHeader.biWidth;

					memcpy_s(&color, sizeof(uint32_t), (uint32_t*)bitmap->memory + bitmap_offset, sizeof(uint32_t));

					if (color != -6815608)	// this color would be cyan, used for transparency
					{
						Draw(x + pixel_x, y + pixel_y, color);
					}
				}
			}

		}
	}
};

#include "main_def.h"
#include "menu.h"
#include "engine.h"
#include "draw.h"
#include <chrono>

using std::chrono::system_clock;
using std::chrono::duration;

int curpos = 0;

// this is where the messages received from the OS (Windows) are treated (e.g. clicked the "X" button: message sent by OS -> APP: close the window)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

		// idk why but this should come first before any input
		case WM_CLOSE: {
			bRunning = FALSE;
			PostQuitMessage(0);
		} break;

		case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_W:
			{
				curpos--;
			}break;

			case VK_S:
			{
				curpos++;
			}break;

			default:
				break;
			}
		}

		default:
			// any messages we did not treat? pass them to a default function
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd) {

	Engine _Engine;
	Draw _Draw;
	Menu _Menu;
	// check if there's some problem initializing the engine
	if (_Engine.Init(WndProc) != 0)
	{
		MessageBoxA(_Engine.get_hwnd(), "Failed to initialize the Engine!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	game_bitmap wolf3d_bmp = { 0 };

	if (LoadBMP_FromDisk(".\\assets\\wolf3d_tex.bmpx", &wolf3d_bmp) != 0)
	{
		MessageBoxA(NULL, "Failed to load a bmpx file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	game_bitmap knight_bmp = { 0 };

	if (LoadBMP_FromDisk(".\\assets\\knight.bmpx", &knight_bmp) != 0)
	{
		MessageBoxA(NULL, "Failed to load a bmpx file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	////////////////////
	//   INIT GAME    //
	////////////////////

	player_t player = { 5.0f, 5.0f, 100, 10.0f, 3.0f, 10 };

	float game_time = 0.0f;

	const float speed = 10.0f;

	float dir_X = -1.0f;
	float dir_Y = 0.0f;

	float plane_X = 0.0f;
	float plane_Y = 0.66f;

	auto start_time = system_clock::now();
	auto end_time = system_clock::now();
	
	float target_fps = 1.0f / 60.0f;

	while (bRunning && player.health != 0) {

		_Engine.msg_loop();
		
		// we keep track how much time has gone by between frames
		// so that we know how much we have moved when the scene is rendered

		end_time = system_clock::now();
		duration<float> elapsedTime = end_time - start_time;
		start_time = end_time;
		float elapsed_time = elapsedTime.count();

		/*if (elapsed_time < target_fps)
		{
			DWORD sleep_time = (DWORD)(target_fps * 1000 - elapsed_time * 1000);
			Sleep(sleep_time);
		}*/

		if (start_game)
		{
			if (GetAsyncKeyState(VK_ESCAPE) < 0)
				bRunning = FALSE;

			float rot_speed = 3.0f;

			// clear screen
			_Draw.FillRectangle(0, 0, WIN_WIDTH, WIN_HEIGHT / 2, 0x222222);
			_Draw.FillRectangle(0, WIN_HEIGHT / 2, WIN_WIDTH, WIN_HEIGHT / 2, 0x555555);


			////////////////////
			//  3D RENDERING  //
			////////////////////

			// WALL CASTING
			for (int x = 0; x < WIN_WIDTH; x++)
			{
				// camera x coordinates
				float camera_X = 2 * x / float(WIN_WIDTH) - 1;

				float ray_dirX = dir_X + plane_X * camera_X;
				float ray_dirY = dir_Y + plane_Y * camera_X;

				int map_X = int(player.x);
				int map_Y = int(player.y);

				float side_distX;
				float side_distY;

				float delta_distX = std::abs(1 / ray_dirX);
				float delta_distY = std::abs(1 / ray_dirY);

				float perp_wallDist;

				int step_X;
				int step_Y;

				bool hit = false;
				bool side = false;

				// finds which direction the ray points to
				if (ray_dirX < 0)
				{
					step_X = -1;
					side_distX = (player.x - map_X) * delta_distX;
				}
				else
				{
					step_X = 1;
					side_distX = (map_X + 1.0f - player.x) * delta_distX;
				}

				if (ray_dirY < 0)
				{
					step_Y = -1;
					side_distY = (player.y - map_Y) * delta_distY;

				}
				else
				{
					step_Y = 1;
					side_distY = (map_Y + 1.0f - player.y) * delta_distY;
				}

				// check and find which wall was hit
				while (hit == false)
				{
					if (side_distX < side_distY)
					{
						side_distX = side_distX + delta_distX;
						map_X = map_X + step_X;
						side = false;
					}
					else
					{
						side_distY = side_distY + delta_distY;
						map_Y = map_Y + step_Y;
						side = true;
					}

					if (map[map_X][map_Y] > 0)
					{
						hit = true;
					}
				}

				// calculating the distance between the plane and the wall
				if (side == false)
					perp_wallDist = (map_X - player.x + (1 - step_X) / 2) / ray_dirX;
				else
					perp_wallDist = (map_Y - player.y + (1 - step_Y) / 2) / ray_dirY;

				int line_height = int(WIN_HEIGHT / perp_wallDist);

				int draw_start = -line_height / 2 + WIN_HEIGHT / 2;

				if (draw_start < 0)
					draw_start = 0;

				int draw_end = line_height / 2 + WIN_HEIGHT / 2;

				if (draw_end >= WIN_HEIGHT)
					draw_end = WIN_HEIGHT - 1;

				// TODO: understand this stuff
				int tex_num = map[map_X][map_Y];

				float wall_X;	// where the wall was exactly hit

				if (side == false)
					wall_X = player.y + perp_wallDist * ray_dirY;
				else
					wall_X = player.x + perp_wallDist * ray_dirX;

				// calculates which part of the texture to draw (wall_X would go from 0 to 1)
				wall_X -= floorf(wall_X);
				int tex_X = int(wall_X * float(TILE_WIDTH));


				if (side == false && ray_dirX > 0)
					tex_X = (TILE_WIDTH - 1) - tex_X;

				if (side == true && ray_dirY < 0)
					tex_X = (TILE_WIDTH - 1) - tex_X;

				float step = 1.0f * TILE_HEIGHT / line_height;
				float tex_pos = (draw_start - WIN_HEIGHT / 2 + line_height / 2) * step;

				// loop for drawing a single scanline of texture
				for (int y = draw_start; y < draw_end; y++)
				{
					int tex_Y = int(tex_pos) & (TILE_HEIGHT - 1);
					tex_pos += step;

					uint32_t color = 0;

					// depending on the wall the ray has hit, the texture would be different
					switch (tex_num)
					{
					case 1:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 1, 1, tex_X, tex_Y);
					}break;

					case 2:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 2, tex_X, tex_Y);
					}break;

					case 3:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 4, 8, tex_X, tex_Y);
					}break;

					case 4:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 2, 5, tex_X, tex_Y);
					}break;

					case 5:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 3, tex_X, tex_Y);
					}break;

					case 6:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 5, 7, tex_X, tex_Y);
					}break;

					case 7:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 10, tex_X, tex_Y);
					}break;

					case 8:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 5, 8, tex_X, tex_Y);
					}break;

					default:
						LoadTextureIndex(&color, wolf3d_bmp, 6, 10, tex_X, tex_Y);
						break;
					}

					// this makes the texture darker to a specific side of the walls
					if (side == true)
						color = (color >> 1) & 8355711;

					Draw(x, y, color);

				}
				z_buffer[x] = perp_wallDist;
			}

			for (int i = 0; i < MAX_SPRITES; i++)
			{
				sprite_order[i] = i;
				sprite_distance[i] = (player.x - entities[i].x) * (player.x - entities[i].x) + (player.y - entities[i].y) * (player.y - entities[i].y);
			}

			// orders the sprites from far to near
			sort_sprites(sprite_order, sprite_distance, MAX_SPRITES);

			// SPRITES CASTING
			for (int i = 0; i < MAX_SPRITES; i++)
			{
				//translate sprite position to relative to camera
				double sprite_x = entities[sprite_order[i]].x - player.x;
				double sprite_y = entities[sprite_order[i]].y - player.y;

				double dist = sqrt((sprite_x) * (sprite_x)+(sprite_y) * (sprite_y));

				// follow player
				if (dist < 10.0 && dist > 2.0)	entities[i].AI_STATE = FOLLOW;	/* && map[(int)entities[sprite_order[i]].x][(int)entities[sprite_order[i]].y] == 0 */
				else if (dist <= 2.0) entities[i].AI_STATE = ATTACK;
				else entities[i].AI_STATE = IDLE;

				switch (entities[i].AI_STATE)
				{
				case FOLLOW:
				{
					entities[sprite_order[i]].x -= (sprite_x / dist) * entities[sprite_order[i]].speed * elapsed_time;
					entities[sprite_order[i]].y -= (sprite_y / dist) * entities[sprite_order[i]].speed * elapsed_time;
				}break;

				case ATTACK:
				{
					/*if ((2 + (int)game_time) % 8 == 0)
						player.health -= 10;*/
				}break;

				default:
					IDLE;
					break;
				}

				double inv_det = 1.0 / (plane_X * dir_Y - dir_X * plane_Y);

				double transform_x = inv_det * (dir_Y * sprite_x - dir_X * sprite_y);
				double transform_y = inv_det * (-plane_Y * sprite_x + plane_X * sprite_y);

				int sprite_screenX = int((WIN_WIDTH / 2) * (1 + transform_x / transform_y));

				//calculate height of the sprite on screen
				int sprite_height = abs(int(WIN_HEIGHT / (transform_y)));

				//calculate which y to start and end drawing the sprite
				int start_y = WIN_HEIGHT / 2 - sprite_height / 2;
				if (start_y < 0) start_y = 0;
				int end_y = WIN_HEIGHT / 2 + sprite_height / 2;
				if (end_y >= WIN_HEIGHT) end_y = WIN_HEIGHT - 1;

				//calculate which x to start and end drawing the sprite
				int sprite_width = abs(int(WIN_HEIGHT / (transform_y)));
				int start_x = sprite_screenX - sprite_width / 2;
				int end_x = sprite_width / 2 + sprite_screenX;
				if (end_x >= WIN_WIDTH) end_x = WIN_WIDTH - 1;

				float scale = 1.0f * sprite_height / (float)TILE_HEIGHT;

				_Draw.DrawSprite(&knight_bmp, start_x, start_y, transform_y, scale);
			}

			////////////////////
			// HANDLING INPUT //
			////////////////////

			if (GetAsyncKeyState(VK_D) < 0)
			{
				// rotate the view direction and plane
				float oldDirX = dir_X;
				dir_X = dir_X * cosf(-rot_speed * elapsed_time) - dir_Y * sinf(-rot_speed * elapsed_time);
				dir_Y = oldDirX * sinf(-rot_speed * elapsed_time) + dir_Y * cosf(-rot_speed * elapsed_time);
				float oldPlaneX = plane_X;
				plane_X = plane_X * cosf(-rot_speed * elapsed_time) - plane_Y * sinf(-rot_speed * elapsed_time);
				plane_Y = oldPlaneX * sinf(-rot_speed * elapsed_time) + plane_Y * cosf(-rot_speed * elapsed_time);
			}

			if (GetAsyncKeyState(VK_A) < 0)
			{
				// god I love math
				float oldDirX = dir_X;
				dir_X = dir_X * cosf(rot_speed * elapsed_time) - dir_Y * sinf(rot_speed * elapsed_time);
				dir_Y = oldDirX * sinf(rot_speed * elapsed_time) + dir_Y * cosf(-rot_speed * elapsed_time);
				float oldPlaneX = plane_X;
				plane_X = plane_X * cosf(rot_speed * elapsed_time) - plane_Y * sinf(rot_speed * elapsed_time);
				plane_Y = oldPlaneX * sinf(rot_speed * elapsed_time) + plane_Y * cosf(rot_speed * elapsed_time);
			}

			if (GetAsyncKeyState(VK_W) < 0)
			{
				// check ahead of player if there's a wall, if not move
				if (map[int(player.x + dir_X)][int(player.y)] == false)
					player.x += dir_X * speed * elapsed_time;

				if (map[int(player.x)][int(player.y + dir_Y)] == false)
					player.y += dir_Y * speed * elapsed_time;
			}

			if (GetAsyncKeyState(VK_S) < 0)
			{
				// check behind the player if there's a wall, if not move
				if (map[int(player.x - dir_X)][int(player.y)] == false)
					player.x -= dir_X * speed * elapsed_time;

				if (map[int(player.x)][int(player.y - dir_Y)] == false)
					player.y -= dir_Y * speed * elapsed_time;
			}
		}
		else
		{
			_Menu.menu_loop();
		}

		// slap that backybuffer to the screen
		_Engine.blit_to_DIB();
	}

	return 0;
}
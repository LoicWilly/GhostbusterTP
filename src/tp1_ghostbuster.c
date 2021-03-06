/**
 * Name        : tp1_ghostbuster.c
 * Version     : 1.0
 * Description : Ghostbuster game template. FreeRTOS in cooperative mode.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"
#include "fonts.h"
#include "traces_ref.h"
#include "custom_rand.h"
#include "math.h"


#define NO_COLLISION 0
#define GHOST_NB     5
#define STEP         2
// moving step for all objects

// Direction vector. Note that only 8 directions are possible,
// since NORTH|SOUTH is nonsense for example.
enum {
	NORTH=1,
	EAST=2,
	SOUTH=4,
	WEST=8
};

// structure containing object position, size and direction
typedef struct {
	int x;
	int y;
	int radius;
	int dir;
	bool active;
} object_t;

// object instances:  object[0] is the ball, the other objects are the ghosts
object_t object[GHOST_NB+1];
// pointers on the ghosts bitmaps. 2 images by ghost direction.
__DATA(RAM2) uint16_t *ghost_im_left[2], *ghost_im_right[2], *ghost_im_center[2];
uint16_t ghost_width, ghost_height;


/* The function looks at the collision only in the direction taken by the object referenced as "object_id".
 * It detects all collisions among all objects indexes between min_idx and max_idx (skipping object_id itself).
 * If the distance (D) with an object is lower than |x2-x1|<radius1+radius2+STEP and |y2-y1|<radius1+radius2+STEP,
 * the collision is detected.
 * Parameters:
 * 		object_id: index of the moving object trying to detect a collision
 * 		obj_array: array of all objects
 * 		min_idx:   lower index of the object to test for collision
 * 		max_idx:   higher index of the object to test for collision
 * Return: index of the object provoking the collision or NO_COLLISION
 */
int test_collision(int object_id, object_t *obj_array, int min_idx, int max_idx)
{
	int i, dx, dy, dx_col, dy_col, col;

	for (i=min_idx; i<=max_idx; i++)		// search only collisions with ghosts (never with ball)
		if (i!=object_id && object[i].active)
		{
			dx_col=obj_array[object_id].radius+obj_array[i].radius+STEP;
			dy_col=obj_array[object_id].radius+obj_array[i].radius+STEP;
			dx=obj_array[object_id].x-obj_array[i].x;
			dy=obj_array[object_id].y-obj_array[i].y;
			if (ABS(dx) <= dx_col && ABS(dy) <= dy_col)
			{
				col=NO_COLLISION;
				if (dx>0)
					col|=WEST;
				else if (dx<0)
					col|=EAST;
				if (dy>0)
					col|=NORTH;
				else if (dy<0)
					col|=SOUTH;
				if (col & obj_array[object_id].dir)	// collision tested only in the object direction
					return i;		// return (one of) the ID of the object creating the collision
			}
		}
	return NO_COLLISION;
}

int ghostSpeed(int ghost_id)
{
	return (20+ghost_id*2);
}



void deplacementObject(int id)
{

	lcd_filled_rectangle(object[id].x, object[id].y, object[id].x+ghost_width,	object[id].y+ghost_height, LCD_BLACK);


		//display_bitmap16(ghost_im_left[0], ++object[id].x, object[id].y, ghost_width, ghost_height);

		display_bitmap16(ghost_im_left[0], ++object[id].x, object[id].y, ghost_width, ghost_height);

}

void invertDirection(int id_ghost){
	if(object[id_ghost].dir = 1){object[id_ghost].dir = 4;} // nord -> sud
	if(object[id_ghost].dir = 2){object[id_ghost].dir = 8;} // est  -> ouest
	if(object[id_ghost].dir = 4){object[id_ghost].dir = 1;} // sud  -> nord
	if(object[id_ghost].dir = 8){object[id_ghost].dir = 2;} // ouest-> est
}

void initGhost()
{
	for(int i = 1; i<= GHOST_NB;i++)
	{
		object[i].dir = 1;
		object[i].x = 10+i*30;
		object[i].y = 100;
	}
}
//vTaskDelay() - attente passive
//taskYIELD()


int randDirection(int id_object){
	return (int)pow(2,(double)(rnd_32()%4));
}



//Commencez par développer le déplacement d’un fantôme seul
//(sans FreeRTOS), même chose pour la balle (servez‐vous de la structure object_t pour cela).
void ghostTask(void * param)
{
	//int id = (int)param;
	int id = 5;
	while(1)
	{
		deplacementObject(id);
		vTaskDelay(ghostSpeed(id));
	}
}


int main(void)
{
	init_rnd32(1);
	init_lcd();
	clear_screen(LCD_BLACK);
	init_traces(115200, 1, true);		// to be removed if you implement your own traces

	if ((ghost_im_left[0]=read_bmp_file("ghost_l1.bmp", &ghost_width, &ghost_height))==NULL)
		return -1;
	lcd_print(85, 100, SMALLFONT, LCD_WHITE, LCD_BLACK, "Have fun!");
	display_bitmap16(ghost_im_left[0], 110, 150, ghost_width, ghost_height);

	for(int i = 1; i<= GHOST_NB;i++)
	{
		if (xTaskCreate(ghostTask, (signed portCHAR *)"ghostTask", configMINIMAL_STACK_SIZE, i, 1, NULL)!=pdPASS) {}
	}

	vTaskStartScheduler();
	while(1);
	return 1;
}

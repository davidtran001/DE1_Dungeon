#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/* This files provides address values that exist in the system */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

// debugging
#define RLEDs ((volatile long *) 0xFF200000)

// constants
// Screen size 
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

// player boundaries 
#define X_BOUND 320 // 320 - 9 - 9 so model does not go out of bounds
#define Y_BOUND 240 // 240 - 9 - 9 so model does not go out of bounds

// PS/2 Scan Codes
#define W_PRESS 0x1D
#define A_PRESS 0x1C
#define S_PRESS 0x1B
#define D_PRESS 0x23
#define I_PRESS 0x43
#define EMPTY_PRESS 0xF0

// boundary codes
#define EMPTY_CODE 0
#define PLAYER_CODE 1
#define PROJECTILE_CODE 2
#define ZOMBIE_CODE 3
#define BARRELL_CODE 4

// direction codes
#define UP_CODE 0
#define DOWN_CODE 1
#define LEFT_CODE 2
#define RIGHT_CODE 3

// zombie constants
#define MAX_ZOMBIES 5
#define MAX_BARRELLS 3
// projectile
#define BASE_PROJECTILE_SPEED 8

// structs
struct player {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int prev2_x;
    int prev2_y;
    int direction;
    int health;
	int isAlive;
};

struct projectile {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int prev2_x;
    int prev2_y;
    int direction;
    bool isActive;
    int speed;
};

struct zombie {
    int id;
    int x;
    int y;
    int prev_x;
    int prev_y;
    int prev2_x;
    int prev2_y;
    int direction;
    int health;
    bool isAlive;
};

struct barrell {
    int x;
    int y;
	bool isActive;
};
struct health{
    int x;
    int y;
    int prev_x;
    int prev_y;
    int prev2_x;
    int prev2_y;
    int value;
    short int color;
    
}

// global variables
volatile int pixel_buffer_start; // global variable
int player_color = 0xD376D7;
int zombie_color = 0x00F000;
int barrell_color = 0xFFC1CC;
int green = 0x66cc00;
int dx = 2;
int dy = 2;
int d_projectile = 8;
int boundary[X_BOUND][Y_BOUND];
struct zombie zombies[MAX_ZOMBIES];
int num_zombies = 0;
int zombie_buffer = 0;
struct barrell barrells[MAX_BARRELLS];

// global structs
struct player player1 = {X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2, 0, 100, true};
struct projectile proj1 = {0,0,0,0,0,0,0,false,BASE_PROJECTILE_SPEED};
struct health healthBar= {X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2,100};
// function prototypes
struct zombie spawn_zombie();
void draw_zombie(int x, int y, int direction);
void save_twoframes(int *prev_pos_x, int *prev_pox_y, int *prev2_pos_x, int *prev2_pos_y, int x_pos, int y_pos);
void draw_projectile(int x, int y);
void draw_barrell(int x, int y);
void shoot_projectile(int byte1, int byte2, int byte3, struct projectile *p, struct player play);
void player_movement(int byte1, int byte2, int byte3, struct player *p);
void draw_player(int x, int y, int direction);
void wait_for_vsync();
void clear_screen();
void swap(int *xp, int *yp);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void plot_pixel(int x, int y, short int line_color);
void draw_box(int x, int y, short int color);

int main(void) {
    srand(time(NULL));

    // game setup
    /* intialize boundary
    boundary will keep track of the positions of the player, zombies and other coordinate based objects in the boundary 
    NOTE: zombies will have an id of 10 <= zombie_id < 15 */
    int i, j;
    for (i = 0; i < X_BOUND; i++) {
        for (j = 0; j < Y_BOUND; j++) {
            boundary[i][j] = EMPTY_CODE;
        }
    }
    boundary[player1.x][player1.y] = PLAYER_CODE;
    boundary[healthBar.x][healthBar.y] = PLAYER_CODE;
    // initialize zombies
    for (i = 0; i < MAX_ZOMBIES; i++) {
        struct zombie z = spawn_zombie(i);
        zombies[i] = z;
    }
   for (i = 0; i< MAX_BARRELLS; i++) {
        barrells[i].x= rand() % (X_BOUND-7 + 1 - 7) + 7;
        barrells[i].y = rand() % (Y_BOUND-7 + 1 - 7) + 7;
        barrells[i].isActive = true;
   }
    // print boundary
    /*for (i = 0; i < X_BOUND; i++) {
        for (j = 0; j < Y_BOUND; j++) {
            printf("%d", boundary[i][j]);
        }
        printf("\n");
    }*/

    // setup PS/2 port
    unsigned char byte1 = 0;
    unsigned char byte2 = 0;
	unsigned char byte3 = 0;

    volatile int * PS2_ptr = (int *) PS2_BASE;  // PS/2 port address
	int PS2_data, RVALID;

    // setup graphics
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

     /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    draw_player(player1.x, player1.y, player1.direction);
    // main loop
    while (1) {
        // discard old drawings
        draw_box(player1.prev2_x, player1.prev2_y, 0x0);
        draw_player(player1.x, player1.y, player1.direction);
        draw_healthBar(healthBar.value,);
        //printf("zombie position is (%d, %d) \n", z.x, z.y);
        //draw_box(z.prev2_x, z.prev2_y, 0x0);
        //draw_zombie(z.x, z.y, z.direction);

        if (num_zombies <= 5) {
            zombie_buffer += 1;
            if (zombie_buffer >= 8000) {
                for (i = 0; i < MAX_ZOMBIES; i++) {
                    if (!zombies[i].isAlive) {
                        struct zombie z = spawn_zombie(i);
                        zombies[i] = z;
                        zombie_buffer = 0;
                        break;
                    }
                }
            }
        }

        for (i = 0; i < MAX_ZOMBIES; i++) {
                if (!zombies[i].isAlive) draw_box(zombies[i].x, zombies[i].y, 0x0);
        }

        for (i = 0; i < MAX_ZOMBIES; i++) {
            if (zombies[i].isAlive) {
                draw_zombie(zombies[i].x, zombies[i].y, zombies[i].direction);
                //printf("drawing zombie %d\n", i);
            }
        }
		for (i = 0; i < MAX_BARRELLS; i++) {
                if (!barrells[i].isActive) draw_box(barrells[i].x, barrells[i].y, 0x0);
        }
		for (i = 0; i < MAX_BARRELLS; i++) {
            if (barrells[i].isActive) {
                draw_barrell(barrells[i].x, barrells[i].y);
            }
        }

        plot_pixel(proj1.prev2_x, proj1.prev2_y, 0x0);
		if(proj1.isActive)
           draw_projectile(proj1.x, proj1.y);
        
        save_twoframes(&player1.prev_x, &player1.prev_y, &player1.prev2_x, &player1.prev2_y, player1.x, player1.y);
        save_twoframes(&proj1.prev_x, &proj1.prev_y, &proj1.prev2_x, &proj1.prev2_y, proj1.x, proj1.y);

        // PS/2 keyboard input
        PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0) {
			/* always save the last three bytes received */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
            //printf("byte1 is %d, byte2 is %d, byte3 is %d \n", byte1, byte2, byte3);
		}
        player_movement(byte1, byte2, byte3, &player1);
        shoot_projectile(byte1, byte2, byte3, &proj1, player1);

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	}
    
}

// make new zombie struct
struct zombie spawn_zombie(int zombie_id) {
    int x_spawn = rand() % (X_BOUND-7 + 1 - 7) + 7;
    int y_spawn = rand() % (Y_BOUND-7 + 1 - 7) + 7;
    struct zombie z = {10+zombie_id, x_spawn, y_spawn, 0, 0, 0, 0, 0, 1, true};
    num_zombies += 1;
    int i, j;
    boundary[x_spawn][y_spawn] = 10+zombie_id;
    /*for (i = 0; i <= 6; i++) {
        for (j = 0; j <= 6; j++) {
            boundary[x_spawn-i][y_spawn-j] = 10+zombie_id;
            boundary[x_spawn-i][y_spawn+j] = 10+zombie_id;
            boundary[x_spawn+i][y_spawn-j] = 10+zombie_id;
            boundary[x_spawn+i][y_spawn+j] = 10+zombie_id;
        }
    }*/
    return z;
}


void draw_zombie(int x, int y, int direction) {
    if (direction == UP_CODE) {
        draw_box(x, y, zombie_color);
    }
    if (direction == DOWN_CODE) {
        draw_box(x, y, zombie_color);
    }
    if (direction == LEFT_CODE) { 
        draw_box(x, y, zombie_color);
    }
    if (direction == RIGHT_CODE) { 
        draw_box(x, y, zombie_color);
    }
}

// save the position of a "object" two frames ago
void save_twoframes(int *prev_pos_x, int *prev_pox_y, int *prev2_pos_x, int *prev2_pos_y, int x_pos, int y_pos) {
    *prev2_pos_x = *prev_pos_x;
    *prev2_pos_y = *prev_pox_y;
    *prev_pos_x = x_pos;
    *prev_pox_y = y_pos;
}

void draw_projectile(int x, int y) {
    plot_pixel(x,y, 0xF176D7);
}
void draw_barrell(int x, int y){
	draw_box (x,y,barrell_color);
}

void calculate_healthBar(struct health *h, struct player play *p ){
    if((h->x- 50>= 0) && (h->x+ 50<= X_BOUND) && (h->y-6 >= 0) && h->y+6 <= Y_BOUND){
        int i, j;
        if(p->health = 100){
             for (i = 0; i < 50; i++) {
                 for (j = 0; j <= 6; j++) {
                     plot_pixel(x-i, y-j, green);
                     plot_pixel(x-i, y+j, green);
                     plot_pixel(x+i, y-j, green);
                     plot_pixel(x+i, y+j, green);
                 }
             }
        }
    }
    else{ 
        int k;
        for(k = 99; k >= p->health; k--){
            int i,j;
            for (i = 0; i<2; i++){
                for (j = 0; j<=6;j++){
                     plot_pixel(x-i, y-j, red);
                     plot_pixel(x-i, y+j, red);
                     plot_pixel(x+i, y-j, red);
                     plot_pixel(x+i, y+j, red);
                 }
             }

        }
    }	
}
void shoot_projectile(int byte1, int byte2, int byte3, struct projectile *p, struct player play) {
    if (byte1 == byte3 && byte2 == 0xF0);

    // when I is pressed, intialize the starting position of the projectile and set it to active 
    else if (byte3 == I_PRESS && !p->isActive) {
        boundary[p->x][p->y] = EMPTY_CODE;
        p->x = play.x;
        p->y = play.y;
        p->direction = play.direction;
        p->isActive = true;
        boundary[p->x][p->y] = PROJECTILE_CODE;
    }

    // update position of an active projectile
    switch (p->direction)
    {
    case UP_CODE:
        if (p->y > 9) p->y = p->y - 1;
        else p->isActive = false;
        break;
    case DOWN_CODE:
        if (p->y < Y_BOUND-9) p->y = p->y + 1;
        else p->isActive = false;
        break;
    case LEFT_CODE:
        if (p->x > 9) p->x = p->x - 1;
        else p->isActive = false;
        break;
    case RIGHT_CODE:
        if (p->x < X_BOUND-9) p->x = p->x + 1;
        else p->isActive = false;
        break;
    default:
        break;
    }
    // if boundary[x][y] is a int >= 10 then a zombie is in that position
	int i;
	for (i = 0; i<MAX_ZOMBIES; i++){
        if (((p->x >=zombies[i].x-2) && (p->x <=zombies[i].x+2)) && ((p->y >= zombies[i].y-2) && (p->y <= zombies[i].y+2))) {
           //printf("HIT ZOMBIE %d", zombie_id);
           zombies[i].health -= 1; // projectile hits zombie and the zombie's health will decrease
           if (zombies[i].health <= 0) { // if the zombie has <= 0 health then it is dead
              zombies[i].isAlive = false;
              num_zombies = num_zombies-1;								              
           }
			p->isActive = false;
			boundary[p->x][p->y] = EMPTY_CODE;
		}
    }
	int j;
   for (j = 0; j<MAX_BARRELLS; j++){
	   if(((p->x >=barrells[j].x-4) && (p->x <= barrells[j].x+4))&& ((p->y >= barrells[j].y-4)&& (p->y <= barrells[j].y+4))){
	     barrells[j].isActive = false; 
	     int k;
		 for(k=0; k< MAX_ZOMBIES; k++){
			 if(((barrells[j].x >= zombies[k].x-160)&&(barrells[j].x <= zombies[k].x+160))&&((barrells[j].y >= zombies[k].y-160)&&(barrells[j].y <= zombies[k].y+160))){
			   zombies[k].isAlive = false;
               num_zombies = num_zombies-1;
			 }
		 
		 if(((barrells[j].x >= play.x-160)&&(barrells[j].x <= play.x+160))&&((barrells[j].y >= play.y-160)&&(barrells[j].y >= play.y+160)))
		       play.health -= 1;
			  if (play.health <= 0){
				  play.isAlive = false;
			  }
		 }	    
	   }
		p->isActive = false;
	    boundary[p->x][p->y] = EMPTY_CODE;   
   }
}

void player_movement(int byte1, int byte2, int byte3, struct player *p) {
    if (byte2 == 0xF0) { // W and D pressed, go diagonally up and right
       return;
    }

    /*// diagonal movement
    if ((byte2 == D_PRESS && byte3 == W_PRESS) || (byte2 == W_PRESS && byte3 == D_PRESS)) { // W and D pressed, go diagonally up and right
        if (player_pos[0] < X_BOUND-7 && player_pos[1] >= 7) {
            boundary[player_pos[0]][player_pos[1]] = 0;
            player_pos[0] = player_pos[0] + dx;
            player_pos[1] = player_pos[1] - dy;
            boundary[player_pos[0]][player_pos[1]] = 1;
        }
        player_direction = 0;
        *RLEDs = 1;
    }

    else if ((byte2 == A_PRESS && byte3 == W_PRESS) || (byte2 == W_PRESS && byte3 == A_PRESS)) { // W and A pressed, go diagonally up and left
        if (player_pos[0] >= 7 && player_pos[1] >= 7) {
            boundary[player_pos[0]][player_pos[1]] = 0;
            player_pos[0] = player_pos[0] - dx;
            player_pos[1] = player_pos[1] - dy;
            boundary[player_pos[0]][player_pos[1]] = 1;
        }
        player_direction = 0;
        *RLEDs = 1;
    }

    else if ((byte3 == S_PRESS && byte2 == A_PRESS) || (byte2 == S_PRESS && byte3 == A_PRESS)) { // S and A pressed, go down and left
        if (player_pos[0] >=7 && player_pos[1] < Y_BOUND-7) {
            boundary[player_pos[0]][player_pos[1]] = 0;
            player_pos[1] = player_pos[1] + dx;
            player_pos[0] = player_pos[0] - dy;
            boundary[player_pos[0]][player_pos[1]] = 1;
        }
        player_direction = 1;
        *RLEDs = 2;
    }

    else if ((byte3 == S_PRESS && byte2 == D_PRESS) || (byte2 == S_PRESS && byte3 == D_PRESS)) { // S and D pressed, go down and right
        if (player_pos[1] < Y_BOUND-7 && player_pos[0] < X_BOUND-7) {
            boundary[player_pos[0]][player_pos[1]] = 0;
            player_pos[1] = player_pos[1] + dx;
            player_pos[0] = player_pos[0] + dy;
            boundary[player_pos[0]][player_pos[1]] = 1;
        }
        player_direction = 1;
        *RLEDs = 2;
    }*/

    // standard movement
    if (byte3 == W_PRESS) { // W pressed, go up
        if (p->y >= 7) {
            boundary[p->x][p->y] = EMPTY_CODE;
            p->y = p->y - dy;
            boundary[p->x][p->y] = PLAYER_CODE;
        } 
        p->direction = 0;
        *RLEDs = 1;
    }
    else if (byte3 == S_PRESS) { // S pressed, go down
        if (p->y < Y_BOUND-7) {
            boundary[p->x][p->y] = EMPTY_CODE;
            p->y = p->y + dy;
            boundary[p->x][p->y] = PLAYER_CODE;
        }
        p->direction = 1;
        *RLEDs = 2;
    }
    else if (byte3 == A_PRESS) { // A pressed, go left
        if (p->x >= 7) {
            boundary[p->x][p->y] = EMPTY_CODE;
            p->x = p->x - dx;
            boundary[p->x][p->y] = PLAYER_CODE;
        }
        p->direction = 2;
        *RLEDs = 4;
    }
    else if (byte3 == D_PRESS) { // D pressed, go right
        if (p->x < X_BOUND-7) {
            boundary[p->x][p->y] = EMPTY_CODE;
            p->x = p->x + dx;
            boundary[p->x][p->y] = PLAYER_CODE;
        }
        p->direction = 3;
        *RLEDs = 8;
    }

}

void draw_player(int x, int y, int direction) {

    if (direction == UP_CODE) { // player looking up
        draw_box(x, y, player_color);
    }
    if (direction == DOWN_CODE) { // player looking down
        draw_box(x, y, player_color);
    }
    if (direction == LEFT_CODE) { // player looking left
        draw_box(x, y, player_color);
    }
    if (direction == RIGHT_CODE) { // player looking right
        draw_box(x, y, player_color);
    }
    plot_pixel(x,y,0x0);
}

void wait_for_vsync() {
	volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
	int status;
	// set S bit to 1
	*pixel_ctrl_ptr = 1;
	// pixel_ctrl_ptr + 3 => 0xFF20302C
    
	status = *(pixel_ctrl_ptr + 3);
	// while (S == 1)
	while ((status & 0x01) != 0) {
		status = *(pixel_ctrl_ptr + 3);
	}
	return;
}

void clear_screen() {
	for (int i = 0; i < 320; i++)
		for(int j = 0; j < 240; j++)
			plot_pixel(i,j,0);
}
void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
void draw_line(int x0, int y0, int x1, int y1, short int color) {
	bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    int y_step;  
	
	if (is_steep) {
  		  swap(&x0, &y0);
          swap(&x1, &y1);			  
	}
     
	if (x0 > x1) { 
         swap(&x0, &x1);
         swap(&y0, &y1);
	}
	
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
	
    if (y0 < y1) {
	   y_step = 1;
    } else {
	   y_step = -1;
    }

    //for x from x0 to x1
    for (int x = x0; x <= x1; x++)	{ 
     	if (is_steep)	{
        plot_pixel(y, x, color);
     } else {
        plot_pixel(x, y, color);
	 }
		
     error = error + deltay; 
		
     if (error > 0) {
         y = y + y_step;
         error = error - deltax;
     }
  }
}

void plot_pixel(int x, int y, short int line_color)	{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

// draw 3x3 box centered at (x,y)
void draw_box(int x, int y, short int color) {
    int i, j;
    for (i = 0; i <= 6; i++) {
        for (j = 0; j <= 6; j++) {
            plot_pixel(x-i, y-j, color);
            plot_pixel(x-i, y+j, color);
            plot_pixel(x+i, y-j, color);
            plot_pixel(x+i, y+j, color);
        }
    }
}
	
	
	
	
	
	
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
};

struct zombie {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int prev2_x;
    int prev2_y;
    int direction;
    int health;
};

struct barrell {
    int x;
    int y;
};


// global variables
volatile int pixel_buffer_start; // global variable
int player_color = 0xD376D7;
int dx = 2;
int dy = 2;
int dx_projectile = 8;
int dy_projectile = 8;
int boundary[X_BOUND][Y_BOUND];

// global structs
struct player player1 = {X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2, X_BOUND/2, Y_BOUND/2, 0, 100};
struct projectile proj1 = {0,0,0,0,0,0,0,false};

// function prototypes
void save_twoframes(int *prev_pos_x, int *prev_pox_y, int *prev2_pos_x, int *prev2_pos_y, int x_pos, int y_pos);
void draw_projectile(int x, int y);
void shoot_projectile(int byte3, struct projectile *p, struct player play);
void player_movement(int byte1, int byte2, int byte3, struct player *p);
void draw_player(int x, int y, int direction);
void wait_for_vsync();
void clear_screen();
void swap(int *xp, int *yp);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void plot_pixel(int x, int y, short int line_color);
void draw_box(int x, int y, short int color);

int main(void) {
    // game setup
    // boundary
    int i, j;
    for (i = 0; i < X_BOUND; i++) {
        for (j = 0; j < Y_BOUND; j++) {
            boundary[i][j] = EMPTY_CODE;
        }
    }
    boundary[player1.x][player1.y] = 1;

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
        plot_pixel(proj1.prev2_x, proj1.prev2_y, 0x0);
        draw_projectile(proj1.x, proj1.y);
        
        // save the position of a "object" two frames ago
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
            printf("byte1 is %d, byte2 is %d, byte3 is %d \n", byte1, byte2, byte3);
		}
        player_movement(byte1, byte2, byte3, &player1);
        shoot_projectile(byte3, &proj1, player1);

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer


		/*if ((byte2 == 0xAA) && (byte3 == 0x00)) {
			// mouse inserted; initialize sending of data
			*(PS2_ptr) = 0xF4;
		}*/
	}
    
}

void save_twoframes(int *prev_pos_x, int *prev_pox_y, int *prev2_pos_x, int *prev2_pos_y, int x_pos, int y_pos) {
    *prev2_pos_x = *prev_pos_x;
    *prev2_pos_y = *prev_pox_y;
    *prev_pos_x = x_pos;
    *prev_pox_y = y_pos;
}

void draw_projectile(int x, int y) {
    plot_pixel(x,y,0xF176D7);
}

void shoot_projectile(int byte3, struct projectile *p, struct player play) {
    // when I is pressed, intialize the starting position of the projectile and set it to active 
    if (byte3 == I_PRESS && !p->isActive) {
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
    case 0:
        if (p->y > 4) p->y = p->y - dy_projectile;
        else p->isActive = false;
        break;
    case 1:
        if (p->y < Y_BOUND-4) p->y = p->y + dy_projectile;
        else p->isActive = false;
        break;
    case 2:
        if (p->x > 4) p->x = p->x - dx_projectile;
        else p->isActive = false;
        break;
    case 3:
        if (p->x < X_BOUND-4) p->x = p->x + dx_projectile;
        else p->isActive = false;
        break;
    default:
        break;
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

    if (direction == 0) { // player looking up
        draw_box(x, y, player_color);
    }
    if (direction == 1) { // player looking down
        draw_box(x, y, player_color);
    }
    if (direction == 2) { // player looking left
        draw_box(x, y, player_color);
    }
    if (direction == 3) { // player looking right
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
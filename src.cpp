// CHIP 8 Emulator/Intrepter

#include "pch.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "stdint.h"
#include <stdio.h>
#include "SDL.h"
#undef main









//fontset -
unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};








class Opcodes
{


	


public:
	//Declarations
	unsigned short opcode;			//Chip 8 has 35 opcodes, all two bytes long (an unsigned short has a length of 2 bytes.)
	unsigned char memory[4096];	    // Chip has 4k memory in total.
	unsigned char V[16];			// Chip 8 has 16 registers in all, ranging from V0 to VE, the 16th register is a 'carry flag'
	unsigned short I;				// Index register
	unsigned short pc;				// Program Counter
	bool drawFlag;


	/*
-------------------------MEMORY MAP----------------------------------------------------
0x000 - 0x1FF  -> Memory for the emulator/intrepter                                    |
0x050 - 0x0A0  -> For the 4x5 pixel font set (built in 0-F)							   |
0x200 - 0xFFF  -> For ROM and RAM                                                      |
																					   |
--------------------------------------------------------------------------------------*/

	unsigned char gfx[64 * 32];     //Screen array.



	//Chip 8 has 2 timers
	unsigned char delay_timer;
	unsigned char sound_timer;

	/*
	Chip 8's sepcification doesn't mention a stack,however it needs to be interpreted for the intrepter/emulator
	*/

	unsigned short stack[16];
	unsigned short sp;              //'sp' is the stack pointer.

	//Chip 8 has a hex based keypad 
	unsigned char key[16];


	void initialize()
	{
		//In this method all registers and memory are initialized once.




		//Initializations
		pc = 0x200;						//Chip-8 doesn't have a BIOS, however it loads the program at memory location 0x200.
		opcode = 0;						//Current Opcode reset.
		I = 0;							//Index register reset.
		sp = 0;							//Stack pointer reset.
		srand(time(NULL));


		for (int i = 0; i < 2048; ++i) 
		{
			gfx[i] = 0;					//Clearing video buffer
		}

		//Clearing the stack, keypad, and V registers
		for (int i = 0; i < 16; ++i) 
		{
			stack[i] = 0;
			key[i] = 0;
			V[i] = 0;
		}

		//Clearing memory
		
		for (int i = 0; i < 4096; ++i) {
			memory[i] = 0;
		}

		
		//Load font set into memory
		for (int i = 0; i < 80; ++i) {
			memory[i] = chip8_fontset[i];
		}

		//Reseting timers
		delay_timer = 0;
		sound_timer = 0;
		





	}
	


	void emulateCycle()
	{
		//This function is called whenever a cycle is execyted, this method Fetches-Decodes-Executes one opcode.

		/*
		Opcodes Fetch-Decode-Execute will be performed here.
		The 'memory' here is a unsigned char array, an char is one Byte long.
		A Chip-8 opcode is 2 bytes long.
		So we need to combine 2 elements in the 'memory' array in order to get the memory location of the opcode.
		This is done by using the following algorithm -

		opcode = memory[pc] << 8 | memory[pc + 1];

		EXPLANATION -
		(assume the opcodes to be in binary for now, in actuality memory locations are represented as hex.)
		('pc' is the program counter,it will point to the memory location of the next opcode in this case.)

		>First we perform a Left Shift on the first element of the 'memory' array which adds eight 0's to the right of the binary number.
		>Then we perform a logical OR operation to the new element with the newly aquired binary number which gives us the actual memory location of the desired opcode.
		*/

		//Fetch.
		opcode = memory[pc] << 8 | memory[pc + 1];

		//Decode.

		switch (opcode & 0xF000)
		{
		case 0x0000:
			switch (opcode & 0x000F)
			{
			case 0x0000:
				for (int i = 0; i < 2048; ++i) {
					gfx[i] = 0;
				}
				drawFlag = true;
				pc += 2;
				break;

				// 00EE - Return from subroutine
			case 0x000E:
				--sp;
				pc = stack[sp];
				pc += 2;
				break;

			default:
				printf("\nUnknown op code: %.4X\n", opcode);
				exit(3);
			}



			break;

		case 0x1000:
			pc = opcode & 0x0FFF;
			break;

		case 0x2000:
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;

		case 0x3000:
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			{
				pc += 4;
			}
			else
				pc += 2;
			break;

		case 0x4000:
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			{
				pc += 4;
			}
			else
				pc += 2;
			break;

		case 0x5000:

			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			{
				pc += 4;
			}

			else
				pc += 2;

			break;

		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
			break;

		case 0x7000:

			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			pc += 2;
			break;

		case 0x8000:
			switch (opcode & 0xF00F)
			{
			case 0x8000:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8001:
				//8XY1	BitOp	Vx=Vx|Vy
				V[opcode & 0x0F00 >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8002:
				//8XY2	BitOp	Vx=Vx&Vy	
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8003:
				//8XY3	BitOp	Vx = Vx ^ Vy
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8004:
				//8XY4	Math	Vx += Vy
				if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8005:
				//8XY5	Math	Vx -= Vy
				if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
					V[0xF] = 0; // there is a borrow
				else
					V[0xF] = 1;
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x8006:
				//8XY6	BitOp	Vx>>=1
				V[(opcode & 0x0F00) >> 8] >>= 1;
				pc += 2;
				break;

			case 0x8007:
				//8XY7	Math	Vx=Vy-Vx
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x800E:
				//8XYE	BitOp	Vx<<=1
				V[(opcode & 0x0F00) >> 8] <<= 1;
				pc += 2;
				break;

			default:
				std::cout << "Unknown opcode 0x" << opcode << std::endl;
				break;
			}
			break;

		case 0x9000:
			//9XY0	Cond	if (Vx != Vy)
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			{
				pc += 2;
			}
			break;

		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;

		case 0xB000:
			//BNNN	Flow	PC=V0+NNN
			pc = V[0x0000] + (opcode & 0x0FFF);
			pc += 2;
			break;

		case 0xC000:
			//CXNN	Rand	Vx=rand()&NN
			V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
			pc += 2;
			break;

		case 0xD000:
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
							V[0xF] = 1;
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc += 2;
		}
		break;

		case 0xE000:
			switch (opcode & 0x00FF)
			{

			case 0x009E:
				if (key[V[(opcode & 0x0F00) >> 8]] != 0)
					pc += 4;
				else
					pc += 2;
				break;


			case 0x00A1:

				if (key[V[(opcode & 0x0F00) >> 8]] == 0)
					pc += 4;
				else
					pc += 2;
				break;

			default:
				printf("Debug console:Unknown opcode [0xF000]: 0x%X\n", opcode);

				break;
			}

		case 0xF000:
			switch (opcode & 0x00FF)
			{

			case 0x0007:
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				pc += 2;
				break;


			case 0x000A:
			{
				bool key_pressed = false;

				for (int i = 0; i < 16; ++i)
				{
					if (key[i] != 0)
					{
						V[(opcode & 0x0F00) >> 8] = i;
						key_pressed = true;
					}
				}


				if (!key_pressed)
					return;

				pc += 2;

			}
			break;


			case 0x0015:
				delay_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;


			case 0x0018:
				sound_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;


			case 0x001E:

				if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
					V[0xF] = 1;
				else
					V[0xF] = 0;
				I += V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;


			case 0x0029:
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				pc += 2;
				break;


			case 0x0033:
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
				pc += 2;
				break;


			case 0x0055:
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
					memory[I + i] = V[i];


				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
				break;

			case 0x0065:
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
					V[i] = memory[I + i];


				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
				break;

			
			}


		}



		if (delay_timer > 0)
			--delay_timer;

		if (sound_timer > 0)
			if (sound_timer == 1)

			{


			
				std::cout << "BEEP!!" << std::endl;

				--sound_timer;
	         }
		



	}
	bool loadApplication(const char * filename)
	{
		initialize();
		printf("Loading: %s\n", filename);

		// Open file
		FILE * pFile = fopen(filename, "rb");
		if (pFile == NULL)
		{
			fputs("File error", stderr);
			return false;
		}

		// Check file size
		fseek(pFile, 0, SEEK_END);
		long lSize = ftell(pFile);
		rewind(pFile);
		printf("Filesize: %d\n", (int)lSize);

		// Allocate memory to contain the whole file
		char * buffer = (char*)malloc(sizeof(char) * lSize);
		if (buffer == NULL)
		{
			fputs("Memory error", stderr);
			return false;
		}

		// Copy the file into the buffer
		size_t result = fread(buffer, 1, lSize, pFile);
		if (result != lSize)
		{
			fputs("Reading error", stderr);
			return false;
		}

		// Copy buffer to Chip8 memory
		if ((4096 - 512) > lSize)
		{
			for (int i = 0; i < lSize; ++i)
				memory[i + 512] = buffer[i];
		}
		else
			printf("Error: ROM too big for memory");

		// Close file, free buffer
		fclose(pFile);
		free(buffer);

		return true;
	}

	

















};


// Keypad keymap
uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};


 int main(int argc, char **argv) {

	// Command usage
	if (argc != 2) {
		std::cout << "Usage: chip8 <ROM file>" << std::endl;
		return 1;
	}

	Opcodes chip8 = Opcodes();   // Initialise Chip8
	chip8.initialize();

	int w = 1024;                   // Window width
	int h = 512;                    // Window height

	
	SDL_Window* window = NULL;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}
	// Create window
	window = SDL_CreateWindow(
		"CHIP-8 Emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		w, h, SDL_WINDOW_SHOWN
	);
	if (window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n",
			SDL_GetError());
		exit(2);
	}

	
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, w, h);

	
	SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		64, 32);

	// Temporary pixel buffer
	uint32_t pixels[2048];


load:
	// Attempt to load ROM
	if (!chip8.loadApplication(argv[1]))
		return 2;

	// Emulation loop
	while(true)
	{
		chip8.emulateCycle();

		// Process SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) exit(0);

			// Process keydown events
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE)
					exit(0);

				if (e.key.keysym.sym == SDLK_F1)
					goto load;      

				for (int i = 0; i < 16; ++i) {
					if (e.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 1;
					}
				}
			}
			// Process keyup events
			if (e.type == SDL_KEYUP) {
				for (int i = 0; i < 16; ++i) {
					if (e.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 0;
					}
				}
			}
		}

		// If draw occurred, redraw SDL screen
		if (chip8.drawFlag) {
			chip8.drawFlag = false;

			
			for (int i = 0; i < 2048; ++i) {
				uint8_t pixel = chip8.gfx[i];
				pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			}
			
			SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
			// Clear screen and render
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		//To slow down emulation speed -
		std::this_thread::sleep_for(std::chrono::microseconds(4000));

	}
}

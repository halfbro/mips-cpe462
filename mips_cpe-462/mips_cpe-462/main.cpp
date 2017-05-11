#include <stdio.h>
#include <iostream>
//Using SDL and standard IO
#include <SDL2-2.0.5\include\SDL.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class Model {
private:
	int *data;
	int w, l, h;

public:
	Model(int width, int length, int height);
	~Model();
	
	int getIntensityAt(int x, int y, int z);
	void readFromFile(FILE *fd);
};

Model::Model(int width, int length, int height) {
	w = width;
	l = length;
	h = height;
	data = new int[w*l*h];
}

Model::~Model() {
	delete[] data;
}

int Model::getIntensityAt(int x, int y, int z) {
	if (x > w || x < 0) return 0;
	if (y > l || y < 0) return 0;
	if (z > h || z < 0) return 0;

	return data[x*l*h + y*h + z];
}

void Model::readFromFile(FILE *fd) {
	for (int i=0; i<h; ++i) {
		for (int j=0; j<l; ++j) {
			for (int k=0; k<w; ++k) {
				fscanf(fd, "%i", &data[i*w*l + j*w + k]);
			}
		}
	}
}

int main ( int argc, char *argv[] ) {

	if (argc != 2) {
		printf("Wrong number of arguments used, argument should be filename\n");
		getchar();
		return 0;
	}

	printf("Loading file %s\n", argv[1]);

	bool modelvalid = true;

	FILE *fd = fopen(argv[1], "r");
	if (fd == NULL) {
		printf("Error opening file\n");
		getchar();
		return 0;
	}

	int length, width, height;
	char fileheader[4];
	fscanf(fd, "%s %i %i %i", fileheader, &width, &length, &height);
	printf("w:%i l:%i h:%i\n", width, length, height);
	getchar();

	if ( !modelvalid ) {
		printf("File could not be read\n");
		getchar();
		return 0;
	}

	Model model(width, length, height);
	model.readFromFile(fd);

	 //The window we'll be rendering to
    SDL_Window* window = NULL;
    
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    } else {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        } else {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );
        }
	}

	bool quit = false;
	SDL_Event e;

	int orientation[3] = {0, 10, 0};

	while (!quit) {
		//Handle events on queue
        while( SDL_PollEvent( &e ) != 0 ) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEMOTION:
				// If left mouse button is held
				if (e.motion.state & SDL_BUTTON_LMASK) {
					orientation[0] += e.motion.xrel;
					orientation[2] -= e.motion.yrel;
				}
				printf("X: %i, Y: %i, Z: %i\n", orientation[0], orientation[1], orientation[2]);
				break;
			case SDL_MOUSEWHEEL:
				orientation[1] -= e.wheel.y;
				printf("X: %i, Y: %i, Z: %i\n", orientation[0], orientation[1], orientation[2]);
				break;
			}
        }

		SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );

		// TODO: get the image from Gary's projection
		//SDL_Surface* imageout = SDL_CreateRGBSurface( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0 );
		
		//Update the surface
        SDL_UpdateWindowSurface( window );
	}

	//Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();
	
	return 0;
}
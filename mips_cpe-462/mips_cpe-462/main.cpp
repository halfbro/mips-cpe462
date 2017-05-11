#include <stdio.h>
#include <iostream>
//Using SDL and standard IO
#include <SDL2-2.0.5\include\SDL.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

float lerp(float t, float a, float b){
	return (1 - t) * a + t * b;
}

void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}

class Model {
private:
	int *data;
	
public:
	int w, l, h;
	Model(int width, int length, int height);
	~Model();
	
	int getIntensityAt(int x, int y, int z);
	int getIntensityNear(float x, float y, float z);
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

int Model::getIntensityNear(float x, float y, float z) {
	if (x<0 || y<0 || z<0) return 0;
	if (x>w || y>l || z>h) return 0;
	int xLow = static_cast<int>(floor(x));
	int yLow = static_cast<int>(floor(y));
	int zLow = static_cast<int>(floor(z));
	
	float y0z0 = lerp(x-xLow, (float)getIntensityAt(xLow, yLow+0, zLow+0), (float)getIntensityAt(xLow+1, yLow+0, zLow+0));
	float y1z0 = lerp(x-xLow, (float)getIntensityAt(xLow, yLow+1, zLow+0), (float)getIntensityAt(xLow+1, yLow+1, zLow+0));
	float y0z1 = lerp(x-xLow, (float)getIntensityAt(xLow, yLow+0, zLow+1), (float)getIntensityAt(xLow+1, yLow+0, zLow+1));
	float y1z1 = lerp(x-xLow, (float)getIntensityAt(xLow, yLow+1, zLow+1), (float)getIntensityAt(xLow+1, yLow+1, zLow+1));

	float z0 = lerp(y-yLow, y0z0, y1z0);
	float z1 = lerp(y-yLow, y0z1, y1z1);

	float point = lerp(z-zLow, z0, z1);

	return static_cast<int>(floor(point));
}

int castRay(float x0, float y0, float z0, float dx, float dy, float dz, Model* m) {
	int max = 0;
	float x = x0;
	float y = y0;
	float z = z0;
	while (y >= -(m->l + 5.0)) {
		if (x>0 && x<m->w && y>0 && y<m->l && z>0 && z<m->h) {
			int intensity = m->getIntensityNear(x, y, z);
			if (intensity > max) max = intensity;
		}
		
		x += dx;
		y += dy;
		z += dz;
	}
	return max;
}


int main ( int argc, char *argv[] ) {

	if (argc != 2) {
		printf("Wrong number of arguments used, argument should be a single filename\n");
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
        window = SDL_CreateWindow( "MIP Ray Casting", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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

	int orientation[3] = {0, 30, 0};

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
				//printf("X: %i, Y: %i, Z: %i\n", orientation[0], orientation[1], orientation[2]);
				break;
			case SDL_MOUSEWHEEL:
				orientation[1] -= e.wheel.y;
				//printf("X: %i, Y: %i, Z: %i\n", orientation[0], orientation[1], orientation[2]);
				break;
			}
			SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );

			int a = orientation[0];
			int b = orientation[1];
			int c = orientation[2];
			double magnitude = sqrt((double) (a*a + b*b + c*c) );
			float norm[3] = {
				(float) -a/magnitude,
				(float) -b/magnitude,
				(float) -c/magnitude
			};
			//printf("X: %f, Y: %f, Z: %f\n", norm[0], norm[1], norm[2]);

			SDL_Surface* imageout = SDL_CreateRGBSurface( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0 );
			SDL_LockSurface(imageout);
			const float xstep = 0.2f;
			const float ystep = 0.2f;
			const float start[3] = {(float) a, (float) b, (float) c};
			float dx = -xstep*SCREEN_WIDTH/2;
			for (int xpixel = 0; xpixel < SCREEN_WIDTH; ++xpixel) {
				float dy = -ystep*SCREEN_HEIGHT/2;
				for (int ypixel = 0; ypixel < SCREEN_HEIGHT; ++ypixel) {
					int intensity = castRay(start[0]+dx, start[1], start[2]+dy, norm[0], norm[1], norm[2], &model);
					if (intensity < 0) intensity = 0;
					if (intensity > 255) intensity = 255;
					Uint8 b = (Uint8) intensity;
					Uint32 pixelval = (b<<24 | b<<16 | b<<8 | 0xff);
					put_pixel32(imageout, xpixel, ypixel, pixelval);
					dy += ystep;
				}
				dx += xstep;
				
			}
			SDL_UnlockSurface(imageout);
			SDL_BlitSurface(imageout, NULL, screenSurface, NULL);

			//Update the surface
			SDL_UpdateWindowSurface( window );
        }
	}

	//Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();
	
	return 0;
}
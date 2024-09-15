#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#undef main
#define true 1
#define false 0
#define PI 3.1415 

// screen size
unsigned const int SCREEN_W = 800;
unsigned const int SCREEN_H = 800;

// Window variables
SDL_Window* Window;
SDL_Surface* WindowSurface;

// Renderer
SDL_Renderer* WindowRenderer;

// Event handler
SDL_Event event;

// Images
SDL_Texture* walkTexture;
SDL_Texture* runTexture;
SDL_Texture* handgunTexture;
SDL_Texture* chainGunTexture;
SDL_Texture* chainGunShootTexture;
SDL_Texture* hkTexture;
SDL_Texture* hkRecoilTexture;
SDL_Texture* flamethrowerTexture;
SDL_Texture* shotgunTexture;

// Global variables
unsigned int running = true;

// Player variables
typedef struct {
	float x;
	float y;
	float velX;
	float velY;
	double angle;
	unsigned int animFrame;
	unsigned int animState;
	Uint32 lastFrame;
	unsigned int sprint;
} Player;
Player player;

int init(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL init error: %s\n",SDL_GetError());
		return false;
	}else{
		// Create Window
		Window = SDL_CreateWindow("SDL Shooter",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_W,SCREEN_H,SDL_WINDOW_SHOWN);
		if(Window == NULL){
			printf("SDL Window Surface error: %s\n",SDL_GetError());
			return false;
		}else{
			// Get Renderer
			WindowRenderer = SDL_CreateRenderer(Window,-1,SDL_RENDERER_ACCELERATED);
			if(WindowRenderer == NULL){
				printf("SDL Renderer error: %s\n",SDL_GetError());
				return false;
			}else{
				SDL_SetRenderDrawColor(WindowRenderer,0xFF,0xFF,0xFF,0xFF);
			}
			// Get Window surface
			WindowSurface = SDL_GetWindowSurface(Window);
		}
	}
	return true;
}

void quit(){
	SDL_DestroyTexture(walkTexture);
	SDL_DestroyTexture(runTexture);
	SDL_DestroyTexture(handgunTexture);
	SDL_DestroyTexture(chainGunTexture);
	SDL_DestroyTexture(chainGunShootTexture);
	SDL_DestroyTexture(hkTexture);
	SDL_DestroyTexture(hkRecoilTexture);
	SDL_DestroyTexture(flamethrowerTexture);
	SDL_DestroyTexture(shotgunTexture);
	walkTexture = NULL;
	runTexture = NULL;
	handgunTexture = NULL;
	chainGunTexture = NULL;
	chainGunShootTexture = NULL;
	hkTexture = NULL;
	hkRecoilTexture = NULL;
	flamethrowerTexture = NULL;
	shotgunTexture = NULL;
	
	SDL_DestroyRenderer(WindowRenderer);
	SDL_DestroyWindow(Window);
	WindowRenderer = NULL;
	Window = NULL;
	
	SDL_Quit();
}

void renderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a){SDL_SetRenderDrawColor(WindowRenderer,r,g,b,a);}
void fillRect(int x, int y, int w, int h){
	SDL_Rect rectToFill; rectToFill.x = x; rectToFill.y = y;rectToFill.w = w; rectToFill.h = h; SDL_RenderFillRect(WindowRenderer,&rectToFill);
}

SDL_Surface* loadBMP(char* filePath){
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface = SDL_LoadBMP(filePath);
	if(loadedSurface == NULL){
		printf("unable to load image %s! SDL error: %s\n", filePath, SDL_GetError());
		return NULL;
	}else{
		optimizedSurface = SDL_ConvertSurface(loadedSurface,WindowSurface->format,0);
		if(optimizedSurface == NULL){
			printf("unable to optimize image %s! SDL error: %s\n", filePath, SDL_GetError());
		}else{
			printf("Loaded successfully image %s!\n",filePath);
		}
		SDL_FreeSurface(loadedSurface);
	}
	return optimizedSurface;
}

SDL_Texture* loadTexture(char* filePath){
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(filePath);
	if(loadedSurface == NULL){
		printf("error loading image: %s, SDL_image error: %s\n",filePath,IMG_GetError());
	}else{
		newTexture = SDL_CreateTextureFromSurface(WindowRenderer,loadedSurface);
		if(newTexture == NULL){
			printf("Unable to make texture from: %s, SDL error: %s\n",filePath,SDL_GetError());
		}else{
			printf("Loaded successfully texture %s!\n",filePath);
		}
		SDL_FreeSurface(loadedSurface);
	}
	return newTexture;
}

void renderTexture(SDL_Texture* textureToRender, SDL_Rect* textureRect, SDL_Rect* textureSectionRect){
	SDL_RenderCopy(WindowRenderer,textureToRender,textureSectionRect,textureRect);
}
void renderTextureEx(SDL_Texture* textureToRender,SDL_Rect* textureRect,SDL_Rect* textureSectionRect,double angle,SDL_Point* center,SDL_RendererFlip flip){
	SDL_RenderCopyEx(WindowRenderer,textureToRender,textureSectionRect,textureRect,angle,center,flip);
}
SDL_Point getTextureSize(SDL_Texture* textureToMeasure){
	SDL_Point textureSize;
	SDL_QueryTexture(textureToMeasure,NULL,NULL,&textureSize.x,&textureSize.y);
	return textureSize;
}

double angle_to(double ax, double ay, double bx, double by){return atan2((by-ay),(bx-ax));}
double to_rad(double theta){return theta/180*PI;}
double to_deg(double theta){return theta/PI*180;}

void renderPlayer(){
	if(player.animState == 0){
		SDL_Point size = getTextureSize(walkTexture);
		SDL_Point scaledSize = {size.x/6*4,size.y*4};
		renderTextureEx(walkTexture,&((SDL_Rect){SCREEN_W/2-scaledSize.x/2,SCREEN_H/2-scaledSize.y/2,scaledSize.x,scaledSize.y}),&((SDL_Rect){size.x/6*player.animFrame,0,size.x/6,size.y}),0.0,NULL,SDL_FLIP_NONE);
	}
}

void render(){
	renderPlayer();
}

void update(){
	
}

void loadMedia(){
	walkTexture = loadTexture("assets/player_walk_strip6.png");
	runTexture = loadTexture("assets/player_run_strip6.png");
	handgunTexture = loadTexture("assets/player_9mmhandgun.png");
	chainGunTexture = loadTexture("assets/player_chaingun.png");
	chainGunShootTexture = loadTexture("assets/player_chaingun_shoot_strip2.png");
	hkTexture = loadTexture("assets/player_hk_stand.png");
	hkRecoilTexture = loadTexture("assets/player_hk_recoil.png");
	flamethrowerTexture = loadTexture("assets/player_flamethrower.png");
	shotgunTexture = loadTexture("assets/player_pumgun_reload_strip5.png");
}

int main(int argv, char* args[]){
	srand(time(NULL));
	
	if(!init()){return -1;}
	
	loadMedia();
	
	while(running){
		while(SDL_PollEvent(&event) != 0){
			if(event.type == SDL_QUIT){
				running = false;
			}if(event.type == SDL_KEYDOWN){
				switch(event.key.keysym.sym){
					case SDLK_w:
						break;
				}
			}
		}
		
		// clear renderer
		renderColor(0x00,0x00,0x00,0xFF);
		SDL_RenderClear(WindowRenderer);
		
		// update physics or type shi
		update();
		
		// render screen elements
		render();
		
		// update renderer
		SDL_RenderPresent(WindowRenderer);
	}
}
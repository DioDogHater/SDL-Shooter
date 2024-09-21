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
const Uint8* keyboard;

// Fonts
TTF_Font* gFont;
SDL_Texture* gTextTexture;

// all structs
typedef struct {
	char* filePath;
	SDL_Texture* texture;
	SDL_Point textureSize;
	SDL_Point center;
	unsigned int frames;
	unsigned int animFrame;
} Animation;

// Assets
Animation walkTexture = {"assets/player_walk_strip6.png",NULL,(SDL_Point){0,0},(SDL_Point){17.5,28.5},6,0};
Animation runTexture = {"assets/player_run_strip6.png",NULL,(SDL_Point){0,0},(SDL_Point){45.0,43.5},6,0};
Animation handgunTexture = {"assets/player_9mmhandgun.png",NULL,(SDL_Point){0,0},(SDL_Point){25.5,30.0},1,0};
Animation chainGunTexture = {"assets/player_chaingun.png",NULL,(SDL_Point){0,0},(SDL_Point){17.5,16.0},1,0};
Animation chainGunShootTexture = {"assets/player_chaingun_shoot_strip2.png",NULL,(SDL_Point){0.0,0.0},(SDL_Point){17.5,15.0},2,0};
Animation hkTexture = {"assets/player_hk_stand.png",NULL,(SDL_Point){0,0},(SDL_Point){31.5,26.5},1,0};
Animation hkRecoilTexture = {"assets/player_hk_recoil.png",NULL,(SDL_Point){0,0},(SDL_Point){31.5,26.5},1,0};
Animation flamethrowerTexture = {"assets/player_flamethrower.png",NULL,(SDL_Point){0,0},(SDL_Point){17.5,28.5},1,0};
Animation shotgunTexture = {"assets/player_pumgun_reload_strip5.png",NULL,(SDL_Point){0,0},(SDL_Point){31.5,28.5},5,0};
Animation* animations[9] = {&walkTexture,&runTexture,&handgunTexture,&chainGunTexture,&chainGunShootTexture,&hkTexture,&hkRecoilTexture,&flamethrowerTexture,&shotgunTexture};

// Global variables
unsigned int running = true;

// Player variables
typedef struct {
	float x;
	float y;
	float velX;
	float velY;
	double angle;
	unsigned int animState;
	Uint32 lastFrame;
	unsigned int sprint;
} Player;
Player player = {400,400,0.0,0.0,0.0,1,0,true};

// initialize all SDL variables
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
			//Initialize PNG Loading
			int imageFlags = IMG_INIT_PNG;
			if(!(IMG_Init(imageFlags) & imageFlags)){
				printf("SDL_image initializing failed, error: %s\n",IMG_GetError());
				return false;
			}
			//Initialize SDL_ttf
			if(TTF_Init() == -1){
				printf("SDL_ttf initializing failed, error %s\n",TTF_GetError());
				return false;
			}
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
	keyboard = SDL_GetKeyboardState(NULL);
	return true;
}

// function called when window closed
void quit(){
	for(int i = 0; i < sizeof(animations)/sizeof(Animation*); i++){
		SDL_DestroyTexture(animations[i]->texture);
		animations[i]->texture = NULL;
	}
	
	TTF_CloseFont(gFont);
	gFont = NULL;
	
	SDL_DestroyRenderer(WindowRenderer);
	SDL_DestroyWindow(Window);
	WindowRenderer = NULL;
	Window = NULL;
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

// Renderer geometry functions
void renderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a){SDL_SetRenderDrawColor(WindowRenderer,r,g,b,a);}
void fillRect(int x, int y, int w, int h){
	SDL_Rect rectToFill = {x,y,w,h}; SDL_RenderFillRect(WindowRenderer,&rectToFill);
}

// Surface/Texture related functions
SDL_Surface* loadImage(char* filePath){
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface = IMG_Load(filePath);
	if(loadedSurface == NULL){
		printf("unable to load image %s! SDL error: %s\n", filePath, IMG_GetError());
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
SDL_Texture* loadText(char* str, SDL_Color textColor){
	SDL_Texture* textTexture = NULL;
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont,str,textColor);
	if(textSurface == NULL){
		printf("Unable to render text surface! SDL_ttf error: %s\n",TTF_GetError());
	}else{
		textTexture = SDL_CreateTextureFromSurface(WindowRenderer,textSurface);
		if(textTexture == NULL){
			printf("Unable to create texture from surface! SDL error: %s",SDL_GetError());
		}
		SDL_FreeSurface(textSurface);
	}
	return textTexture;
}
int loadFont(char* filePath, int size){
	TTF_Font* newFont = TTF_OpenFont(filePath,size);
	if(newFont == NULL){
		printf("Failed to load %s! SDL_ttf error: %s!\n",filePath,TTF_GetError());
		return false;
	}else{
		gFont = newFont;
		TTF_CloseFont(newFont);
		newFont = NULL;
		return true;
	}
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

// Trigonometric functions
double angle_to(double ax, double ay, double bx, double by){return atan2((by-ay),(bx-ax));}
double to_rad(double theta){return theta/180*PI;}
double to_deg(double theta){return theta/PI*180;}
double lerp(double a, double b, double t){return a+((b-a)*t);}

// Player related functions
void changeAnimState(int newAnimState){if(player.animState == newAnimState)return;animations[player.animState]->animFrame = 0; player.animState = newAnimState;}
void renderPlayer(){
	// Sizes and stuff
	Animation* currAnim = animations[player.animState];
	SDL_Point textureSize = currAnim->textureSize;
	float textureCellWidth = textureSize.x/currAnim->frames;
	SDL_Point plSize = {textureCellWidth*2,textureSize.y*2};
	SDL_Point plCenter = currAnim->center;
	
	// Rotate player to point to mouse
	int mouseX; int mouseY; SDL_GetMouseState(&mouseX,&mouseY);
	player.angle = to_deg(angle_to((double)player.x-plCenter.x*2+(plCenter.x*2),(double)player.y-plCenter.y*2+(plCenter.y*2),(double)mouseX,(double)mouseY));
	
	// Render player with rotation and flipping
	renderTextureEx(
		currAnim->texture,
		&((SDL_Rect){player.x-plCenter.x*2,player.y-plCenter.y*2,plSize.x,plSize.y}),
		&((SDL_Rect){currAnim->animFrame*round(textureCellWidth),0,textureCellWidth,textureSize.y}),
		player.angle,
		&((SDL_Point){plCenter.x*2,plCenter.y*2}),
		SDL_FLIP_NONE
	);
}

void render(double deltaTime){
	// render player
	renderPlayer();
	
	// render all map items (WIP)
	renderTexture(gTextTexture,&((SDL_Rect){100,100,400,26}),NULL);
}

void updateAnimFrames(Uint32 currTicks, Animation* currAnim, unsigned int frameRate){
	if(currTicks-player.lastFrame > frameRate){
		currAnim->animFrame++;
		if(currAnim->animFrame >= currAnim->frames) currAnim->animFrame = 0;
		player.lastFrame = currTicks;
	}
}void update(double deltaTime){
	// change animations based on movement
	if(player.animState == 0 && (player.velX != 0.0 || player.velY != 0.0) && player.sprint)changeAnimState(1);
	else if(player.animState == 1 && (!player.sprint || (player.velX == 0.0 && player.velY == 0.0)))changeAnimState(0);
	
	// move player
	player.x += player.velX*(1+player.sprint*0.75)*deltaTime; player.y += player.velY*(1+player.sprint*0.75)*deltaTime;
	
	// update animations
	Uint32 currTicks = SDL_GetTicks();
	Animation* currAnim = animations[player.animState];
	if(currAnim->frames > 1){
		if(player.animState <= 1 && (player.velX != 0.0 || player.velY != 0.0)){updateAnimFrames(currTicks,currAnim,100);return;}
		if(player.animState == 4){updateAnimFrames(currTicks,currAnim,75);return;}
		if(player.animState == 8 && player.sprint /* change with reload timer or wtv */){updateAnimFrames(currTicks,currAnim,75);return;}
	}
	// ignore frame
	animations[player.animState]->animFrame = 0;
	player.lastFrame = currTicks;
}

void loadMedia(){
	// load all animation frames
	for(int i = 0; i < sizeof(animations)/sizeof(Animation*); i++){
		animations[i]->texture = loadTexture(animations[i]->filePath);
		if(animations[i]->texture != NULL){
			animations[i]->textureSize = getTextureSize(animations[i]->texture);
			printf("%s -> W: %d, H: %d",animations[i]->filePath,animations[i]->textureSize.x,animations[i]->textureSize.y);
			if(animations[i]->frames > 1){
				printf(" || Cell size: %d",animations[i]->textureSize.x/animations[i]->frames);
			}printf("\n");
		}
	}
	//load text
	gTextTexture = loadText("assets/arial.ttf",(SDL_Color){255,255,255});
}

int main(int argv, char* args[]){
	// initialize randomness
	srand(time(NULL));
	
	if(!init()){return -1;}
	
	// load all assets
	loadMedia();
	loadFont("assets/arial.ttf",26);
	
	// initialize deltaTime
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;
	double deltaTime = 0;
	
	while(running){
		// Update delta time
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		deltaTime = (double)((NOW-LAST)*1000/(double)SDL_GetPerformanceFrequency());
		
		// Handle events
		while(SDL_PollEvent(&event) != 0){
			if(event.type == SDL_QUIT){
				running = false;
			}if(event.type == SDL_KEYDOWN){
				switch(event.key.keysym.sym){
					case SDLK_w:
						player.velY = -0.2;
						break;
					case SDLK_s:
						player.velY = 0.2;
						break;
					case SDLK_a:
						player.velX = -0.2;
						break;
					case SDLK_d:
						player.velX = 0.2;
						break;
					case SDLK_1:
						if(player.animState == 2){changeAnimState(0);}else{changeAnimState(2);}
						break;
					case SDLK_2:
						if(player.animState == 5 || player.animState == 6){changeAnimState(0);}else{changeAnimState(5);}
						break;
					case SDLK_3:
						if(player.animState == 8){changeAnimState(0);}else{changeAnimState(8);}
						break;
					case SDLK_4:
						if(player.animState == 3 || player.animState == 4){changeAnimState(0);}else{changeAnimState(3);}
						break;
				}
			}if(event.type == SDL_KEYUP){
				switch(event.key.keysym.sym){
					case SDLK_w:
						if(player.velY < 0)player.velY = 0.0;
						break;
					case SDLK_s:
						if(player.velY > 0)player.velY = 0.0;
						break;
					case SDLK_a:
						if(player.velX < 0)player.velX = 0.0;
						break;
					case SDLK_d:
						if(player.velX > 0)player.velX = 0.0;
				}
			}
			player.sprint = keyboard[SDL_SCANCODE_LSHIFT];
		}
		
		// clear renderer
		renderColor(0x00,0x00,0x00,0xFF);
		SDL_RenderClear(WindowRenderer);
		
		// update physics or type shi
		update(deltaTime);
		
		// render screen elements
		render(deltaTime);
		
		// update renderer
		SDL_RenderPresent(WindowRenderer);
	}
}
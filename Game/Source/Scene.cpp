#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Player.h"
#include "SceneIntro.h"
#include "Map.h"
#include "ModuleFadeToBlack.h"

#include <SDL_mixer\include\SDL_mixer.h>

#include "Defs.h"
#include "Log.h"

Scene::Scene() : Module()
{
	name.Create("scene");
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	// Load map
	app->SetLastScene((Module*)this);

	app->player->Init();
	app->player->Start();

	app->map->active = true;

	app->map->Load("Mapa_PixelArt.tmx");
	// Load music
	app->audio->PlayMusic("Assets/audio/music/LOKI_8bits.ogg");
	img = app->tex->Load("Assets/textures/sky.png");
	animationFather.texture = app->tex->Load("Assets/textures/Dino_Orange.png");
	
	animationFather.position = { 2352, 495 };
	idleAnim.loop = true;
	idleAnim.speed = 0.025;

	for (int i = 0; i < 4; i++)
		idleAnim.PushBack({ 117 * i,0, 117, 117 });

	animationFather.currentAnimation = &idleAnim;

	SDL_QueryTexture(img, NULL,NULL,&imgW,&imgH);

	app->render->camera.y -= imgH;

	return true;
}

void Scene::SetDebugCollaider(bool value)
{
	if (value == NULL)
		debugCollisions = !debugCollisions;
	else
		debugCollisions = value;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	//DEBUG KEYS
	DebugKeys();

	//Draw Background
	Parallax();

	// Draw map
	app->map->Draw();

	iPoint vec;
	vec.x = 0, vec.y = 0;
	app->input->GetMousePosition(vec.x, vec.y);

	animationFather.currentAnimation->Update();

	if (app->player->CheckVictory({ app->player->playerData.position.x + app->player->playerData.velocity + 48,
		app->player->playerData.position.y }) && victory == false)
	{
		LOG("Congratulations, YOU WIN!");
		victory = true;
	}
	if (app->player->CheckGameOver() && lose == false && app->player->godMode == false)
	{
		LOG("GAME OVER!");
		lose = true;
	}
	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;
	SDL_Rect rectFather;
	rectFather = animationFather.currentAnimation->GetCurrentFrame();

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;
	if (victory == true)
	{
		victory = false;
		app->fade->FadeToBlack(this, (Module*)app->sceneWin, 0);
		return true;
	}
	if (lose == true)
	{
		lose = false;
		app->fade->FadeToBlack(this, (Module*)app->sceneLose, 0);
		return true;
	}
	app->render->DrawTextureFlip(animationFather.texture, animationFather.position.x, animationFather.position.y - (rectFather.h), &rectFather);
	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	if (!active)
		return true;

	LOG("Freeing scene");
	Mix_HaltMusic();
	app->map->CleanUp();
	app->tex->UnLoad(img);
	app->tex->UnLoad(animationFather.texture);
	app->player->CleanUp();

	active = false;
	return true;
}

void Scene::Parallax()
{
	speedImg = -0.9f;
	imgX = (int)(app->render->camera.x / 6) - 10;
	imgX *= speedImg;

	imgY = (int)((app->render->camera.y / 6) + 1250) * 0.2f;

	app->render->DrawTexture(img, imgX, imgY);
}

void Scene::DebugKeys()
{
	if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		app->render->camera.x = 0;
		app->player->playerData.position = app->player->positionInitial;
		app->player->playerData.direction = WALK_R;
		Mix_RewindMusic();
	}

	if (app->input->GetKey(SDL_SCANCODE_F7) == KEY_DOWN)
		app->SaveConfigRequested();

	if (app->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		app->LoadConfigRequested();

	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		app->SaveGameRequest();

	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		app->LoadGameRequest();

	if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) {
		SetDebugCollaider();
	}

	if (app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		if (app->player->godMode == false)app->player->godMode = true;
		else app->player->godMode = false;
	}
}

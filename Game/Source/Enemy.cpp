#include "Enemy.h"
#include "Player.h"
#include "Pathfinding.h"

#include "Defs.h"
#include "Log.h"

Enemy::Enemy() : Entity()
{
	name.Create("Enemy");
}

Enemy::Enemy(TypeEntity pTypeEntity, iPoint pPosition, float pVelocity, SDL_Texture* pTexture)
	: Entity(pTypeEntity, pPosition, pVelocity, pTexture)
{
	lastPath = new DynArray<iPoint>();
	name.Create("Enemy");
}


Enemy::~Enemy()
{}

bool Enemy::Start()
{
	active = true;
	entityData->texture = app->tex->Load("Assets/Textures/enemy_walk.png");

	entityData->velocity = 1;

	idleAnim->loop = true;
	idleAnim->speed = 0.04f;

	for (int i = 0; i < 8; i++)
		idleAnim->PushBack({ (48 * i),0, 48, 48 });

	walkAnim->loop = true;
	walkAnim->speed = 0.04f;

	for (int i = 0; i < 4; i++)
		walkAnim->PushBack({ (48 * i),48, 48, 48 });

	deadAnim->loop = false;
	deadAnim->speed = 0.08f;

	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 2; i++)
			deadAnim->PushBack({ 192 + (67 * i),48, 67, 48 });
	}
	
	checkDestination->Start();

	entityData->currentAnimation = idleAnim;

	if (entityData->type == GROUND_ENEMY)
	{
		entityData->numPoints = 4;
		entityData->pointsCollision = new iPoint[4]{ { 0, 0 }, { 48 , 0 }, { 48,-48 }, { 0 ,-48 } };
	}
	if (entityData->type == AIR_ENEMY)
	{
		entityData->numPoints = 4;
		entityData->pointsCollision = new iPoint[4]{ { 0, 0 }, { 48 , 0 }, { 48,-48 }, { 0 ,-48 } };
	}

	destination=app->map->WorldToMap(app->player->playerData.position.x, app->player->playerData.position.y);

	return true;
}

bool Enemy::Awake(pugi::xml_node& config)
{
	LOG("Loading Enemy Parser");
	bool ret = true;
	
	return ret;
}
bool Enemy::Radar(iPoint distance)
{
	if (entityData->position.DistanceManhattan(distance) < range) return true;

	return false;
}
bool Enemy::PreUpdate()
{
	//app->pathfinding->ComputePathAStar();
	iPoint currentPositionPlayer = app->player->playerData.position;
	if (Radar(currentPositionPlayer) && entityData->state != DEADING)
	{
		if (isDetected == false)app->pathfinding->ResetPath(app->map->WorldToMap(entityData->position.x, entityData->position.y)), isDetected = true;
		entityData->state = WALK;
		entityData->currentAnimation = walkAnim;

		iPoint auxPositionEnemey[4];
		for (int i = 0; i < 4; i++)
		{
			auxPositionEnemey[i] = { entityData->position.x + entityData->pointsCollision[i].x,
				entityData->position.y + entityData->pointsCollision[i].y };
		}
		iPoint auxPositionPlayer[6];
		for (int i = 0; i < 6; i++)
		{
			auxPositionPlayer[i] = {currentPositionPlayer.x + app->player->playerData.pointsCollision[i].x,
				-48+currentPositionPlayer.y + app->player->playerData.pointsCollision[i].y };

		}
		if (collision.IsInsidePolygons(auxPositionPlayer, app->player->playerData.numPoints, auxPositionEnemey, entityData->numPoints)&& collision.IsInsidePolygons(auxPositionEnemey, entityData->numPoints, auxPositionPlayer, app->player->playerData.numPoints))
		{
			entityData->state = DEADING;
			entityData->currentAnimation = deadAnim;
			app->player->SetHit();
		}
	}
	else if (entityData->state != DEADING)entityData->state = IDLE, entityData->currentAnimation = idleAnim, isDetected = false;
	if (entityData->state == DEADING && entityData->currentAnimation->HasFinished())pendingToDelete = true, entityData->state = DEAD;
	return true;
}
bool Enemy::Update(float dt)
{
	entityData->currentAnimation->Update();
	if (entityData->state == WALK)
	{
		//Direction
		if (entityData->position.x < app->player->playerData.position.x)entityData->direction = WALK_R;
		else entityData->direction = WALK_L;
		//If player move
		iPoint mapPositionEnemy = app->map->WorldToMap(entityData->position.x, entityData->position.y);
		iPoint worldPositionPalyer = app->player->playerData.position;
		iPoint mapPositionPalyer = app->map->WorldToMap(worldPositionPalyer.x, worldPositionPalyer.y);

		if (checkDestination->check(1000))
		{
			if (destination != mapPositionPalyer)
			{
				app->pathfinding->ResetPath(mapPositionEnemy);
				checkDestination->Start();
				app->pathfinding->ComputePathAStar(mapPositionEnemy, mapPositionPalyer);
				lastPath = app->pathfinding->GetLastPath();
				destination = mapPositionPalyer;
			}
		}
		//Move Enemy
		int i;
		for (i = 0; i < lastPath->Count(); i++)
		{
			if (mapPositionEnemy == iPoint({ lastPath->At(i)->x, lastPath->At(i)->y })) break;
		}

		if (lastPath->At(i + 1) != NULL)
		{
			iPoint nextPositionEnemy = *lastPath->At(i + 1);
			iPoint nextAuxPositionEenemy = MapToWorld(nextPositionEnemy);
			if (nextAuxPositionEenemy.x < entityData->position.x && (CheckCollision({mapPositionEnemy.x-1, mapPositionEnemy.y+1})==1 || CheckCollision({ mapPositionEnemy.x - 1, mapPositionEnemy.y + 2 }) == 1))
			{
				entityData->position.x -= entityData->velocity;
			}
			else if (nextAuxPositionEenemy.x > entityData->position.x && (CheckCollision({ mapPositionEnemy.x+1, mapPositionEnemy.y + 1 })==1 || CheckCollision({ mapPositionEnemy.x+1, mapPositionEnemy.y + 2 }) == 1))
			{
				entityData->position.x += entityData->velocity;
			}
			if (nextAuxPositionEenemy.y > entityData->position.y )
			{
				entityData->position.y += entityData->velocity+1;
			}
		}
		else //if the next position is destination continue with current direction
		{
			if(entityData->direction == WALK_R) entityData->position.x += entityData->velocity;
			else entityData->position.x -= entityData->velocity;

		}
		if (entityData->type == AIR_ENEMY)
		{
			if (lastPath->At(i + 1)->x < mapPositionEnemy.x)
			{
				entityData->position.x -= entityData->velocity;
			}
			else if (lastPath->At(i + 1)->x > mapPositionEnemy.x)
			{
				entityData->position.x += entityData->velocity;
			}
			if (lastPath->At(i + 1)->x < mapPositionEnemy.x)
			{
				entityData->position.y -= entityData->velocity;
			}
			else if (lastPath->At(i + 1)->x > mapPositionEnemy.x)
			{
				entityData->position.y += entityData->velocity;
			}
		}
	}

	return true;
}
bool Enemy::PostUpdate()
{

	SDL_Rect rectEnemy;
	rectEnemy = entityData->currentAnimation->GetCurrentFrame();
	// Draw player in correct direction
	if (entityData->direction == MoveDirection::WALK_L)
		app->render->DrawTexture(entityData->texture, entityData->position.x, entityData->position.y, &rectEnemy);
	if (entityData->direction == MoveDirection::WALK_R)
		app->render->DrawTextureFlip(entityData->texture, entityData->position.x - (rectEnemy.w - idleAnim->frames->w), entityData->position.y, &rectEnemy);

	return true;
}

bool Enemy::CleanUp()
{

	if (!active)
		return true;
	pendingToDelete = true;
	app->tex->UnLoad(entityData->texture);
	active = false;

	return true;
}
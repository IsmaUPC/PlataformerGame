#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Point.h"
#include "Module.h"
#include "Animation.h"


#include "PugiXml\src\pugixml.hpp"


enum State {

	IDLE,
	JUMP,
	WALK,
	RUN,
};

enum MoveDirection {

	WALK_L,
	WALK_R,
};

struct PlayerData
{
	iPoint position;
	State state;
	MoveDirection direction;
	Animation* currentAnimation;
	int velocity;
	bool isJumped;
	bool isJumpedAgain;
	
	SDL_Texture* texture;
	static const int numPoints = 6;
	int pointsCollision[numPoints][2] = { { 1,0 },{47 , 0},{ 1,-54 },{47 ,-54 }, {1,-47},{47,-47} };
};


class Player : public Module
{
public:

	Player();

	virtual ~Player();

	bool Awake(pugi::xml_node&);

	bool Start();

	bool PreUpdate();

	bool Update(float dt);

	void PlayerMoveAnimation();

	void PlayerControls();

	void Jump();

	void MovePlayer(MoveDirection playerDirection);

	

	bool PostUpdate();

	bool CleanUp();

	bool Load(const char* path);

	bool CollisionPlayer(iPoint nextPosition);

	bool CheckCollision(iPoint positionMapPlayer);

	bool CollisionJumping(iPoint positionMapPlayer);

private:

	bool LoadPlayer();
	void Fallings();
	void MoveToDirection(int velocity);

public:


	PlayerData playerData;
	
private:

	Animation idleAnim;
	Animation* walkAnim = new Animation();
	Animation* atakAnim = new Animation();
	Animation* damageAnim = new Animation();
	Animation* runAnim = new Animation( );
	Animation* jumpAnim = new Animation( );

	float velY=0;
	int velWalk;

	pugi::xml_document playerFile;
	SString folder;
	iPoint tmp;
	int nextYDown;
	int nextYUp;

};

#endif // _PLAYER_H_



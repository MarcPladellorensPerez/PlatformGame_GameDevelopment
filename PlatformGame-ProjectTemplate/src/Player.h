#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();

	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	// Load player parameters from XML
	bool LoadParameters(pugi::xml_node parameters);

private:

	void GetPhysicsValues();
	void Move();
	void Jump();
	void Teleport();
	void ApplyPhysics();
	void Draw(float dt);

public:

	// Player parameters (loaded from XML)
	float speed = 4.0f;
	float jumpForce = 2.5f;

	SDL_Texture* texture = NULL;
	int texW, texH;

	// Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	bool isJumping = false; // Flag to check if the player is currently jumping

private:
	b2Vec2 velocity;
	AnimationSet anims;

	// Configuration paths
	std::string texturePath;
	std::string animTsxPath;
	std::string pickCoinFxPath;
};
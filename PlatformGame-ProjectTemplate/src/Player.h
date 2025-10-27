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
	void MoveGodMode();
	void Jump();
	void DoubleJump();
	void Dash();
	void Teleport();
	void ApplyPhysics();
	void Draw(float dt);
	void UpdateCamera();  // Función separada para actualizar la cámara

public:

	// Player parameters (loaded from XML)
	float speed = 4.0f;
	float jumpForce = 2.5f;
	float dashForce = 10.0f;
	int dashDuration = 200;      // milliseconds
	int dashCooldown = 500;      // milliseconds

	SDL_Texture* texture = nullptr;
	int texW = 32;
	int texH = 32;

	// Audio fx
	int pickCoinFxId = 0;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody = nullptr;
	bool isJumping = false;       // Flag to check if the player is currently jumping
	bool hasDoubleJump = false;   // Flag for double jump availability
	bool spaceWasReleased = false; // Flag to track if space was released after first jump

	// Dash state
	bool isDashing = false;
	float dashTimer = 0.0f;       // Current dash duration timer
	float dashCooldownTimer = 0.0f; // Cooldown timer
	int dashDirection = 0;        // -1 left, 1 right, 0 none

	// God mode
	bool godMode = false;

private:
	b2Vec2 velocity = { 0.0f, 0.0f };
	AnimationSet anims;

	// Configuration paths
	std::string texturePath;
	std::string animTsxPath;
	std::string pickCoinFxPath;
};
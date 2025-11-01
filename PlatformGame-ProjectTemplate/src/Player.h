#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>

struct SDL_Texture;
struct Checkpoint;

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
	void UpdateCamera();

	// Sistema de muerte y respawn
	void CheckDeath();
	void Die();
	void Respawn();

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
	bool isJumping = false;
	bool hasDoubleJump = false;
	bool spaceWasReleased = false;

	// Dash state
	bool isDashing = false;
	float dashTimer = 0.0f;
	float dashCooldownTimer = 0.0f;
	int dashDirection = 0;

	// God mode
	bool godMode = false;

	// Death system
	bool isDead = false;
	float respawnTimer = 0.0f;
	float respawnDelay = 1000.0f; // 1 second delay
	Vector2D spawnPosition;       // Initial spawn position

	void ActivateCheckpoint(Checkpoint* checkpoint);

private:
	b2Vec2 velocity = { 0.0f, 0.0f };
	AnimationSet anims;
	Checkpoint* currentCheckpoint = nullptr;
	float checkpointRadius = 32.0f;
	void CheckCheckpoints();


	// Configuration paths
	std::string texturePath;
	std::string animTsxPath;
	std::string pickCoinFxPath;
};
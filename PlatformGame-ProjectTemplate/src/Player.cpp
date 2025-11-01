#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {
	return true;
}

bool Player::LoadParameters(pugi::xml_node parameters) {

	// Load initial position (SPAWN POSITION)
	position.setX(parameters.child("position").attribute("x").as_float());
	position.setY(parameters.child("position").attribute("y").as_float());

	spawnPosition = position;

	// Load movement parameters
	speed = parameters.child("movement").child("speed").attribute("value").as_float();
	jumpForce = parameters.child("movement").child("jumpForce").attribute("value").as_float();

	// Load advanced movement parameters
	dashForce = parameters.child("movement").child("dashForce").attribute("value").as_float();
	dashDuration = parameters.child("movement").child("dashDuration").attribute("value").as_int();
	dashCooldown = parameters.child("movement").child("dashCooldown").attribute("value").as_int();

	// Load texture info
	texturePath = parameters.child("texture").attribute("path").as_string();
	texW = parameters.child("texture").attribute("width").as_int();
	texH = parameters.child("texture").attribute("height").as_int();

	// Load animation path
	animTsxPath = parameters.child("animations").attribute("tsx_path").as_string();

	// Load audio paths
	pickCoinFxPath = parameters.child("audio").child("fx").attribute("path").as_string();

	LOG("Player parameters loaded successfully");
	LOG("  Position: (%.2f, %.2f)", position.getX(), position.getY());
	LOG("  Spawn Position: (%.2f, %.2f)", spawnPosition.getX(), spawnPosition.getY());

	return true;
}

bool Player::Start() {

	// Load animations using the path from config
	std::unordered_map<int, std::string> aliases = { {0,"idle"},{11,"move"},{22,"jump"} };
	anims.LoadFromTSX(animTsxPath.c_str(), aliases);
	anims.SetCurrent("idle");

	// Load texture using the path from config
	texture = Engine::GetInstance().textures->Load(texturePath.c_str());

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	// Initialize audio effect using the path from config
	pickCoinFxId = Engine::GetInstance().audio->LoadFx(pickCoinFxPath.c_str());

	return true;
}

bool Player::Update(float dt)
{
	if (isDead) {
		respawnTimer -= dt;
		if (respawnTimer <= 0.0f) {
			Respawn();
		}
		return true;
	}

	GetPhysicsValues();

	// Different movement logic for God Mode
	if (godMode) {
		MoveGodMode();
	}
	else {
		// Update dash cooldown
		if (dashCooldownTimer > 0.0f) {
			dashCooldownTimer -= dt;
		}

		// Update dash state
		if (isDashing) {
			dashTimer -= dt;
			if (dashTimer <= 0.0f) {
				isDashing = false;
			}
		}

		Move();
		Jump();
		DoubleJump();
		Dash();

		CheckDeath();
	}
	CheckCheckpoints();

	Teleport();
	ApplyPhysics();
	Draw(dt);

	return true;
}

void Player::Teleport() {
	// Teleport the player to spawn position
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_T) == KEY_DOWN) {
		Respawn();
		LOG("Player teleported to spawn position");
	}
}

void Player::CheckDeath() {
	// Get map size
	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();

	// Death by falling (below map)
	if (position.getY() > mapSize.getY() + 100.0f) {
		LOG("Player died: Fell off the map!");
		Die();
	}

	// Death by going above map (optional, uncomment if needed)
	// if (position.getY() < -100.0f) {
	//     LOG("Player died: Went above the map!");
	//     Die();
	// }
}

void Player::Die() {
	if (isDead || godMode) return; // Can't die in god mode

	isDead = true;
	respawnTimer = respawnDelay;

	// Stop all movement
	Engine::GetInstance().physics->SetLinearVelocity(pbody, 0.0f, 0.0f);

	// Reset states
	isJumping = false;
	hasDoubleJump = false;
	spaceWasReleased = false;
	isDashing = false;

	LOG("=== PLAYER DIED ===");
	LOG("Respawning in %.2f ms...", respawnDelay);
}

void Player::Respawn() {
	LOG("=== PLAYER RESPAWNED ===");

	isDead = false;
	respawnTimer = 0.0f;

	// Reset velocity
	Engine::GetInstance().physics->SetLinearVelocity(pbody, 0.0f, 0.0f);

	// Move to spawn position
	pbody->SetPosition((int)spawnPosition.getX(), (int)spawnPosition.getY());
	position = spawnPosition;

	// Reset states
	isJumping = false;
	hasDoubleJump = false;
	spaceWasReleased = false;
	isDashing = false;
	dashCooldownTimer = 0.0f;

	// Reset animation
	anims.SetCurrent("idle");

	LOG("Player respawned at (%.2f, %.2f)", spawnPosition.getX(), spawnPosition.getY());
}

void Player::GetPhysicsValues() {
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	lastYVelocity = velocity.y;

	if (!isDashing) {
		velocity = { 0.0f, velocity.y };
	}
}

void Player::Move() {
	if (isDashing) {
		return;
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -speed;
		anims.SetCurrent("move");
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = speed;
		anims.SetCurrent("move");
	}
}

void Player::MoveGodMode() {
	float godSpeed = speed * 2.0f;

	velocity = { 0.0f, 0.0f };

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		velocity.y = -godSpeed;
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		velocity.y = godSpeed;
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -godSpeed;
		anims.SetCurrent("move");
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = godSpeed;
		anims.SetCurrent("move");
	}

	if (velocity.x == 0.0f && velocity.y == 0.0f) {
		anims.SetCurrent("idle");
	}
}

void Player::Jump() {
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		anims.SetCurrent("jump");
		isJumping = true;
		hasDoubleJump = false;
		spaceWasReleased = false;
	}

	if (isJumping && !spaceWasReleased && !hasDoubleJump &&
		Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_IDLE) {
		spaceWasReleased = true;
		hasDoubleJump = true;
	}
}

void Player::DoubleJump() {
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN &&
		isJumping && spaceWasReleased && hasDoubleJump) {

		b2Vec2 currentVel = Engine::GetInstance().physics->GetLinearVelocity(pbody);
		float doubleJumpVelocity = -jumpForce * 1.8f;
		Engine::GetInstance().physics->SetLinearVelocity(pbody, currentVel.x, doubleJumpVelocity);
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce * 0.3f, true);

		anims.SetCurrent("jump");
		hasDoubleJump = false;
	}
}

void Player::Dash() {
	if (dashCooldownTimer <= 0.0f && !isDashing) {

		int desiredDashDir = 0;
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN ||
			Engine::GetInstance().input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_DOWN) {

			if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
				desiredDashDir = -1;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				desiredDashDir = 1;
			}
			else {
				desiredDashDir = 1;
			}
		}

		if (desiredDashDir != 0) {
			isDashing = true;
			dashTimer = static_cast<float>(dashDuration);
			dashCooldownTimer = static_cast<float>(dashCooldown);
			dashDirection = desiredDashDir;

			velocity.x = dashForce * static_cast<float>(dashDirection);

			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(
				pbody,
				dashForce * 0.5f * static_cast<float>(dashDirection),
				0.0f,
				true
			);
		}
	}
}

void Player::ApplyPhysics() {
	if (godMode) {
		b2Body_SetGravityScale(pbody->body, 0.0f);
		b2Body_SetType(pbody->body, b2_kinematicBody);
		Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
	}
	else {
		b2Body_SetGravityScale(pbody->body, 1.0f);
		b2Body_SetType(pbody->body, b2_dynamicBody);

		if (isOnOneWayPlatform && currentOneWayPlatform != nullptr) {
			int playerX, playerY;
			pbody->GetPosition(playerX, playerY);

			if (abs(playerY - oneWayPlatformY) > 2) {
				pbody->SetPosition(playerX, oneWayPlatformY);
			}

			velocity.y = 0.0f;
		}
		if (isDashing) {
			velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
		else if (isJumping) {
			velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
		else {
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
	}
}

void Player::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	int x, y;
	pbody->GetPosition(x, y);
	position.setX(static_cast<float>(x));
	position.setY(static_cast<float>(y));

	UpdateCamera();

	if (isDead) {

	}

	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);
}

void Player::UpdateCamera() {
	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();
	int cameraW = Engine::GetInstance().render->camera.w;
	int cameraH = Engine::GetInstance().render->camera.h;

	int desiredCameraX = static_cast<int>(-position.getX() + static_cast<float>(cameraW) / 4.0f);
	int desiredCameraY = static_cast<int>(-position.getY() + static_cast<float>(cameraH) / 2.0f);

	if (desiredCameraX > 0) {
		desiredCameraX = 0;
	}

	int maxCameraX = -(static_cast<int>(mapSize.getX()) - cameraW);
	if (desiredCameraX < maxCameraX) {
		desiredCameraX = maxCameraX;
	}

	if (desiredCameraY > 0) {
		desiredCameraY = 0;
	}

	int maxCameraY = -(static_cast<int>(mapSize.getY()) - cameraH);
	if (desiredCameraY < maxCameraY) {
		desiredCameraY = maxCameraY;
	}

	Engine::GetInstance().render->camera.x = desiredCameraX;
	Engine::GetInstance().render->camera.y = desiredCameraY;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	// Ignore collisions when dead or in god mode (except items)
	if (isDead || (godMode && physB->ctype != ColliderType::ITEM)) {
		return;
	}

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isJumping = false;
		hasDoubleJump = false;
		spaceWasReleased = false;
		anims.SetCurrent("idle");
		break;
	case ColliderType::PLATFORM_ONEWAY:
	{
		if (lastYVelocity > 0.0f) {
			int playerX, playerY;
			pbody->GetPosition(playerX, playerY);

			int platformX, platformY;
			physB->GetPosition(platformX, platformY);

			if (playerY < platformY - 8) {

				Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity.x, 0.0f);

				isJumping = false;
				hasDoubleJump = false;
				spaceWasReleased = false;
				anims.SetCurrent("idle");

				isOnOneWayPlatform = true;
				currentOneWayPlatform = physB;
				oneWayPlatformY = playerY;  

				pbody->GetPosition(playerX, playerY);
				oneWayPlatformY = playerY;

				LOG("Landed on ONE-WAY platform at Y: %d", playerY);
			}
		}
		break;
	}
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		break;

	case ColliderType::ENEMY:
		LOG("Player died: Hit damage object (spike/trap)!");
		Die();
		break;

	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;

	default:
		break;
	}
}

void Player::CheckCheckpoints() {
	Checkpoint* nearbyCheckpoint = Engine::GetInstance().map->GetCheckpointAt(
		position.getX(),
		position.getY(),
		checkpointRadius
	);

	if (nearbyCheckpoint != nullptr && !nearbyCheckpoint->activated) {
		ActivateCheckpoint(nearbyCheckpoint);
	}
}

void Player::ActivateCheckpoint(Checkpoint* checkpoint) {
	if (checkpoint == nullptr) return;

	if (currentCheckpoint != nullptr) {
		currentCheckpoint->activated = false;
	}

	checkpoint->activated = true;
	currentCheckpoint = checkpoint;

	spawnPosition.setX(checkpoint->x);
	spawnPosition.setY(checkpoint->y);

	LOG("=== CHECKPOINT ACTIVADO ===");
	LOG("Checkpoint: %s", checkpoint->name.c_str());
	LOG("Nueva posicion de respawn: (%.2f, %.2f)", spawnPosition.getX(), spawnPosition.getY());

}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	if (physB->ctype == ColliderType::PLATFORM_ONEWAY && physB == currentOneWayPlatform) {
		isOnOneWayPlatform = false;
		currentOneWayPlatform = nullptr;
		LOG("Left ONE-WAY platform");
	}
}
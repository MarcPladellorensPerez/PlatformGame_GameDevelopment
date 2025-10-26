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
	// Parameters are loaded in LoadParameters() now
	return true;
}

// Load parameters from XML
bool Player::LoadParameters(pugi::xml_node parameters) {

	// Load initial position
	position.setX(parameters.child("position").attribute("x").as_float());
	position.setY(parameters.child("position").attribute("y").as_float());

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
	LOG("  Speed: %.2f", speed);
	LOG("  Jump Force: %.2f", jumpForce);
	LOG("  Dash Force: %.2f", dashForce);
	LOG("  Texture: %s", texturePath.c_str());

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

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	// Initialize audio effect using the path from config
	pickCoinFxId = Engine::GetInstance().audio->LoadFx(pickCoinFxPath.c_str());

	return true;
}

bool Player::Update(float dt)
{
	GetPhysicsValues();

	// Different movement logic for God Mode
	if (godMode) {
		MoveGodMode();
	}
	else {
		// Update dash cooldown
		if (dashCooldownTimer > 0.0f) {
			dashCooldownTimer -= dt; // dt is in milliseconds
		}

		// Update dash state
		if (isDashing) {
			dashTimer -= dt; // dt is in milliseconds
			if (dashTimer <= 0.0f) {
				isDashing = false;
				LOG("Dash ended");
			}
		}

		// Debug: Log jump state when in air and space is pressed
		if (isJumping && Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			LOG("[UPDATE] Jump state: isJumping=%d, hasDoubleJump=%d, spaceWasReleased=%d",
				isJumping, hasDoubleJump, spaceWasReleased);
		}

		Move();
		Jump();
		DoubleJump();
		Dash();
	}

	Teleport();
	ApplyPhysics();
	Draw(dt);

	return true;
}

void Player::Teleport() {
	// Teleport the player to a specific position for testing purposes
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_T) == KEY_DOWN) {
		pbody->SetPosition(96, 96);
	}
}

void Player::GetPhysicsValues() {
	// Read current velocity
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	// Don't reset horizontal velocity during dash
	if (!isDashing) {
		velocity = { 0.0f, velocity.y }; // Reset horizontal velocity by default
	}
}

void Player::Move() {
	// Don't override velocity during dash
	if (isDashing) {
		return;
	}

	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -speed;
		anims.SetCurrent("move");
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = speed;
		anims.SetCurrent("move");
	}
}

// God Mode movement (fly in all directions)
void Player::MoveGodMode() {
	float godSpeed = speed * 2.0f; // Faster in god mode

	velocity = { 0.0f, 0.0f }; // Reset velocity

	// Move in all 4 directions
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

	// Keep idle animation if not moving
	if (velocity.x == 0.0f && velocity.y == 0.0f) {
		anims.SetCurrent("idle");
	}
}

void Player::Jump() {
	// Basic jump - only when on ground
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		anims.SetCurrent("jump");
		isJumping = true;
		hasDoubleJump = false;
		spaceWasReleased = false;
		LOG("First Jump! Force: %.2f", jumpForce);
	}

	// Enable double jump ONLY ONCE when space is released after first jump
	// The key check: !hasDoubleJump ensures this only happens once
	if (isJumping && !spaceWasReleased && !hasDoubleJump &&
		Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_IDLE) {
		spaceWasReleased = true;
		hasDoubleJump = true; // Now double jump is available (ONLY ONCE)
		LOG("Space released - Double jump now available!");
	}
}

// Double Jump
void Player::DoubleJump() {
	// Double jump - ONLY if:
	// 1. In the air (isJumping)
	// 2. Space was released (spaceWasReleased) 
	// 3. Double jump is STILL available (hasDoubleJump)
	// 4. Space is pressed again
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN &&
		isJumping && spaceWasReleased && hasDoubleJump) {

		// Get current velocity
		b2Vec2 currentVel = Engine::GetInstance().physics->GetLinearVelocity(pbody);
		LOG("Double Jump triggered! Current velocity before: (%.2f, %.2f)", currentVel.x, currentVel.y);

		// Apply double jump with similar force to first jump (not stronger)
		float doubleJumpVelocity = -jumpForce * 1.8f; // Slightly weaker than velocity method
		Engine::GetInstance().physics->SetLinearVelocity(pbody, currentVel.x, doubleJumpVelocity);

		// Apply a smaller impulse for smoother effect
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce * 0.3f, true);

		// Check velocity after
		b2Vec2 newVel = Engine::GetInstance().physics->GetLinearVelocity(pbody);
		LOG("Double Jump applied! New velocity: (%.2f, %.2f)", newVel.x, newVel.y);

		// Reset animation for visual feedback
		anims.SetCurrent("jump");

		// CRITICAL: CONSUME double jump permanently until ground touch
		hasDoubleJump = false;

		LOG("Double jump CONSUMED - hasDoubleJump is now FALSE!");
		LOG("No more jumps possible until touching ground!");
	}

	// Debug: log if trying to double jump when not available
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN &&
		isJumping && !hasDoubleJump) {
		LOG("Cannot double jump - already used! hasDoubleJump=%d", hasDoubleJump);
	}
}

// Dash
void Player::Dash() {
	// Check if can dash (cooldown finished and not already dashing)
	if (dashCooldownTimer <= 0.0f && !isDashing) {

		// Determine dash direction from input
		int desiredDashDir = 0;
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN ||
			Engine::GetInstance().input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_DOWN) {

			// Dash in the direction the player is moving
			if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
				desiredDashDir = -1;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				desiredDashDir = 1;
			}
			else {
				// If no direction pressed, use last movement direction
				// Default to right if never moved
				desiredDashDir = 1;
			}
		}

		// Start dash if direction is valid
		if (desiredDashDir != 0) {
			isDashing = true;
			dashTimer = static_cast<float>(dashDuration);
			dashCooldownTimer = static_cast<float>(dashCooldown);
			dashDirection = desiredDashDir;

			// Set high dash velocity
			velocity.x = dashForce * static_cast<float>(dashDirection);

			// Also apply an impulse for immediate effect
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(
				pbody,
				dashForce * 0.5f * static_cast<float>(dashDirection),
				0.0f,
				true
			);

			LOG("Dash! Direction: %d, Velocity: %.2f", dashDirection, velocity.x);
		}
	}
}

void Player::ApplyPhysics() {
	// In God Mode, disable collisions and gravity
	if (godMode) {
		// Disable gravity
		b2Body_SetGravityScale(pbody->body, 0.0f);

		// Change body type to kinematic (no collision response)
		b2Body_SetType(pbody->body, b2_kinematicBody);

		// Apply velocity directly
		Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
	}
	else {
		// Re-enable gravity
		b2Body_SetGravityScale(pbody->body, 1.0f);

		// Restore to dynamic body
		b2Body_SetType(pbody->body, b2_dynamicBody);

		// During dash, apply the full dash velocity
		if (isDashing) {
			// Maintain dash velocity, allow gravity
			velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
		// Preserve vertical speed while jumping
		else if (isJumping) {
			velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
		else {
			// Normal movement
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
		}
	}
}

void Player::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX(static_cast<float>(x));
	position.setY(static_cast<float>(y));

	//L10: TODO 7: Center the camera on the player
	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();
	float limitLeft = static_cast<float>(Engine::GetInstance().render->camera.w) / 4.0f;
	float limitRight = mapSize.getX() - static_cast<float>(Engine::GetInstance().render->camera.w) * 3.0f / 4.0f;
	if (position.getX() - limitLeft > 0.0f && position.getX() < limitRight) {
		Engine::GetInstance().render->camera.x = static_cast<int>(-position.getX() + static_cast<float>(Engine::GetInstance().render->camera.w) / 4.0f);
	}

	// L10: TODO 5: Draw the player using the texture and the current animation frame
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	// Ignore damage in God Mode
	if (godMode && physB->ctype != ColliderType::ITEM) {
		return; // Don't process collisions except items
	}

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("=== PLATFORM COLLISION ===");
		LOG("  Before reset: isJumping=%d, hasDoubleJump=%d, spaceWasReleased=%d",
			isJumping, hasDoubleJump, spaceWasReleased);

		// Reset ALL jump flags when touching the ground
		isJumping = false;
		hasDoubleJump = false;
		spaceWasReleased = false;
		anims.SetCurrent("idle");

		LOG("  After reset: isJumping=%d, hasDoubleJump=%d, spaceWasReleased=%d",
			isJumping, hasDoubleJump, spaceWasReleased);
		LOG("=== GROUNDED - Ready for new jump cycle ===");
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}
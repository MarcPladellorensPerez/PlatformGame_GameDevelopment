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

// NEW METHOD: Load parameters from XML
bool Player::LoadParameters(pugi::xml_node parameters) {

	// Load initial position
	position.setX(parameters.child("position").attribute("x").as_float());
	position.setY(parameters.child("position").attribute("y").as_float());

	// Load movement parameters
	speed = parameters.child("movement").child("speed").attribute("value").as_float();
	jumpForce = parameters.child("movement").child("jumpForce").attribute("value").as_float();

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
		Move();
		Jump();
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
	velocity = { 0, velocity.y }; // Reset horizontal velocity by default, this way the player stops when no key is pressed
}

void Player::Move() {

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

	velocity = { 0, 0 }; // Reset velocity

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
	if (velocity.x == 0 && velocity.y == 0) {
		anims.SetCurrent("idle");
	}
}

void Player::Jump() {
	// This function can be used for more complex jump logic if needed
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		anims.SetCurrent("jump");
		isJumping = true;
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

		// Preserve vertical speed while jumping
		if (isJumping == true) {
			velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
		}

		// Apply velocity via helper
		Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
	}
}

void Player::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	//L10: TODO 7: Center the camera on the player
	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();
	float limitLeft = Engine::GetInstance().render->camera.w / 4;
	float limitRight = mapSize.getX() - Engine::GetInstance().render->camera.w * 3 / 4;
	if (position.getX() - limitLeft > 0 && position.getX() < limitRight) {
		Engine::GetInstance().render->camera.x = -position.getX() + Engine::GetInstance().render->camera.w / 4;
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
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		anims.SetCurrent("idle");
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
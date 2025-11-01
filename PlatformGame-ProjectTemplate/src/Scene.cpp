#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Physics.h"

Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = std::dynamic_pointer_cast<Player>(Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER));

	pugi::xml_node playerConfigNode = configParameters.child("player");
	if (playerConfigNode) {
		std::string playerConfigPath = playerConfigNode.attribute("config_path").as_string();

		pugi::xml_document playerConfigFile;
		pugi::xml_parse_result result = playerConfigFile.load_file(playerConfigPath.c_str());

		if (result) {
			LOG("Player config file loaded: %s", playerConfigPath.c_str());
			player->LoadParameters(playerConfigFile.child("player_config"));
		}
		else {
			LOG("Error loading player config file: %s", result.description());
		}
	}

	//L08: TODO 4: Create a new item using the entity manager and set the position to (200, 672) to test
	std::shared_ptr<Item> item = std::dynamic_pointer_cast<Item>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM));
	item->position = Vector2D(200, 672);

	return ret;
}

// Called before the first frame
bool Scene::Start()
{

	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/background_music.wav");
	Engine::GetInstance().audio->SetMusicVolume(0.1f);
	Engine::GetInstance().audio->SetFxVolume(0.1f);
	LOG("Audio volumes configured: Music=0.4, FX=0.6");
	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/background_music.wav");


	LoadLevel(1);
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");
	
	Vector2D spawnPos = Engine::GetInstance().map->GetPlayerSpawnPosition();

	if (player) {
		player->position = spawnPos;
		player->spawnPosition = spawnPos;

		if (player->pbody) {
			player->pbody->SetPosition((int)spawnPos.getX(), (int)spawnPos.getY());
		}

		LOG("Player position set to spawn: (%.2f, %.2f)", spawnPos.getX(), spawnPos.getY());
	}

	return true;
}

bool Scene::PreUpdate()
{
	return true;
}

bool Scene::Update(float dt)
{
	float camSpeed = 100.0f;

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		Engine::GetInstance().render->camera.y -= (int)ceil(camSpeed * dt / 1000.0f);

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		Engine::GetInstance().render->camera.y += (int)ceil(camSpeed * dt / 1000.0f);

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		Engine::GetInstance().render->camera.x -= (int)ceil(camSpeed * dt / 1000.0f);

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		Engine::GetInstance().render->camera.x += (int)ceil(camSpeed * dt / 1000.0f);


	return true;
}

bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

void Scene::LoadLevel(int levelNumber) {
	// Descargar nivel anterior
	UnloadLevel();

	currentLevel = levelNumber;

	std::string levelFile = "Level" + std::to_string(levelNumber) + ".tmx";
	Engine::GetInstance().map->Load("Assets/Maps/", levelFile);

	Vector2D spawnPos = Engine::GetInstance().map->GetPlayerSpawnPosition();
	if (player) {
		player->position = spawnPos;
		player->spawnPosition = spawnPos;
		if (player->pbody) {
			player->pbody->SetPosition((int)spawnPos.getX(), (int)spawnPos.getY());
		}
	}

	LOG("Level %d loaded", levelNumber);
}

void Scene::UnloadLevel() {
	Engine::GetInstance().map->CleanUp();
	LOG("Level unloaded");
}
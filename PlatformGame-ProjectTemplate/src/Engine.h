#pragma once

#include <memory>
#include <list>
#include "Module.h"
#include "Timer.h"
#include "PerfTimer.h"
#include "pugixml.hpp"
#include <SDL3/SDL.h>

// Modules
class Window;
class Input;
class Render;
class Textures;
class Audio;
class Scene;
class EntityManager;
class Map;
//L08 TODO 2: Add Physics module
class Physics;

class Engine
{
public:

	// Public method to get the instance of the Singleton
	static Engine& GetInstance();

	//	
	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	float GetDt() const {
		return dt;
	}

	// Draw debug help menu
	void DrawDebugHelp();

	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

	// Modules
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Render> render;
	std::shared_ptr<Textures> textures;
	std::shared_ptr<Audio> audio;
	std::shared_ptr<Scene> scene;
	// L04: TODO 1: Add the EntityManager Module to the Engine
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<Map> map;
	// L08: TODO 2: Add Physics module
	std::shared_ptr<Physics> physics;

private:

	// Private constructor to prevent instantiation
	// Constructor
	Engine();

	// Delete copy constructor and assignment operator to prevent copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load config file
	bool LoadConfig();

	std::list<std::shared_ptr<Module>> moduleList;

	// Delta time
	float dt;

	// Calculate timing measures
	Timer startupTime;
	PerfTimer frameTime;
	PerfTimer lastSecFrameTime;

	uint64_t frameCount = 0;
	uint32_t framesPerSecond = 0;
	uint32_t lastSecFrameCount = 0;

	float averageFps = 0.0f;
	float secondsSinceStartup = 0.0f;

	// FPS control
	uint32_t targetFrameRate = 60;
	uint32_t cappedMs = 1000 / targetFrameRate;

	std::string gameTitle = "Platformer Game";

	// L05 TODO 2: Declare a xml_document to load the config file
	pugi::xml_document configFile;

	// Debug options
	bool showDebugHelp = false;
	bool debugColliders = false;
	bool godMode = false;

	// Textura para el help menu
	SDL_Texture* helpMenuTexture = nullptr;
};
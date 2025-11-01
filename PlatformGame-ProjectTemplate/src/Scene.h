#pragma once

#include "Module.h"
#include "Player.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Get player pointer
	std::shared_ptr<Player> GetPlayer() { return player; }

	void LoadLevel(int levelNumber);

	void UnloadLevel();
private:


	//L03: TODO 3b: Declare a Player attribute
	std::shared_ptr<Player> player;
	int currentLevel = 1;

};
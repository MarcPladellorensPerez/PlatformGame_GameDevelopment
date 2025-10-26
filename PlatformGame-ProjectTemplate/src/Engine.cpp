#include <iostream>
#include <sstream>
#include <iomanip>

#include "Engine.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Map.h"
#include "Physics.h"
#include "Log.h"

// Constructor
Engine::Engine() {

    LOG("Constructor Engine::Engine");

    // L2: TODO 3: Measure the amount of ms that takes to execute the Engine constructor and LOG the result
    Timer timer = Timer();
    startupTime = Timer();
    frameTime = PerfTimer();
    lastSecFrameTime = PerfTimer();
    frameCount = 0;

    // L4: TODO 1: Add the EntityManager Module to the Engine

    // Modules
    window = std::make_shared<Window>();
    input = std::make_shared<Input>();
    render = std::make_shared<Render>();
    textures = std::make_shared<Textures>();
    audio = std::make_shared<Audio>();
    // L08: TODO 2: Add Physics module
    physics = std::make_shared<Physics>();
    scene = std::make_shared<Scene>();
    map = std::make_shared<Map>();
    entityManager = std::make_shared<EntityManager>();

    // Ordered for awake / Start / Update
    // Reverse order of CleanUp
    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(input));
    AddModule(std::static_pointer_cast<Module>(textures));
    AddModule(std::static_pointer_cast<Module>(audio));
    // L08: TODO 2: Add Physics module
    AddModule(std::static_pointer_cast<Module>(physics));
    AddModule(std::static_pointer_cast<Module>(map));
    AddModule(std::static_pointer_cast<Module>(scene));
    AddModule(std::static_pointer_cast<Module>(entityManager));

    // Render last 
    AddModule(std::static_pointer_cast<Module>(render));

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App Constructor: %f", timer.ReadMSec());
}

// Static method to get the instance of the Engine class, following the singleton pattern
Engine& Engine::GetInstance() {
    static Engine instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

void Engine::AddModule(std::shared_ptr<Module> module) {
    module->Init();
    moduleList.push_back(module);
}

// Called before render is available
bool Engine::Awake() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Awake and LOG the result
    Timer timer = Timer();

    LOG("Engine::Awake");

    //L05 TODO 2: Add the LoadConfig() method here
    LoadConfig();
    // L05: TODO 3: Read the title from the config file and set the variable gameTitle, read targetFrameRate and set the variables
    gameTitle = configFile.child("config").child("engine").child("title").child_value();
    targetFrameRate = configFile.child("config").child("engine").child("targetFrameRate").attribute("value").as_int();

    //Iterates the module list and calls Awake on each module
    bool result = true;
    for (const auto& module : moduleList) {
        // L05: TODO 4: Call the LoadParameters function for each module
        module->LoadParameters(configFile.child("config").child(module.get()->name.c_str()));
        result = module->Awake();

        if (!result) {
            break;
        }
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App Awake(): %f", timer.ReadMSec());

    return result;
}

// Called before the first frame
bool Engine::Start() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Start() and LOG the result
    Timer timer = Timer();

    LOG("Engine::Start");

    //Iterates the module list and calls Start on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Start();
        if (!result) {
            break;
        }
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App CleanUp(): %f", timer.ReadMSec());

    return result;
}

// Called each loop iteration
bool Engine::Update() {

    bool ret = true;
    PrepareUpdate();

    // Check for quit
    if (input->GetWindowEvent(WE_QUIT) == true)
        ret = false;

    // Toggle debug help with H key
    if (input->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
        showDebugHelp = !showDebugHelp;

		// Debug help menu 
        if (showDebugHelp) {
            LOG("=== DEBUG HELP MENU ===");
            LOG("H     - Toggle this help");
            LOG("F9    - Show/Hide colliders [%s]", debugColliders ? "ON" : "OFF");
            LOG("F10   - God Mode [%s]", godMode ? "ON" : "OFF");
            LOG("F11   - Toggle FPS cap [%d]", targetFrameRate);
            LOG("ESC   - Exit game");
            LOG("=======================");
        }
        else {
            LOG("Debug Help: HIDDEN");
        }
    }

    // Toggle collider visualization with F9
    if (input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) {
        debugColliders = !debugColliders;
        physics->debug = debugColliders;
        LOG("Debug Colliders: %s", debugColliders ? "ON" : "OFF");
    }

    // Toggle God Mode with F10
    if (input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
        godMode = !godMode;

        // Apply god mode to player
        if (scene && scene->GetPlayer()) {
            scene->GetPlayer()->godMode = godMode;
        }

        LOG("God Mode: %s", godMode ? "ON" : "OFF");
    }

    if (ret == true)
        ret = PreUpdate();

    if (ret == true)
        ret = DoUpdate();

    if (ret == true)
        ret = PostUpdate();

    FinishUpdate();
    return ret;
}

// Called before quitting
bool Engine::CleanUp() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Start() and LOG the result
    Timer timer = Timer();

    LOG("Engine::CleanUp");

    //Iterates the module list and calls CleanUp on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->CleanUp();
        if (!result) {
            break;
        }
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App CleanUp(): %f", timer.ReadMSec());

    return result;
}

// ---------------------------------------------
void Engine::PrepareUpdate()
{
    frameTime.Start();
}

// ---------------------------------------------
void Engine::FinishUpdate()
{
    // FPS calculation
    double currentDt = frameTime.ReadMs();

    // Cap framerate if needed
    if (targetFrameRate > 0) {
        cappedMs = 1000 / targetFrameRate;
        if (currentDt < cappedMs) {
            uint32_t delay = (uint32_t)(cappedMs - currentDt);
            SDL_Delay(delay);
        }
    }

    // Update frame count
    frameCount++;
    lastSecFrameCount++;

    // Amount of time since game start (use a low resolution timer)
    secondsSinceStartup = startupTime.ReadSec();

    // Amount of ms took the last update (dt)
    dt = (float)frameTime.ReadMs();

    // Calculate average FPS
    if (lastSecFrameTime.ReadMs() > 1000) {
        lastSecFrameTime.Start();
        framesPerSecond = lastSecFrameCount;
        lastSecFrameCount = 0;

        // Calculate true average (cumulative)
        if (averageFps == 0.0f) {
            averageFps = (float)framesPerSecond;
        }
        else {
            averageFps = (averageFps + framesPerSecond) / 2.0f;
        }
    }

    // Get vsync status
    bool vsyncEnabled = false;
    if (render && render->renderer) {
        int vsync = 0;
        SDL_GetRenderVSync(render->renderer, &vsync);
        vsyncEnabled = (vsync == 1);
    }

    // Format window title with FPS info
    // Format: "FPS: XX / Avg.FPS: XX.XX / Last-frame MS: XX.XX / Vsync: on/off"
    std::stringstream ss;
    ss << gameTitle
        << " | FPS: " << framesPerSecond
        << " / Avg.FPS: " << std::fixed << std::setprecision(2) << averageFps
        << " / Last-frame MS: " << std::fixed << std::setprecision(2) << dt
        << " / Vsync: " << (vsyncEnabled ? "on" : "off");

    std::string titleStr = ss.str();
    window->SetTitle(titleStr.c_str());
}


// Call modules before each loop iteration
bool Engine::PreUpdate()
{
    //Iterates the module list and calls PreUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PreUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules on each loop iteration
bool Engine::DoUpdate()
{
    //Iterates the module list and calls Update on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Update(dt);
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules after each loop iteration
bool Engine::PostUpdate()
{
    //Iterates the module list and calls PostUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PostUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Draw debug help menu
void Engine::DrawDebugHelp()
{
    if (!showDebugHelp) return;

    // Background semi-transparent
    SDL_Rect helpRect = { 10, 10, 400, 200 };
    render->DrawRectangle(helpRect, 0, 0, 0, 200, true, false);

    // Border
    render->DrawRectangle(helpRect, 255, 255, 0, 255, false, false);
}

// Load config from XML file
bool Engine::LoadConfig()
{
    bool ret = true;

    // L05: TODO 2: Load config.xml file using load_file() method from the xml_document class
    // If the result is ok get the main node of the XML
    // else, log the error
    // check https://pugixml.org/docs/quickstart.html#loading

    pugi::xml_parse_result result = configFile.load_file("config.xml");
    if (result)
    {
        LOG("config.xml parsed without errors");
    }
    else
    {
        LOG("Error loading config.xml: %s", result.description());
    }

    return ret;
}
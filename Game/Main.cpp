#include "Engine.h"
#include "Core/File.h"
#include "Framework/Scene.h"

using namespace nu;

int main()
{
    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    Scene scene;
    scene.Load("scenes/scene-helmet.json");

    auto& renderer = Engine::Instance().GetRenderer();
    auto& editor = Engine::Instance().GetEditor();

    // MAIN LOOP
    bool quit = false;
    while (!quit) 
    {
        // UPDATE
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            editor.ProcessEvent(event);

            if (event.type == SDL_EVENT_QUIT) 
            {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
            {
                quit = true;
            }
        }

        // ENGINE
        Engine::Instance().Update();
        float dt = Engine::Instance().GetTime().GetDeltaTime();

        // SCENE
        scene.Update(dt);

        // EDITOR
        editor.BeginFrame();
        editor.Update(scene);

        // RENDER 
        renderer.BeginFrame();

        scene.Draw(renderer);

        renderer.EndRenderPass();
        editor.Draw(renderer);


        renderer.EndFrame();
    }

    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}


#include "Engine.h"
#include "Core/File.h"
#include "Math/Transform.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Model.h"
#include "Resources/ResourceManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

using namespace nu;

int main()
{
    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    auto model = Resources().Get<Model>("models/spot.obj", Engine::Instance().GetRenderer());
    auto texture = Resources().Get<Texture>("textures/spot_diffuse.png", Engine::Instance().GetRenderer());

    auto vshader = Resources().Get<nu::Shader>("shaders/lit.vert", Engine::Instance().GetRenderer(), 1);
    auto fshader = Resources().Get<nu::Shader>("shaders/lit.frag", Engine::Instance().GetRenderer(), 1, 1);

    auto pipeline = std::make_shared<Pipeline>();

    pipeline->AddVertexBuffer(sizeof(Model::Vertex));
    pipeline->AddVertexAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, position));
    pipeline->AddVertexAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Model::Vertex, texcoord));
    pipeline->AddVertexAttribute(2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, normal));
    pipeline->AddVertexAttribute(3, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, tangent));

    pipeline->Create(*vshader.get(), *fshader.get(),
        Engine::Instance().GetRenderer().GetGPUDevice(),
        Engine::Instance().GetRenderer().GetWindow());

	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

    struct vertexUniform
    {
        glm::mat4 mvp;
    };

    vertexUniform uniform;
    glm::vec3 cameraPosition{ 0, 0, 5 };

    Transform transform;

    // MAIN LOOP
    bool quit = false;
    while (!quit) 
    {
        // UPDATE
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
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
        float time = Engine::Instance().GetTime().GetTime();

		//color.b = sin(time * 10.0f) * 0.5f + 0.5f;

        glm::vec3 direction{0};
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_A)) direction.x -= 1.0f;
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_D)) direction.x += 1.0f;
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_Q)) direction.y -= 1.0f;
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_E)) direction.y += 1.0f;
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_W)) direction.z -= 1.0f;
		if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_S)) direction.z += 1.0f;

        cameraPosition += direction * 5.0f * dt;

        // TRANSFORM MATRIX
        transform.rotation.x = time * 90;

        // VIEW MATRIX
        auto view = glm::lookAtRH(cameraPosition, cameraPosition + glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });

        // PROJECION MATRIX
		float aspectRatio = Engine::Instance().GetRenderer().GetWidth() / (float)Engine::Instance().GetRenderer().GetHeight();
        auto projection = glm::perspective(glm::radians(70.0f), aspectRatio, 0.01f, 20.0f);

        // MVP
        uniform.mvp = projection * view * transform.GetMatrix();

        // RENDER
        Engine::Instance().GetRenderer().BeginFrame();

        Engine::Instance().GetRenderer().SetPipeline(*pipeline);
        Engine::Instance().GetRenderer().SetVertexUniform(0, &uniform, sizeof(uniform));
        Engine::Instance().GetRenderer().SetFragmentUniform(0, &color, sizeof(color));
        Engine::Instance().GetRenderer().SetFragmentTexture(0, *texture);

        model->Draw(Engine::Instance().GetRenderer());

        Engine::Instance().GetRenderer().EndFrame();
    }

    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}


#include "Engine.h"
#include "Core/File.h"
#include "Math/Transform.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Model.h"
#include "Resources/ResourceManager.h"
#include "Framework/Scene.h"

#include "Components/CameraComponent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#define MAX_LIGHTS 8

using namespace nu;

enum class LightType : uint32_t
{
    Directional,
    Point,
    Spot
};


int main()
{
    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    Scene scene;
    scene.Load("scenes/scene.json");

    auto model = Resources().Get<Model>("models/ogre.obj", Engine::Instance().GetRenderer());
    auto texture = Resources().Get<Texture>("textures/ogre_diffuse.bmp", Engine::Instance().GetRenderer());

    auto vshader = Resources().Get<nu::Shader>("shaders/phong.vert", Engine::Instance().GetRenderer(), 3);
    auto fshader = Resources().Get<nu::Shader>("shaders/phong.frag", Engine::Instance().GetRenderer(), 3, 1);

    auto pipeline = std::make_shared<Pipeline>();

    pipeline->AddVertexBuffer(sizeof(Model::Vertex));
    pipeline->AddVertexAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, position));
    pipeline->AddVertexAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Model::Vertex, texcoord));
    pipeline->AddVertexAttribute(2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, normal));
    pipeline->AddVertexAttribute(3, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, tangent));

    pipeline->Create(*vshader.get(), *fshader.get(),
        Engine::Instance().GetRenderer().GetGPUDevice(),
        Engine::Instance().GetRenderer().GetWindow());

    // UNIFORMS
    struct VertexUniform
    {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::vec3 cameraPosition;
        float padding1;
    };
    CHECK_UNIFORM_BOUNDARY(VertexUniform);

    VertexUniform vertexUniform;

    struct Light
    {
        glm::vec3 position;
        LightType type;
        glm::vec3 direction;
        float range;
        glm::vec3 color;
        float intensity;
        glm::vec3 ambient;
        float innerCutoff;
        float outerCutoff;
        glm::vec3 padding;
    };

    struct LightUniform
    {
        Light lights[MAX_LIGHTS];
        glm::vec3 ambient;
        uint32_t numLights;
    };

    CHECK_UNIFORM_BOUNDARY(LightUniform);
	LightUniform lightUniform;

    Light light1{
        .position {10, 10, 10},
        .type { LightType::Point },
        .direction{ 0, -1, 0},
        .range{ 400 },
        .color{1, 0, 0},
        .intensity{ 1 },
        .ambient{ 0.1f },
        .innerCutoff{ 0.3f},
        .outerCutoff{ 1.6f}
    };

    Light light2{
    .position {-10, 10, 10},
    .type { LightType::Point },
    .direction{ 0, -1, 0},
    .range{ 400 },
    .color{0, 0, 1},
    .intensity{ 1 },
    .ambient{ 0.1f },
    .innerCutoff{ 0.3f},
    .outerCutoff{ 1.6f}
    };

    Light light3{
    .position {-10, -10, 10},
    .type { LightType::Point },
    .direction{ 0, -1, 0},
    .range{ 400 },
    .color{0, 1, 0},
    .intensity{ 1 },
    .ambient{ 0.1f },
    .innerCutoff{ 0.3f},
    .outerCutoff{ 1.6f}
    };

    lightUniform.lights[0] = light1;
    lightUniform.lights[1] = light2;
	lightUniform.lights[2] = light3;
	lightUniform.numLights = 3;
	lightUniform.ambient = glm::vec3{ 0.2f };

    struct MaterialUniform
    {
        glm::vec4 color;
        glm::vec2 tiling;
        glm::vec2 offset;
        float shininess;
        glm::vec3 padding;
    };
    CHECK_UNIFORM_BOUNDARY(MaterialUniform);

    MaterialUniform  materialUniform
    {
        .color{1, 1, 1, 1},
        .tiling{ 1.0f, 1.0f},
        .offset{ 0.0f, 0.0f},
        .shininess{ 128.f }
    };


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

        // SCENE
        scene.Update(dt);       

        //lightUniform.lights[0].position = glm::eulerAngleY(time) * glm::vec4{ 0, 0, 10, 1 };
        //lightUniform.lights[1].position = glm::eulerAngleX(time * 0.5f) * glm::vec4{ 0, 0, 10, 1 };

        // CAMERA
		auto camera = scene.GetActorByName("Camera");
		auto cameraComponent = camera->GetComponent<CameraComponent>();
		vertexUniform.cameraPosition = camera->GetTransform().position;

        // TRANSFORM MATRIX
        //transform.rotation.x = time * 90;
        //transform.rotation.x = 180;
		transform.scale = glm::vec3{ 5, 5, 5 };

        // VIEW MATRIX
        //auto view = glm::lookAtRH(vertexUniform.cameraPosition, vertexUniform.cameraPosition + glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });

        // PROJECION MATRIX
		//float aspectRatio = Engine::Instance().GetRenderer().GetWidth() / (float)Engine::Instance().GetRenderer().GetHeight();
        //auto projection = glm::perspective(glm::radians(70.0f), aspectRatio, 0.01f, 300.0f);
     
        // MVP
        vertexUniform.model = transform.GetMatrix();
        vertexUniform.mvp = cameraComponent->GetProjection() * cameraComponent->GetView() * vertexUniform.model;

        // RENDER
        Engine::Instance().GetRenderer().BeginFrame();

        Engine::Instance().GetRenderer().SetPipeline(*pipeline);

        Engine::Instance().GetRenderer().SetVertexUniform(UniformSlot::Object, &vertexUniform, sizeof(vertexUniform));
        Engine::Instance().GetRenderer().SetVertexUniform(UniformSlot::Light, &lightUniform, sizeof(lightUniform));
        Engine::Instance().GetRenderer().SetVertexUniform(UniformSlot::Material, &materialUniform, sizeof(materialUniform));

        Engine::Instance().GetRenderer().SetFragmentUniform(UniformSlot::Object, &vertexUniform, sizeof(vertexUniform));
        Engine::Instance().GetRenderer().SetFragmentUniform(UniformSlot::Light, &lightUniform, sizeof(lightUniform));
        Engine::Instance().GetRenderer().SetFragmentUniform(UniformSlot::Material, &materialUniform, sizeof(materialUniform));
        Engine::Instance().GetRenderer().SetFragmentTexture(0, *texture);

        model->Draw(Engine::Instance().GetRenderer());

        Engine::Instance().GetRenderer().EndFrame();
    }

    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}


#include "CloudSystem.h"
#include "DrawingUtilities.h"
#include "GameWindow.h"
#include "Plane.h"
#include "raylib.h"
#include "VectorMath.h"

#include <cmath>
#include <cstdint>

#if defined( EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif //

constexpr auto initialScreenWidth = 1024;
constexpr auto initialScreenHeight = 768;

constexpr auto windowTitle = "Combatants";

namespace { // unnamed

    class Bullet
    {
    public:
        Bullet(Color color, Vector2 position, Vector2 speed)
            : color(color), position(position), speed(speed) {}

        bool Update( const GameWindow &window, float deltaTime)
        {
            position += speed * deltaTime;
            position = {
                Wrap(position.x, static_cast<float>(window.width)),
                Wrap(position.y, static_cast<float>(window.height))};
            return (lifeTime -= deltaTime) > 0;
        }

        void Draw()
        {
            DrawCircleV(position, 4, color);
        }

    private:
        Color color = RED;
        Vector2 position;
        Vector2 speed;

        float lifeTime = 2.0f;
    };

    void Draw( std::vector<Bullet>& bullets)
    {
        for (auto& bullet : bullets)
        {
            bullet.Draw();
        }
    }

    void Update( GameWindow &window, std::vector<Bullet>& bullets, float deltaTime)
    {
        for (auto it = bullets.begin(); it != bullets.end();)
        {
            if (not it->Update( window, deltaTime))
            {
                it = bullets.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

void ControlPlane(Plane& plane)
{
    bool keyPressed = false;
    if (IsKeyDown(KEY_RIGHT))
    {
        plane.DeltaPitch(2);
        keyPressed = true;
    }
    else if (IsKeyDown(KEY_LEFT))
    {
        plane.DeltaPitch(-2);
        keyPressed = true;
    }

    const auto roll = plane.GetRoll();
    if (not keyPressed or (roll != 0 and roll != 128))
    {
        static constexpr std::int8_t rollCorrection = 4;
        if (const auto pitch = plane.GetPitch(); pitch >= 64 and pitch < 192)
        {
            if (const auto roll = plane.GetRoll(); roll != 128)
            {
                plane.DeltaRoll(roll > 128 ? -rollCorrection : rollCorrection);
            }
        }
        else
        {
            if (roll != 0)
            {
                plane.DeltaRoll(roll >= 128 ? +rollCorrection : -rollCorrection);
            }
        }

    }
}


void UpdateSound(Music& engine, const Plane& plane)
{
    SetMusicPan(engine, 0.75f - (plane.GetPosition().x / (float)initialScreenWidth)/2.0f);
    SetMusicPitch(engine, 1.0f - (plane.GetSpeedVector().y / plane.GetSpeed()) * 0.5f);
    UpdateMusicStream(engine);
}

struct Sounds
{
    Music engine = LoadMusicStream(ASSETS_PATH "engine_sound.wav");
    Sound gun  = LoadSound(ASSETS_PATH "gun_sound.wav");
    float gunVolume = 0.5f;
    float engineVolume = 0.5f;

    ~Sounds()
    {
        UnloadMusicStream( engine);
        UnloadSound(gun);
    }

    void EnableSound(bool enableEngine, bool enableGun)
    {
        SetSoundVolume(gun, enableGun ? gunVolume : 0.0f);
        SetMusicVolume(engine, enableEngine ? engineVolume : 0.0f);
    }
};

struct GameAudio
{
    GameAudio()
    {
        InitAudioDevice();
    }

    ~GameAudio()
    {
        CloseAudioDevice();
    }
};

struct Game : public GameAudio, public GameWindow
{
public:
    static Game &GetInstance()
    {
        static Game instance;
        return instance;
    }

    Game(const Game&)               = delete;
    Game& operator=(const Game&)    = delete;
    Game(Game&&)                    = delete;
    Game& operator=(Game&&)         = delete;
    Plane &GetPlane() { return planes[0]; }

    void Update()
    {
        const float deltaTime = GetFrameTime();
        GameWindow::Update();
        ControlPlane(planes[0]);

        for (auto& plane : planes)
        {
            plane.Update( *this, deltaTime);
        }        clouds.Update( *this, deltaTime);

        if (IsKeyPressed(KEY_SPACE))
        {
            SetSoundPan( sounds.gun, 1.0f - (planes[0].GetPosition().x / (float)initialScreenWidth)/2.0f);
            PlaySound(sounds.gun);
            bullets.emplace_back( DARKGREEN, planes[0].GetPosition(), planes[0].GetSpeedVector() * 2.0f);
        }

        ::Update( *this, bullets, deltaTime);

        UpdateSound(sounds.engine, planes[0]);

        BeginDrawing();
        ClearBackground(SKYBLUE);
        Draw(bullets);

        for (const auto &plane : planes)
        {
            plane.Draw( *this);
        }

        DrawCloudSystem(clouds, *this);

        EndDrawing();
    }

    void EnableSound(bool enableEngine, bool enableGun)
    {
        sounds.EnableSound(enableEngine, enableGun);
    }

private:
    Game()
    :
    GameWindow( initialScreenWidth, initialScreenHeight, "Combatants"),
    planes{{
        {"green", { initialScreenWidth / 2.0f + 20, initialScreenHeight / 2.0f}, 220, 128},
        {"red", { initialScreenWidth / 2.0f - 20, initialScreenHeight / 2.0f}, 220, 0}}}
    {
        PlayMusicStream( sounds.engine);
        bullets.reserve(20);
    }


    Sounds                  sounds;
    std::vector<Bullet>     bullets;
    std::array<Plane, 2>    planes;
    CloudSystem             clouds = CreateRandomCloudSystem(
        6, 15, 40.0f/1024, 0.9f);
};

void UpdateDrawFrame()
{
    static auto &game = Game::GetInstance();
    game.Update();
}
}

#if defined( EMSCRIPTEN)

extern "C" {
    void EnableSound( bool enableEngine, bool enableGun)
    {
        auto &game = Game::GetInstance();
        game.EnableSound( enableEngine, enableGun);
    }
}

#endif // EMSCRIPTEN

int main(void)
{
#if defined( EMSCRIPTEN)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    auto &game = Game::GetInstance();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        game.Update();
    }

#endif // EMSCRIPTEN
    return 0;
}

#include "CloudSystem.h"
#include "GameWindow.h"
#include "Plane.h"
#include "raylib.h"
#include "VectorMath.h"
#include "Bullet.h"

#include <cmath>
#include <cstdint>
#include <tuple>
#include <vector>

#if defined( EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif //

constexpr auto initialScreenWidth = 1024;
constexpr auto initialScreenHeight = 768;

constexpr auto windowTitle = "Combatants";

namespace { // unnamed

    /**
     * Is a point inside a rectangle?
     *
     */
    inline bool IsIn( const Vector2& point, const Rectangle& box)
    {
        return (point.x >= box.x and point.x <= box.x + box.width
                and point.y >= box.y and point.y <= box.y + box.height);
    }

    using Collisions = std::vector< std::tuple<unsigned int, unsigned int>>;

    /**
     * Find all collisions between points and collidables and return all found
     * collisions.
     *
     */
    template< typename PointObjects, typename Collidables>
    Collisions FindCollisions( PointObjects &points, Collidables &collidables)
    {
        Collisions collisions;
        for (size_t i = 0; i < points.size(); ++i)
        {
            for (size_t j = 0; j < collidables.size(); ++j)
            {
                if (
                    IsIn(GetPosition(points[i]), collidables[j].GetBoundingBox())
                    and collidables[j].Collides( GetPosition(points[i])))
                {
                    collisions.emplace_back( i, j);
                }
            }
        }
        return collisions;
    }

/**
 * Control the plane.
 *
 * Left and right arrow keys control the pitch. When the plane is
 * upside down, it will roll until it is upright again.
 *
 */
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

/**
 * This class mainly exists to own the audio assets used in the game.
 */
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

/**
 * This class is used to initialize and close the audio device.
 */
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

/**
 * The Game object is a singleton that holds all the game objects, such as
 * planes, bullets and clouds. It also initializes relevant parts of raylib.
 *
 * This object also implements the game frame update and draw functions.
 */
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

        // First, do updates.
        GameWindow::Update();

        // make the plane react to keyboard input
        ControlPlane(planes[0]);

        // do physics updates
        for (auto& plane : planes)
        {
            plane.Update( *this, deltaTime);
        }

        // let clouds do their thing
        clouds.Update( *this, deltaTime);

        // create bullets when the space key is pressed
        if (IsKeyPressed(KEY_SPACE))
        {
            SetSoundPan( sounds.gun, 1.0f - (planes[0].GetPosition().x / (float)initialScreenWidth)/2.0f);
            PlaySound(sounds.gun);
            bullets.emplace_back( DARKGREEN, 0, planes[0].GetPosition(), planes[0].GetSpeedVector() * 2.0f);
        }

        // update bullet trajectories
        ::Update( bullets, *this, deltaTime);
        DoCollisions();

        // adapt the sounds to  what is happening.
        UpdateSound(sounds.engine, planes[0]);
    }

    void Draw()
    {
        using ::Draw;

        // All physics and interactions are done, now draw the frame.
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // 1. bullets
        Draw( bullets);

        // 2. planes
        for (const auto &plane : planes)
        {
            plane.Draw( *this);
        }

        // 3. clouds
        DrawCloudSystem(clouds, *this);

        EndDrawing();
    }

    void UpdateAndDraw()
    {
        Update();
        Draw();
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
    }

    void DoCollisions()
    {
        const auto collisions = FindCollisions( bullets, planes);

        for (const auto& collision : collisions)
        {
            const auto bulletIndex = std::get<0>(collision);
            const auto planeIndex  = std::get<1>(collision);
            if (bullets[bulletIndex].GetOwner() == planeIndex)
            {
                continue;
            }
            bullets.erase( bullets.begin() + bulletIndex); // TODO: deal with multiple bullets and index shift
            planes[planeIndex].DeltaRoll( 32);
        }
    }

    Sounds                  sounds;
    std::array<Plane, 2>    planes;
    Bullets                 bullets;
    CloudSystem             clouds = CreateRandomCloudSystem(
        6, 15, 40.0f/1024, 0.9f);
};

void UpdateDrawFrame()
{
    static auto &game = Game::GetInstance();
    game.UpdateAndDraw();
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
    auto &game = Game::GetInstance();
    game.EnableSound( false, true);

#if defined( EMSCRIPTEN)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        game.UpdateAndDraw();
    }

#endif // EMSCRIPTEN
    return 0;
}

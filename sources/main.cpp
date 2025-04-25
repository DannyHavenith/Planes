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

    // Uniform handling of Draw(). This defines concepts for drawable objects,
    // rather than defining a Draw() interface.

    template<typename T>
    concept SelfDrawable = requires(T d, const GameWindow &window) {
        d.Draw(window);
    };

    template<SelfDrawable D>
    void Draw( const D &drawable, const GameWindow &window)
    {
        drawable.Draw( window);
    }

    template< typename T>
    concept Drawable = requires(const T d, const GameWindow &window) {
        Draw( d, window);
    };

    template<typename T>
    concept DrawableRange = std::ranges::range<T> and Drawable<typename T::value_type>;

    template<DrawableRange Container>
    void Draw( const Container &drawables, const GameWindow &window)
    {
        for (const auto &drawable : drawables)
        {
            Draw( drawable, window);
        }
    }

    template< typename T>
    concept SelfUpdateable = requires(T u, const GameWindow &window, float deltaTime) {
        u.Update(window, deltaTime);
    };

    template<SelfUpdateable U>
    void Update( U &updateable, const GameWindow &window, float deltaTime)
    {
        updateable.Update(window, deltaTime);
    }

    template< typename T>
    concept Updateable = requires(T u, const GameWindow &window, float deltaTime) {
        Update( u, window, deltaTime);
    };
    template< typename T>
    concept UpdateableRange = std::ranges::range<T> and Updateable<typename T::value_type>;

    template<UpdateableRange Container>
    void Update( Container &updateables, const GameWindow &window, float deltaTime)
    {
        for (auto &updateable : updateables)
        {
            Update( updateable, window, deltaTime);
        }
    }

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
    ~Game()                         = default;

    Plane &GetPlane() { return planes[0]; }


    void Update()
    {
        using ::Update;
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
            controls[i]( i, planes[i], *this, bullets, sounds);
        }

        Update( planes, *this, deltaTime);

        Update( bullets, *this, deltaTime);
        Update( clouds, *this, deltaTime);
        DoCollisions();

        // adapt the sounds to what is happening.
        UpdateSound(sounds.engine, planes[0]);
    }

    void Draw()
    {
        using ::Draw;

        // All physics and interactions are done, now draw the frame.
        BeginDrawing();
        ClearBackground(SKYBLUE);

        Draw( bullets, *this);
        Draw( planes, *this);
        Draw( clouds, *this);

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

    void DrawDebugIndicators( bool draw)
    {
        DrawPlaneDebugIndicators( draw);
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

#include "CloudSystem.h"
#include "GameWindow.h"
#include "Plane.h"
#include "raylib.h"
#include "VectorMath.h"
#include "Bullet.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <set>
#include <sstream>
#include <tuple>

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

    void Draw( const SelfDrawable auto &drawable, const GameWindow &window)
    {
        drawable.Draw( window);
    }

    template< typename T>
    concept Drawable = requires(const T d, const GameWindow &window) {
        Draw( d, window);
    };

    template<typename T>
    concept DrawableRange = std::ranges::range<T> and Drawable<typename T::value_type>;

    void Draw( const DrawableRange auto &drawables, const GameWindow &window)
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

    void Update( SelfUpdateable auto &updateable, const GameWindow &window, float deltaTime)
    {
        updateable.Update(window, deltaTime);
    }

    template< typename T>
    concept Updateable = requires(T u, const GameWindow &window, float deltaTime) {
        Update( u, window, deltaTime);
    };
    template< typename T>
    concept UpdateableRange = std::ranges::range<T> and Updateable<typename T::value_type>;

    void Update( UpdateableRange auto &updateables, const GameWindow &window, float deltaTime)
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

    using Collisions = std::set< std::tuple<unsigned int, unsigned int>>;

    /**
     * Find all collisions between points and collidables and return all found
     * collisions.
     *
     * Each point object will appear at most once in the result and because
     * Collisions is a set, the collisions are ordered by the point object
     * index.
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
                    collisions.emplace( i, j);
                    continue; // Stop looking for more collisions for this point.
                }
            }
        }
        return collisions;
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
 * Control the plane with keyboard keys.
 *
 * When the plane is upside down, it will roll until it is upright again.
 *
 */
 struct KeyboardPlaneControl
 {
    KeyboardKey keyLeft = KEY_LEFT;
    KeyboardKey keyRight = KEY_RIGHT;
    KeyboardKey keyTrigger = KEY_SPACE;

    void RollToUpright(Plane& plane)
    {
        const auto roll = plane.GetRoll();
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

    void operator()(
        std::size_t planeIndex,
        float deltaTime,
        Plane& plane,
        const GameWindow& window,
        Bullets &bullets,
        Sounds &sounds)
    {
        bool keyPressed = false;
        if (IsKeyDown(keyRight))
        {
            plane.DeltaPitch( 120.0 * deltaTime + 0.5f);
            keyPressed = true;
        }
        else if (IsKeyDown(keyLeft))
        {
            plane.DeltaPitch( -120.0 * deltaTime - 0.5f);
            keyPressed = true;
        }

        const auto roll = plane.GetRoll();
        if ((not keyPressed or (roll != 0 and roll != 128))and not (plane.GetState() == Plane::Crashing))
        {
            RollToUpright(plane);
        }

        // create bullets when the trigger key is pressed
        if (IsKeyPressed(keyTrigger) and plane.GetState() == Plane::Flying)

        {
            if (plane.Fire( bullets))
            {
                SetSoundPan( sounds.gun, 1.0f - (plane.GetPosition().x / (float)initialScreenWidth)/2.0f);
                PlaySound(sounds.gun);
            }
        }
    }
};

/**
 * A 'control' strategy that does not allow the user to control the plane, but rather lets the plane
 *
 */
using PlaneControl =
    std::function<
        void(
            std::size_t planeIndex,
            float deltaTime,
            Plane&,
            const GameWindow&,
            Bullets&,
            Sounds&)>;

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


    /**
    * Update the world state.
    */
    void Update()
    {
        using ::Update;
        const float deltaTime = GetFrameTime();

        // First, do updates.
        // figure out screen size
        GameWindow::Update();
        HandleGameMechanics();

        // let players control their planes
        assert(players.size() == planes.size());
        for (std::size_t i = 0; i < players.size(); ++i)
        {
            players[i].control( i, deltaTime, planes[i], *this, bullets, sounds);
        }

        // Do physics.
        Update( planes, *this, deltaTime);
        Update( bullets, *this, deltaTime);
        Update( clouds, *this, deltaTime);

        // Do physics that go bang.
        DoCollisions();



        // adapt the sounds to what is happening.
        UpdateSound(sounds.engine, planes[0]);
    }

    void HandleGameMechanics()
    {
        // reset planes that are in crashed state
        for (auto& plane : planes)
        {
            if (plane.GetState() == Plane::Crashed)
            {
                plane.Reset({ width / 2.0f, height / 2.0f }, 220, 0);
            }
        }
    }

    std::string FormatScore(int score)
    {
        return (std::ostringstream() << std::setw(2) << std::setfill('0') << score). str();
    }

    void DrawScore()
    {


        const int fontSize = 50;
        const int offset = 20;
        const int dropShadowOffset = 3;

        // Format scores with leading zeros using std::format
        const std::string score0 = FormatScore( players[0].score);
        const std::string score1 = FormatScore( players[1].score);

        int textWidth = MeasureText(score1.c_str(), fontSize);


        // Draw drop shadow for player 0's score
        DrawText(score0.c_str(), offset + dropShadowOffset, offset + dropShadowOffset, fontSize, Fade( GRAY, 0.5f));
        // Draw player 0's score in the top left corner
        DrawText(score0.c_str(), offset, offset, fontSize, planes[0].GetColor());

        // Draw drop shadow for player 1's score
        DrawText(score1.c_str(), width - textWidth - offset + dropShadowOffset, offset + dropShadowOffset, fontSize, GRAY);
        // Draw player 1's score in the top right corner
        DrawText(score1.c_str(), width - textWidth - offset, offset, fontSize, planes[1].GetColor());

        // Draw the bullet count for plane 0 directly below the score
        planes[0].DrawBulletCount(*this, { offset, offset + fontSize + 10.0f });

        // Draw the bullet count for plane 1 directly below the score
        planes[1].DrawBulletCount(*this, { static_cast<float>(width - textWidth - offset), offset + fontSize + 10.0f });


    }

    /**
    * Draw the world.
    */
    void Draw()
    {
        using ::Draw;

        // All physics and interactions are done, now draw the frame.
        BeginDrawing();
        ClearBackground(SKYBLUE);

        Draw( bullets, *this);
        Draw( planes, *this);
        Draw( clouds, *this);
        DrawScore();

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
        {0, "green", DARKGREEN, { initialScreenWidth / 2.0f + 20, initialScreenHeight / 2.0f}, 220, 128},
        {1, "red", RED, { initialScreenWidth / 2.0f - 20, initialScreenHeight / 2.0f}, 220, 0}}}
    {
        PlayMusicStream( sounds.engine);
    }

    void DoCollisions()
    {
        const auto collisions = FindCollisions( bullets, planes);

        // we assume that:
        // 1. collisions are ordered by the index of the bullet
        // 2. each bullet occurs only once in the list of collisions
        // These assumptions must be guaranteed by the FindCollisions function.
        int bulletIndexOffset = 0;
        for (const auto& collision : collisions)
        {
            const auto bulletIndex = std::get<0>(collision);
            const auto planeIndex  = std::get<1>(collision);
            if (
                bullets[bulletIndex].GetOwner() != planeIndex
                and planes[planeIndex].GetState() == Plane::Flying)
            {
                // A bullet of one player has hit a plane of the other player.
                bullets.erase( bullets.begin() + bulletIndex + bulletIndexOffset);
                planes[planeIndex].SetState(Plane::Crashing);
                --bulletIndexOffset;

                players[ bullets[bulletIndex].GetOwner()].score += 1;
            }
        }
    }

    struct Player
    {
        PlaneControl control;
        int score = 0;
    };

    Sounds                  sounds;
    std::array< Player, 2>  players = {
        Player{KeyboardPlaneControl( KEY_LEFT, KEY_RIGHT, KEY_SPACE)},
        Player{KeyboardPlaneControl( KEY_A, KEY_D, KEY_LEFT_SHIFT)}
    };
    std::array<Plane, 2>    planes;
    Bullets                 bullets;
    CloudSystem             clouds = CreateRandomCloudSystem(
        4, 24, 50.0f/1024, 0.9f);
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

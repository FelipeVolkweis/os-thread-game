#ifndef GAME_HH
#define GAME_HH

#include <SFML/Graphics.hpp>
#include <random>

#include <threadmill.h>
#include <player.h>

class Game {
public:
    Game();

    ~Game();

    void run();

private:
    void loadAssets();

    void processEvents();
    void handlePlayerAction(sf::Keyboard::Key key);

    void collectPackage();

    void update(float deltaTime);

    void spawnRandomPackage();

    void render();
    void updateScoreText();

    void updateLivesText();

    void updatePackageSpeed();
    void updatePackageSpawnInterval();

    void resetGame();

    Threadmill* getThreadmillByLane(int lane);

    void updateActiveThreadmills();

    sf::RenderWindow window;
    sf::Font font;
    sf::Text textScore;
    sf::Text textLives;

    int score;
    int lives;

    std::vector<int> laneYs = { THREADMILL_Y_POS_TOP, THREADMILL_Y_POS_CENTER, THREADMILL_Y_POS_BOTTOM };
    Threadmill threadmillTop;
    Threadmill threadmillCenter;
    Threadmill threadmillBottom;
    Player player;

    std::mt19937 rng;
    std::uniform_int_distribution<int> distLane;

    sf::Clock spawnTimer;

    float currentSpawnInterval;
    int spawnIntervalSteps;

    int nextId;
};

#endif // GAME_HH
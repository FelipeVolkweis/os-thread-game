#ifndef GAME_HH
#define GAME_HH

#include <SFML/Graphics.hpp>
#include <random>

#include <threadmill.h>
#include <player.h>

/**
 * @class Game
 * @brief Classe responsável por gerenciar o ciclo de vida do jogo.
 *
 * A classe Game encapsula toda a lógica principal do jogo, incluindo a inicialização,
 * execução, atualização e renderização. Ela também gerencia os recursos do jogo,
 * como fontes e textos, e mantém o controle do estado do jogo, como pontuação e vidas.
 *
 * @details
 * A classe Game contém métodos privados para carregar recursos, processar eventos,
 * atualizar o estado do jogo e renderizar a tela. Ela também gerencia a criação e
 * atualização de objetos do jogo, como esteiras e o jogador.
 *
 * @note
 * Esta classe utiliza a biblioteca SFML para renderização e manipulação de eventos.
 *
 * @see Threadmill
 * @see Player
 */
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
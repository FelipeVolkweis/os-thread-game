#include <iostream>

#include <game.h>

/**
 * @brief Construtor da classe Game.
 * 
 * Inicializa a janela do jogo, as esteiras, o jogador, o gerador de números aleatórios,
 * o temporizador de spawn e outras variáveis necessárias para o funcionamento do jogo.
 * 
 * - Configura a janela com limite de taxa de quadros.
 * - Inicializa a semente do gerador de números aleatórios.
 * - Carrega os recursos necessários.
 * - Inicializa a pontuação e vidas do jogador.
 * - Configura os textos de pontuação e vidas.
 * - Adiciona um pacote inicial à esteira central.
 * - Atualiza as esteiras ativas.
 */
Game::Game()
    : window(sf::VideoMode(WIDTH, HEIGHT), "Threadmill: The Game"),
      threadmillTop(THREADMILL_Y_POS_TOP, PACKAGE_SPEED_BASE),
      threadmillCenter(THREADMILL_Y_POS_CENTER, PACKAGE_SPEED_BASE),
      threadmillBottom(THREADMILL_Y_POS_BOTTOM, PACKAGE_SPEED_BASE), player(laneYs),
      rng(std::random_device{}()), distLane(0, 2), spawnTimer(),
      currentSpawnInterval(PACKAGE_SPAWN_INTERVAL_BASE), spawnIntervalSteps(0), nextId(1) {
    window.setFramerateLimit(60);
    srand(time(NULL));
    loadAssets();

    score = SCORE_INITIAL;
    lives = MAX_LIVES;

    textScore.setFont(font);
    textScore.setCharacterSize(SCORE_TEXT_SIZE);
    textScore.setFillColor(sf::Color::White);
    textScore.setPosition(SCORE_TEXT_POS_X, SCORE_TEXT_POS_Y);
    updateScoreText();

    textLives.setFont(font);
    textLives.setCharacterSize(SCORE_TEXT_SIZE);
    textLives.setFillColor(sf::Color::White);
    textLives.setPosition(LIVES_TEXT_POS_X, LIVES_TEXT_POS_Y);
    updateLivesText();

    threadmillCenter.addPackage(nextId++);

    updateActiveThreadmills();
}

Game::~Game() {}

/**
 * @brief Executa o loop principal do jogo.
 * 
 * Esta função inicia o relógio e entra em um loop que continua enquanto a janela estiver aberta.
 * Dentro do loop, calcula o tempo decorrido desde o último quadro, processa eventos, atualiza o estado do jogo
 * e renderiza o conteúdo na janela.
 */
void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();
        update(deltaTime);
        render();
    }
}

/**
 * @brief Carrega os recursos necessários para o jogo.
 * 
 * Esta função carrega a fonte e a textura do pacote. Se ocorrer um erro durante
 * o carregamento de qualquer um dos recursos, uma mensagem de erro será exibida
 * no console.
 * 
 * @note Certifique-se de que os caminhos dos arquivos de fonte e textura estão
 * corretos e que os arquivos existem no local especificado.
 */
void Game::loadAssets() {
    if (!font.loadFromFile(FONT_PATH)) {
        std::cout << "Error loading font." << std::endl;
    }

    if (!Package::loadTexture()) {
        std::cout << "Error loading package texture." << std::endl;
    }
}

/**
 * @brief Processa os eventos da janela do jogo.
 *
 * Esta função verifica e processa todos os eventos que ocorrem na janela do jogo.
 * Se a janela for fechada, ela será encerrada. Se uma tecla for pressionada, 
 * a ação correspondente do jogador será tratada.
 */
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::KeyPressed)
            handlePlayerAction(event.key.code);
    }
}

/**
 * @brief Manipula a ação do jogador com base na tecla pressionada.
 *
 * Esta função é chamada para lidar com as ações do jogador quando uma tecla é pressionada.
 * Dependendo da tecla pressionada, o jogador pode coletar um pacote ou mudar de faixa.
 *
 * @param key A tecla pressionada pelo jogador.
 *
 * - Se a tecla for `sf::Keyboard::Space`, o jogador coleta um pacote.
 * - Se a tecla for `sf::Keyboard::W` ou `sf::Keyboard::Up`, o jogador muda para a faixa acima.
 * - Se a tecla for `sf::Keyboard::S` ou `sf::Keyboard::Down`, o jogador muda para a faixa abaixo.
 */
void Game::handlePlayerAction(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Space) {
        collectPackage();
    }
    if (key == sf::Keyboard::W || key == sf::Keyboard::Up) {
        player.switchLane(-1);
        updateActiveThreadmills();
    }
    if (key == sf::Keyboard::S || key == sf::Keyboard::Down) {
        player.switchLane(1);
        updateActiveThreadmills();
    }
}

/**
 * @brief Coleta pacotes do threadmill atual.
 *
 * Esta função coleta pacotes do threadmill na mesma faixa que o jogador está atualmente.
 * Se o threadmill atual contiver pacotes válidos que o jogador pode pegar, o pacote é coletado,
 * a pontuação é incrementada, a velocidade dos pacotes e o intervalo de spawn são atualizados.
 * Em seguida, o pacote coletado é removido do threadmill e o texto da pontuação é atualizado.
 */
void Game::collectPackage() {
    int currentLane = player.getCurrentLane();
    Threadmill *currentThreadmill = getThreadmillByLane(currentLane);
    if (currentThreadmill) {
        std::map<int, Package> packages = currentThreadmill->getPackages();
        std::vector<int> collectedPackages;
        for (auto &[id, package] : packages) {
            if (package.isValid() && player.canGrabPackage(package)) {
                collectedPackages.push_back(id);
                score++;
                updatePackageSpeed();
                updatePackageSpawnInterval();
                break;
            }
        }
        for (int id : collectedPackages) {
            currentThreadmill->removePackage(id);
        }
        updateScoreText();
    }
}

/**
 * @brief Atualiza o estado do jogo.
 *
 * Esta função é chamada a cada frame para atualizar o estado do jogo com base no tempo decorrido.
 * Ela lida com a perda de pacotes, atualiza o número de vidas, reinicia o jogo se necessário,
 * processa a entrada do jogador e gera novos pacotes em intervalos regulares.
 *
 * @param deltaTime O tempo decorrido desde a última atualização, em segundos.
 */
void Game::update(float deltaTime) {
    int totalLostPackages = 0;
    totalLostPackages += threadmillTop.getAndResetLostPackages();
    totalLostPackages += threadmillCenter.getAndResetLostPackages();
    totalLostPackages += threadmillBottom.getAndResetLostPackages();

    if (totalLostPackages > 0) {
        lives -= totalLostPackages;
        if (lives < 0)
            lives = 0;
        updateLivesText();

        if (lives <= 0) {
            resetGame();
        }
    }

    player.handleInput(deltaTime);

    if (spawnTimer.getElapsedTime().asSeconds() >= currentSpawnInterval) {
        spawnRandomPackage();
        spawnTimer.restart();
    }
}

/**
 * @brief Gera um pacote aleatório e o adiciona a uma das esteiras.
 *
 * Esta função gera um número aleatório e, com base no valor gerado,
 * adiciona um novo pacote a uma das três esteiras: superior, central ou inferior.
 * O pacote recebe um identificador único que é incrementado a cada novo pacote.
 */
void Game::spawnRandomPackage() {
    int x = rand();
    if (x % 3 == 0) {
        threadmillTop.addPackage(nextId++);
    } else if (x % 3 == 1) {
        threadmillCenter.addPackage(nextId++);
    } else {
        threadmillBottom.addPackage(nextId++);
    }
}

/**
 * @brief Renderiza o estado atual do jogo na janela.
 * 
 * Esta função limpa a janela com uma cor de fundo específica e desenha
 * os elementos do jogo, incluindo as partes do threadmill, o jogador,
 * a pontuação e as vidas restantes. Após desenhar todos os elementos,
 * a função exibe o conteúdo na janela.
 */
void Game::render() {
    sf::Color backgroundColor(36, 36, 52); // #507bba
    window.clear(backgroundColor);

    threadmillTop.draw(window, font);
    threadmillCenter.draw(window, font);
    threadmillBottom.draw(window, font);

    player.draw(window);

    window.draw(textScore);
    window.draw(textLives);

    window.display();
}

/**
 * @brief Atualiza o texto da pontuação com o valor atual da pontuação.
 *
 * Esta função define a string do objeto textScore para exibir a pontuação
 * atual do jogo. A pontuação é convertida para uma string e concatenada
 * com o prefixo "Score: ".
 */
void Game::updateScoreText() {
    textScore.setString("Score: " + std::to_string(score));
}

/**
 * @brief Atualiza o texto que exibe o número de vidas restantes.
 *
 * Esta função atualiza o texto do objeto `textLives` para exibir o número
 * atual de vidas restantes no jogo. O texto é formatado como "Vidas: X",
 * onde X é o número de vidas.
 */
void Game::updateLivesText() {
    textLives.setString("Vidas: " + std::to_string(lives));
}

/**
 * @brief Atualiza a velocidade dos pacotes na esteira.
 *
 * A velocidade dos pacotes é calculada com base em uma velocidade base,
 * incrementada por um valor que depende da pontuação atual do jogo.
 * A nova velocidade é então aplicada às três esteiras: superior, central e inferior.
 */
void Game::updatePackageSpeed() {
    float newSpeed =
        PACKAGE_SPEED_BASE + (static_cast<int>(score / SCORE_THRESHOLD) * PACKAGE_SPEED_INCREMENT);
    threadmillTop.setPackageSpeed(newSpeed);
    threadmillCenter.setPackageSpeed(newSpeed);
    threadmillBottom.setPackageSpeed(newSpeed);
}

/**
 * @brief Atualiza o intervalo de spawn dos pacotes com base na pontuação atual.
 *
 * Esta função ajusta o intervalo de spawn dos pacotes conforme a pontuação do jogo aumenta.
 * A cada vez que a pontuação atinge um múltiplo do SCORE_THRESHOLD, o intervalo de spawn
 * é decrementado por PACKAGE_SPAWN_INTERVAL_DECREMENT, até um valor mínimo definido por 
 * PACKAGE_SPAWN_INTERVAL_MIN.
 *
 * @note Esta função deve ser chamada periodicamente para garantir que o intervalo de spawn
 * seja atualizado conforme a pontuação do jogo aumenta.
 */
void Game::updatePackageSpawnInterval() {
    while (score >= (spawnIntervalSteps + 1) * SCORE_THRESHOLD) {
        currentSpawnInterval -= PACKAGE_SPAWN_INTERVAL_DECREMENT;
        if (currentSpawnInterval < PACKAGE_SPAWN_INTERVAL_MIN) {
            currentSpawnInterval = PACKAGE_SPAWN_INTERVAL_MIN;
        }
        spawnIntervalSteps++;
    }
}

/**
 * @brief Reinicia o estado do jogo para os valores iniciais.
 * 
 * Esta função redefine a pontuação, vidas e intervalos de spawn para os valores iniciais.
 * Além disso, limpa todos os pacotes das esteiras e adiciona um novo pacote na esteira central.
 * 
 * @details
 * - A pontuação é redefinida para SCORE_INITIAL.
 * - As vidas são redefinidas para MAX_LIVES.
 * - Atualiza os textos de pontuação e vidas na interface.
 * - Atualiza a velocidade dos pacotes.
 * - Define o intervalo de spawn atual para PACKAGE_SPAWN_INTERVAL_BASE.
 * - Limpa todos os pacotes das esteiras superior, central e inferior.
 * - Adiciona um novo pacote na esteira central com um novo identificador.
 */
void Game::resetGame() {
    score = SCORE_INITIAL;
    lives = MAX_LIVES;
    updateScoreText();
    updateLivesText();

    updatePackageSpeed();

    currentSpawnInterval = PACKAGE_SPAWN_INTERVAL_BASE;
    spawnIntervalSteps = 0;

    threadmillTop.clearPackages();
    threadmillCenter.clearPackages();
    threadmillBottom.clearPackages();

    threadmillCenter.addPackage(nextId++);
}

/**
 * @brief Retorna a esteira correspondente à faixa especificada.
 *
 * Esta função recebe um número de faixa (lane) e retorna um ponteiro para a
 * esteira correspondente. As faixas são mapeadas da seguinte forma:
 * - 0: Esteira superior
 * - 1: Esteira central
 * - 2: Esteira inferior
 *
 * @param lane O número da faixa (0, 1 ou 2).
 * @return Um ponteiro para a esteira correspondente à faixa especificada,
 *         ou nullptr se a faixa for inválida.
 */
Threadmill *Game::getThreadmillByLane(int lane) {
    switch (lane) {
    case 0:
        return &threadmillTop;
    case 1:
        return &threadmillCenter;
    case 2:
        return &threadmillBottom;
    default:
        return nullptr;
    }
}

/**
 * @brief Atualiza as esteiras rolantes ativas no jogo.
 *
 * Esta função desativa todas as esteiras rolantes (superior, central e inferior)
 * e ativa apenas a esteira rolante correspondente à faixa atual do jogador.
 */
void Game::updateActiveThreadmills() {
    threadmillTop.deactivate();
    threadmillCenter.deactivate();
    threadmillBottom.deactivate();

    int currentLane = player.getCurrentLane();
    Threadmill *currentThreadmill = getThreadmillByLane(currentLane);
    if (currentThreadmill) {
        currentThreadmill->activate();
    }
}
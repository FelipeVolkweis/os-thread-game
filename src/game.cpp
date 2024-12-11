#include <iostream>

#include <game.h>

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

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::loadAssets() {
    if (!font.loadFromFile(FONT_PATH)) {
        std::cout << "Error loading font." << std::endl;
    }

    if (!Package::loadTexture()) {
        std::cout << "Error loading package texture." << std::endl;
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::KeyPressed)
            handlePlayerAction(event.key.code);
    }
}

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

void Game::updateScoreText() {
    textScore.setString("Score: " + std::to_string(score));
}

void Game::updateLivesText() {
    textLives.setString("Vidas: " + std::to_string(lives));
}

void Game::updatePackageSpeed() {
    float newSpeed =
        PACKAGE_SPEED_BASE + (static_cast<int>(score / SCORE_THRESHOLD) * PACKAGE_SPEED_INCREMENT);
    threadmillTop.setPackageSpeed(newSpeed);
    threadmillCenter.setPackageSpeed(newSpeed);
    threadmillBottom.setPackageSpeed(newSpeed);
}

void Game::updatePackageSpawnInterval() {
    while (score >= (spawnIntervalSteps + 1) * SCORE_THRESHOLD) {
        currentSpawnInterval -= PACKAGE_SPAWN_INTERVAL_DECREMENT;
        if (currentSpawnInterval < PACKAGE_SPAWN_INTERVAL_MIN) {
            currentSpawnInterval = PACKAGE_SPAWN_INTERVAL_MIN;
        }
        spawnIntervalSteps++;
    }
}

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
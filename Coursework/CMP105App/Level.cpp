#include "Level.h"

Level::Level(sf::RenderWindow& hwnd, Input& in) :
    BaseLevel(hwnd, in), m_timerText(m_font), m_winText(m_font), m_highScores(m_font)
{
    m_isGameOver = false;

    sf::Vector2f levelSize = { 800.f, 500.f };
    m_levelBounds = { {0.f, 0.f}, {levelSize } };

    // Initialise the Player (Rabbit)
    m_playerRabbit = new Rabbit();
    if (!m_rabbitTexture.loadFromFile("gfx/rabbit_sheet.png")) std::cerr << "no rabbit texture";
    if (!m_sheepTexture.loadFromFile("gfx/sheep_sheet.png")) std::cerr << "no sheep texture";
    m_playerRabbit->setSize({ 32,32 });
    m_playerRabbit->setInput(&m_input);
    m_playerRabbit->setTexture(&m_rabbitTexture);
    m_playerRabbit->setWorldSize(levelSize.x, levelSize.y);

    loadLevel("data/level.txt", levelSize);

    //// Initialise Sheep (initial position AND pointer to the rabbit
    //for (int i = 0; i < 3; i++)
    //{
    //    m_sheepList.push_back(new Sheep(sf::Vector2f(200.f + 100 * i, 400.f - 100 * i), m_playerRabbit));
    //    m_sheepList[i]->setTexture(&m_sheepTexture);
    //    m_sheepList[i]->setSize({ 32,32 });
    //    m_sheepList[i]->setWorldSize(levelSize.x, levelSize.y);
    //}

    // Initialise Timer
    m_gameTimer.restart();

    // UI Setup
    if (!m_font.openFromFile("font/arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }

    m_timerText.setFont(m_font);
    m_timerText.setCharacterSize(24);
    m_timerText.setFillColor(sf::Color::White);

    // "Game Over" text setup
    m_winText.setFont(m_font);
    m_winText.setString("ROUND COMPLETE!");
    m_winText.setCharacterSize(50);
    m_winText.setFillColor(sf::Color::Blue);
    m_winText.setPosition({ -1000.f, 100.f });  // outside of view

    m_highScores.setFont(m_font);
    m_highScores.setFillColor(sf::Color(0xFFFFFFFF));
    m_highScores.setCharacterSize(24);
    m_highScores.setPosition({ 400.f,250.f });


    //// setup goal
    //m_goal.setSize({ 50, 50 });
    //m_goal.setFillColor(sf::Color::Blue);
    //m_goal.setPosition({ 250, 250 });
    //m_goal.setCollisionBox({ { 0,0 }, { 50,50 } });

    // setup walls
    //for (int i = 0; i < 2; i++)
    //{
    //    GameObject wall;
    //    wall.setPosition({ 100.f + i * 600, 100.f});
    //    wall.setSize({ 50,300 });
    //    wall.setFillColor(sf::Color::Black);
    //    wall.setCollisionBox({ { 0,0 }, { 50,300 } });
    //    m_walls.push_back(wall);
    //}

    m_bgFarm.setFillColor(sf::Color::Green);
    m_bgFarm.setSize(levelSize);
    

}

Level::~Level()
{
	// We made a lot of **new** things, allocating them on the heap
	// So now we have to clean them up
	delete m_playerRabbit;
	for (Sheep* s : m_sheepList)
	{
		delete s;
	}
	m_sheepList.clear();
}

void Level::UpdateCamera()
{
    sf::View view = m_window.getView();
    sf::Vector2f targetPos = m_playerRabbit->getPosition(); 

    sf::Vector2f viewSize = view.getSize();
    sf::Vector2f halfSize = { viewSize.x / 2.f, viewSize.y / 2.f };

    sf::Vector2f newCenter;

    newCenter.x = std::clamp(targetPos.x,
        m_levelBounds.position.x + halfSize.x,
        m_levelBounds.size.x - halfSize.x);

    newCenter.y = std::clamp(targetPos.y,
        m_levelBounds.position.y + halfSize.y,
        m_levelBounds.size.y - halfSize.y);

    view.setCenter(newCenter);
    
    // top-left corner of the current view in world space
    sf::Vector2f viewTopLeft = newCenter - halfSize;
    // Position text at the bottom-left of the current view with a small margin (e.g., 30px)
    float margin = 30.f;
    m_timerText.setPosition({ viewTopLeft.x + margin, viewTopLeft.y + viewSize.y - margin });

    m_window.setView(view);
}

bool Level::CheckWinCondition()
{
    for (auto s : m_sheepList) if (s->isAlive()) return false;
    m_winText.setPosition({ 100.f, 100.f });
 
    return true;
}

void Level::writeHighScore(float time) {

    std::ofstream highScoreFile("Data/data.txt", std::ios::app);
    if (!highScoreFile) std::cerr << "No file exists, may not be able to create a new one... \n";

    highScoreFile << std::fixed << std::setprecision(3) << time << "\n";

    std::cout << "Saved high score.\n";
    highScoreFile.close();

    sortHighScore();
}

void Level::sortHighScore() {

    std::string currentHighScore;
    std::string scoreData;
    std::vector <float> scores;

    std::ifstream highScoreRead("Data/data.txt");
    if (!highScoreRead) std::cerr << "There is no data!\n";

    while (std::getline(highScoreRead, currentHighScore)) {
        scores.push_back(std::stof(currentHighScore));
        std::cout << "added a high score to the vector\n";
    }

    //only sort if needed
    if (scores.size() < 2) {
        highScoreRead.close();
        std::cout << "Sorting is impossible - returning early!\n";
        return;
    }        
    else std::sort(scores.begin(), scores.end(), std::greater<float>());
        
    std::ofstream highScoreWrite("Data/data.txt");
    if (!highScoreWrite) std::cerr << "There is no data!\n"; //nothing *should've* changed, but just in case....

    for (float a : scores)
    highScoreWrite << std::fixed << std::setprecision(3) << a << "\n";

    highScoreWrite.close();

}

void Level::displayHighScores() {

    std::ifstream highScoreFile("Data/data.txt");
    if (!highScoreFile) std::cerr << "There is no data!\n";

    std::string currentHighScore;
    std::string scoreData;

    while (std::getline(highScoreFile, currentHighScore)) {
        std::cout << "adding score " << currentHighScore << " to the scores string...\n";
        scoreData = scoreData + currentHighScore + "\n";
    }

    m_highScores.setString(scoreData);
    std::cout << "Displayed sorted high scores.\n";
    highScoreFile.close();
}

void Level::loadLevel(std::string filename, sf::Vector2f worldSize) {

    std::ifstream levelFile("Data/level.txt");
    if (!levelFile) std::cerr << "There is no level data file!\n";

    std::string type;

    int numberOfSheep = 0;

    float xPos;
    float yPos;
    float width;
    float height;

    while (levelFile >> type) {

        if (type == "RABBIT") {
            levelFile >> xPos >> yPos;
            m_playerRabbit->setPosition({ xPos, yPos });
        }
        if (type == "SHEEP") {
            levelFile >> xPos >> yPos;
            m_sheepList.push_back(new Sheep({xPos, yPos}, m_playerRabbit));
            m_sheepList[numberOfSheep]->setTexture(&m_sheepTexture);
            m_sheepList[numberOfSheep]->setSize({ 32,32 });
            m_sheepList[numberOfSheep]->setWorldSize(worldSize.x, worldSize.y);
            numberOfSheep++;
        }
        if (type == "WALL") {
            levelFile >> xPos >> yPos >> width >> height;
            GameObject wall;
            wall.setPosition({xPos,yPos});
            wall.setSize({ width,height });
            wall.setFillColor(sf::Color::Black);
            wall.setCollisionBox({ { 0,0 }, { width,height } });
            m_walls.push_back(wall);
        }
        if (type == "GOAL") {
            levelFile >> xPos >> yPos >> width >> height;
            m_goal.setPosition({ xPos, yPos });
            m_goal.setSize({ width, height });
            m_goal.setFillColor(sf::Color::Blue);
            m_goal.setCollisionBox({ { 0,0 }, { width,height } });

        }

    }

}


// handle user input
void Level::handleInput(float dt)
{
	m_playerRabbit->handleInput(dt);
}

// checks and manages sheep-sheep, sheep-wall, sheep-goal, player-wall
void Level::manageCollisions()
{
    for (int i = 0; i < m_sheepList.size(); i++)
    {
        if (!m_sheepList[i]->isAlive()) continue;   // ignore scored sheep.

        // sheep collide with walls, with other sheep and with the goal.
        for (auto wall : m_walls)
        {
            if (Collision::checkBoundingBox(wall, *m_sheepList[i]))
            {
                m_sheepList[i]->collisionResponse(wall);
            }
        }
        for (int j = i + 1; j < m_sheepList.size(); j++)
        {
            if(!m_sheepList[j]->isAlive()) continue; // ignore scored sheep here too
            if (Collision::checkBoundingBox(*m_sheepList[i], *m_sheepList[j]))
            {
                // DANGER check i and j carefully here team.
                m_sheepList[i]->collisionResponse(*m_sheepList[j]);
                m_sheepList[j]->collisionResponse(*m_sheepList[i]);
            }
        }
        if (Collision::checkBoundingBox(*m_sheepList[i], m_goal))
            m_sheepList[i]->collideWithGoal(m_goal);
    }
    for (auto wall : m_walls)
    {
        if (Collision::checkBoundingBox(wall, *m_playerRabbit))
        {
            m_playerRabbit->collisionResponse(wall);
        }
    }
}

// Update game objects
void Level::update(float dt)
{
    if (m_isGameOver) return;   // if the game is over, don't continue trying to process game logic

    m_playerRabbit->update(dt);
    for (Sheep* s : m_sheepList)
    {
        if (s->isAlive()) s->update(dt);
    }

    // Timer 
    float timeElapsed = m_gameTimer.getElapsedTime().asSeconds();
    m_timerText.setString("Time: " + std::to_string(static_cast<int>(timeElapsed)));


    manageCollisions();
    UpdateCamera();
    m_isGameOver = CheckWinCondition();
    if (m_isGameOver) {
        writeHighScore(timeElapsed);
        displayHighScores();
    }

}

// Render level
void Level::render()
{
	beginDraw();
    m_window.draw(m_bgFarm);
    m_window.draw(m_goal);
    for (auto w : m_walls) m_window.draw(w);
    for (auto s : m_sheepList)
    {
        if (s->isAlive()) m_window.draw(*s);
    }
    m_window.draw(*m_playerRabbit);
    m_window.draw(m_timerText);
    m_window.draw(m_winText);
    m_window.draw(m_highScores);
    
	endDraw();
}



#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp> 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <random>
#ifdef _WIN32
#include <windows.h> 
#endif
#include <thread> 
#include <chrono> 

using namespace std;

void closeConsole() {
#ifdef _WIN32
    FreeConsole();
#endif
}

class Character {
public:
    string name;
    int health;
    int attackPower;
    int magicPower;
    int defense;
    int level;

    Character(string n, int h, int a, int m, int d = 0, int lvl = 1)
        : name(n), health(h), attackPower(a), magicPower(m), defense(d), level(lvl) {
    }

    void attack(Character& enemy) {
        int damage = rand() % (attackPower + 1);  // Random value between 0 and attackPower
        if (damage == 0) {
            damage = 1;  // Ensure at least 1 damage is dealt
        }
        cout << name << " attacks " << enemy.name << " for " << damage << " damage!" << endl;
        enemy.health -= damage;
    }

    void useMagic(Character& enemy) {
        int magicDamage = magicPower;
        cout << name << " uses magic on " << enemy.name << " for " << magicDamage << " damage!" << endl;
        enemy.health -= magicDamage;
    }

    bool isAlive() {
        return health > 0;
    }

    void levelUp() {
        level++;
        attackPower += 5;
        magicPower += 5;
        health += 20;
        defense += 2;
        cout << name << " leveled up to level " << level << "!" << endl;
    }
};

// Function for Battle Simulation in Level 1
std::string startBattle() {
    srand(time(0)); // Random number for encounters

    // 1. Creating player and enemy characters
    Character player("Hero", 100, 20, 30, 5);  // Player with health, attack power, defense
    Character enemy("Dragon", 150, 25, 40, 10); // Adjusted enemy health to 150

    bool gameOver = false;
    std::string unlockedLetters; // Variable to hold unlocked letters

    // Battle Simulation (Level 1)
    while (!gameOver) {
        cout << "\nYour Health: " << player.health << " | Enemy's Health: " << enemy.health << endl;
        cout << "Choose an action:" << endl;
        cout << "1. Attack" << endl;
        cout << "2. Use Magic" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
        case 1:
            player.attack(enemy);
            break;
        case 2:
            player.useMagic(enemy);
            break;
        default:
            cout << "Invalid choice! Please choose again." << endl;
            continue;
        }

        // Check if the enemy is defeated
        if (!enemy.isAlive()) {
            cout << enemy.name << " has been defeated!" << endl;
            player.levelUp();  // Level up the player after defeating the enemy
            cout << "\n" << player.name << "'s Updated Stats:" << endl;
            cout << "Health: " << player.health << endl;
            cout << "Attack Power: " << player.attackPower << endl;
            cout << "Magic Power: " << player.magicPower << endl;
            cout << "Defense: " << player.defense << endl;

            // Close the console before starting the Snake game
            closeConsole();

          
            unlockedLetters = "Euat"; // Set the unlocked letters
            cout << "\nCongratulations! You have unlocked the following letters: " << unlockedLetters << endl;

            gameOver = true;
        }

        // Enemy attacks (randomized damage)
        cout << enemy.name << " attacks you!" << endl;
        int damage = rand() % (enemy.attackPower + 1);
        if (damage == 0) {
            damage = 1;  // Ensure at least 1 damage is dealt
        }

        player.health -= damage;
        cout << enemy.name << " deals " << damage << " damage to " << player.name << "!" << endl;

        // Check if the player is defeated
        if (!player.isAlive()) {
            cout << player.name << " has been defeated!" << endl;
            gameOver = true;
        }
    }

    return unlockedLetters; // Return the unlocked letters
}

template <typename T>
T customclamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Star class definition
class Star {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Star(float x, float y, float radius, sf::Vector2f velocity) {
        shape.setPosition(x, y);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::White);
        shape.setOrigin(radius, radius);
        this->velocity = velocity;
    }

    void update(float dt, sf::RenderWindow& window) {
        shape.move(velocity * dt);
        if (shape.getPosition().x < 0 || shape.getPosition().x > window.getSize().x ||
            shape.getPosition().y < 0 || shape.getPosition().y > window.getSize().y) {
            reset(window);
        }
    }

private:
    void reset(sf::RenderWindow& window) {
        shape.setPosition(rand() % window.getSize().x, rand() % window.getSize().y);
        velocity = sf::Vector2f((rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f);
    }
};

void recreateWindow(sf::RenderWindow& window, bool fullscreen) {
    window.close();
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    if (fullscreen) {
        window.create(desktopMode, "Fantasy Adventure Game", sf::Style::Fullscreen);
    }
    else {
        window.create(sf::VideoMode(1280, 720), "Fantasy Adventure Game", sf::Style::Default);
    }
    window.setFramerateLimit(60);
}

void centerText(sf::Text& text, const sf::RenderWindow& window, float yOffset) {
    text.setPosition(window.getSize().x / 2 - text.getGlobalBounds().width / 2, yOffset);
}

struct pair_hash {
    template <typename T>
    std::size_t operator()(const T& v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

struct SnakeNode {
    sf::Vector2i position;
    std::shared_ptr<SnakeNode> prev;
    std::shared_ptr<SnakeNode> next;

    SnakeNode(int x, int y) : position(x, y), prev(nullptr), next(nullptr) {}
};

class SnakeGame {
public:
    SnakeGame() : window(sf::VideoMode(800, 600), "Snake Game"), snakeDirection(0, 0), score(0), gameOver(false) {
        srand(static_cast<unsigned int>(time(0)));

        snakeHead = std::make_shared<SnakeNode>(400, 300);
        snakeTail = snakeHead;

        spawnFood();
        spawnObstacles();
    }

    void run() {
        sf::Clock clock;
        float deltaTime = 0.0f;
        while (window.isOpen()) {
            float elapsedTime = clock.restart().asSeconds();
            deltaTime += elapsedTime;

            processEvents();
            if (deltaTime > 0.1f) {
                update();
                deltaTime = 0.0f;
            }
            render();
        }
    }

    bool shouldExit() const {
        return exitGame;
    }

private:
    sf::RenderWindow window;
    std::shared_ptr<SnakeNode> snakeHead;
    std::shared_ptr<SnakeNode> snakeTail;
    sf::Vector2i snakeDirection;
    sf::Vector2i foodPosition;
    int score;
    bool gameOver;
    bool exitGame = false;
    std::unordered_set<sf::Vector2i, pair_hash> obstacles;
    static const int blockSize = 20;
    static const int gridWidth = 40;
    static const int gridHeight = 30;
    static const int windowWidth = 800;
    static const int windowHeight = 600;

    void quitGame() {
        exitGame = true; // Set the exit flag
        window.close(); // Close the window
    }

    void processEvents() {
        sf::Event event;
        while
            (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                quitGame(); // Call quitGame when the window is closed
            }

            if (event.type == sf::Event::KeyPressed) {
                if (!gameOver) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        quitGame(); // Call quitGame when Backspace is pressed
                    }
                    if (event.key.code == sf::Keyboard::S && snakeDirection != sf::Vector2i(0, blockSize)) {
                        snakeDirection = sf::Vector2i(0, -blockSize);
                    }
                    else if (event.key.code == sf::Keyboard::Down && snakeDirection != sf::Vector2i(0, -blockSize)) {
                        snakeDirection = sf::Vector2i(0, blockSize);
                    }
                    else if (event.key.code == sf::Keyboard::Left && snakeDirection != sf::Vector2i(blockSize, 0)) {
                        snakeDirection = sf::Vector2i(-blockSize, 0);
                    }
                    else if (event.key.code == sf::Keyboard::Right && snakeDirection != sf::Vector2i(-blockSize, 0)) {
                        snakeDirection = sf::Vector2i(blockSize, 0);
                    }
                }

                if (gameOver && event.key.code == sf::Keyboard::R) {
                    resetGame();
                }
            }
        }
    }

    void update() {
        if (gameOver || exitGame) return; // Exit if the game is over or exit flag is set

        int newHeadX = snakeHead->position.x + snakeDirection.x;
        int newHeadY = snakeHead->position.y + snakeDirection.y;

        if (newHeadX < 0 || newHeadX >= windowWidth || newHeadY < 0 || newHeadY >= windowHeight) {
            gameOver = true;
            return;
        }

        std::unordered_set<sf::Vector2i, pair_hash> snakePositions;
        auto current = snakeHead;
        while (current) {
            if (snakePositions.count(current->position)) {
                gameOver = true;
                return;
            }
            snakePositions.insert(current->position);
            current = current->next;
        }

        if (obstacles.count({ newHeadX, newHeadY })) {
            gameOver = true;
            return;
        }

        auto newHead = std::make_shared<SnakeNode>(newHeadX, newHeadY);
        newHead->next = snakeHead;
        snakeHead->prev = newHead;
        snakeHead = newHead;

        if (newHead->position == foodPosition) {
            score += 10;
            spawnFood();
        }
        else {
            snakeTail = snakeTail->prev;
            snakeTail->next = nullptr;
        }
    }

    void render() {
        window.clear(sf::Color::Black);

        sf::RectangleShape border(sf::Vector2f(windowWidth, windowHeight));
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2);
        window.draw(border);

        auto current = snakeHead;
        while (current) {
            if (current == snakeHead) {
                sf::CircleShape headShape(blockSize / 2);
                headShape.setPosition(current->position.x, current->position.y);
                headShape.setFillColor(sf::Color::Yellow);
                window.draw(headShape);
            }
            else {
                sf::RectangleShape segmentShape(sf::Vector2f(blockSize, blockSize));
                segmentShape.setPosition(current->position.x, current->position.y);
                segmentShape.setFillColor(sf::Color::Green);
                segmentShape.setOutlineColor(sf::Color::Black);
                segmentShape.setOutlineThickness(1);
                window.draw(segmentShape);
            }
            current = current->next;
        }

        sf::CircleShape foodBlock(blockSize / 2);
        foodBlock.setPosition(foodPosition.x, foodPosition.y);
        foodBlock.setFillColor(sf::Color::Red);
        window.draw(foodBlock);

        for (const auto& obstacle : obstacles) {
            sf::RectangleShape obstacleShape(sf::Vector2f(blockSize, blockSize));
            obstacleShape.setPosition(obstacle.x, obstacle.y);
            obstacleShape.setFillColor(sf::Color::Blue);
            window.draw(obstacleShape);
        }

        sf::Font font;
        if (!font.loadFromFile("D:/arial.ttf")) {
            cout << "Error loading font!" << endl;
        }
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString("Score: " + to_string(score));
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);
        window.draw(scoreText);

        if (gameOver) {
            sf::Text gameOverText("Game Over! Press 'R' to Restart ", font, 30);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition(200, 250);
            window.draw(gameOverText);
        }

        window.display();
    }

    void spawnFood() {
        foodPosition.x = (rand() % gridWidth) * blockSize;
        foodPosition.y = (rand() % gridHeight) * blockSize;

        while (isFoodOnSnake() || obstacles.count(foodPosition)) {
            foodPosition.x = (rand() % gridWidth) * blockSize;
            foodPosition.y = (rand() % gridHeight) * blockSize;
        }
    }

    void spawnObstacles() {
        int numObstacles = 5;
        for (int i = 0; i < numObstacles; ++i) {
            int obstacleX = (rand() % gridWidth) * blockSize;
            int obstacleY = (rand() % gridHeight) * blockSize;

            while (obstacles.count({ obstacleX, obstacleY }) || isFoodOnSnake() || snakeHead->position == sf::Vector2i(obstacleX, obstacleY)) {
                obstacleX = (rand() % gridWidth) * blockSize;
                obstacleY = (rand() % gridHeight) * blockSize;
            }

            obstacles.insert({ obstacleX, obstacleY });
        }
    }

    bool isFoodOnSnake() {
        auto current = snakeHead;
        while (current) {
            if (current->position == foodPosition) return true;
            current = current->next;
        }
        return false;
    }

    void resetGame() {
        snakeHead = std::make_shared<SnakeNode>(400, 300);
        snakeTail = snakeHead;
        snakeDirection = sf::Vector2i(0, 0);
        score = 0;
        spawnFood();
        obstacles.clear();
        spawnObstacles();
        gameOver = false;
        exitGame = false; // Reset exit flag
    }
};
// Utility function to convert a string to lowercase
string toLowerCase(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// Class for managing riddles and their solutions
class RiddleGame {
private:
    struct Riddle {
        string question;
        string answer;
        vector<string> hints; // Stores up to 3 hints
    };

    vector<Riddle> riddles; // Store riddles in a vector
    int score;              // Player's score
    string unlockedLetters; // Track unlocked letters
    int currentRiddleIndex; // Index for current riddle

    // Populate riddles and hints
    void initializeRiddles() {
        riddles.push_back({ "I speak without a mouth and hear without ears. I have no body, but I come alive with the wind. What am I?", "echo", {"It's a sound.", "It's something intangible.", "It repeats itself."} });
        riddles.push_back({ "The more of this you take, the more you leave behind. What is it?", "footsteps", {"You make them when you walk.", "They are visible in sand or snow.", "They are left behind you."} });
        riddles.push_back({ "I am not alive, but I can grow. I don't have lungs, but I need air. What am I?", "fire", {"It's a natural phenomenon.", "It consumes and spreads.", "It can be extinguished."} });
        riddles.push_back({ "What has keys but can't open locks?", "piano", {"It's a musical instrument.", "It produces sound.", "You play it with your fingers."} });
        riddles.push_back({ "I am tall when I am young, and I am short when I am old. What am I?", "candle", {"It provides light.", "It burns down over time.", "It has a wick."} });
        riddles.push_back({ "What has a head, a tail, but no body?", "coin", {"It’s a small object.", "It’s used in money transactions.", "It has two sides."} });
        riddles.push_back({ "What can travel around the world while staying in the corner?", "stamp", {"It's related to mailing.", "It sticks to something.", "It has a small image on it."} });
        riddles.push_back({ "What gets wetter as it dries?", "towel", {"It's used after washing.", "It’s soft and absorbent.", "It’s used to dry things."} });
        riddles.push_back({ "What comes once in a minute, twice in a moment, but never in a thousand years?", "m", {"It’s a letter.", "It’s in the word 'minute'.", "It’s not a number."} });
        riddles.push_back({ "What is always in front of you but can’t be seen?", "future", {"It hasn't happened yet.", "It’s what’s to come.", "It can’t be predicted with certainty."} });
    }

    // Shuffle riddles to make the order random
    void shuffleRiddles() {
        unsigned seed = static_cast<unsigned>(time(0));  // Convert time_t to unsigned
        default_random_engine rng(seed); // Create random engine with seed
        shuffle(riddles.begin(), riddles.end(), rng); // Shuffle using the random engine
    }

    // Check if a letter should be unlocked based on the score
    char checkForUnlock() {
        char unlockedLetter = '\0'; // Initialize to null character
        if (score >= 70 && unlockedLetters.find('a') == string::npos) {
            unlockedLetters += 'a';
            unlockedLetter = 'a';
        }
        if (score >= 140 && unlockedLetters.find('e') == string::npos) {
            unlockedLetters += 'e';
            unlockedLetter = 'e';
        }
        if (score >= 210 && unlockedLetters.find('v') == string::npos) {
            unlockedLetters += 'v';
            unlockedLetter = 'v';
        }
        return unlockedLetter; // Return the unlocked letter
    }

    // Display the unlock message
    void displayUnlockMessage(sf::RenderWindow& window, sf::Text& text, char letter) {
        string unlockMessage = "You've unlocked the letter: " + string(1, letter);
        text.setString(unlockMessage);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 300); // Position the unlock message

        window.clear();
        window.draw(text);
        window.display();
        sf::sleep(sf::seconds(2)); // Pause to show the unlock message
    }

    // Check if the game is won
    bool checkWinCondition() {
        return unlockedLetters == "aev"; // If all letters are unlocked
    }

public:
    RiddleGame() : score(0), unlockedLetters(""), currentRiddleIndex(0) {
        initializeRiddles();
        shuffleRiddles(); // Randomize the order of riddles
    }

    // Method to display the score bar
    void drawScoreBar(sf::RenderWindow& window) {
        float maxWidth = 500.0f; // Maximum width for the score bar
        float currentWidth = (score / 500.0f) * maxWidth; // Scale the current score

        sf::RectangleShape scoreBar(sf::Vector2f(currentWidth, 20)); // Height of 20px
        scoreBar.setFillColor(sf::Color::Green); // Color of the score bar
        scoreBar.setPosition(150, 50); // Position the score bar at the top of the screen

        window.draw(scoreBar);
    }

    // Method to display the current riddle and take user input via SFML
    void displayRiddle(sf::RenderWindow& window, sf::Text& text, const string& riddleText, sf::Font& billoFont) {
        text.setFont(billoFont);
        text.setString(riddleText);
        text.setCharacterSize(30); // Set font size to make it readable
        text.setFillColor(sf::Color::White); // Set text color to white
        text.setPosition(100, 200);  // Adjust position to fit screen

        window.draw(text);  // Draw the riddle question text
    }

    // Method to display score and riddle number
    void displayScoreAndRiddleNumber(sf::RenderWindow& window, sf::Text& text, sf::Font& font) {
        string scoreText = "Score: " + to_string(score);
        string riddleNumberText = "Riddle: " + to_string(currentRiddleIndex + 1) + "/" + to_string(riddles.size());

        text.setString(scoreText + "\n" + riddleNumberText);
        text.setFont(font);  // Use the Augusta font for the score and riddle number
        window.clear();
        window.draw(text);
        window.display();
    }

    // Method to display hint
    void displayHint(sf::RenderWindow& window, sf::Text& text, const string& hint) {
        text.setString("Hint: " + hint);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 300); // Position the hint message

        window.clear();
        window.draw(text);
        window.display();
        sf::sleep(sf::seconds(2)); // Pause to show the hint message
    }

    // Method to play the game
    void playGame(sf::RenderWindow& window, sf::Text& text, sf::Font& font, sf::Font& billoFont) {
        while (currentRiddleIndex < riddles.size()) {
            Riddle& riddle = riddles[currentRiddleIndex];  // Ensure that riddle is non-const
            string question = riddle.question;
            string correctAnswer = riddle.answer;
            string userAnswer;
            bool answeredCorrectly = false;

            // Display the score bar and riddle number
            displayScoreAndRiddleNumber(window, text, font);

            // Display riddle question using Billo font
            window.clear();
            drawScoreBar(window);
            displayRiddle(window, text, question, billoFont);
            window.display();

            bool waitingForInput = true;

            // Create squares (textboxes) for each letter in the answer
            vector<sf::RectangleShape> answerSquares;
            float squareWidth = 50;
            float squareHeight = 50;
            for (size_t i = 0; i < correctAnswer.size(); ++i) {
                sf::RectangleShape square(sf::Vector2f(squareWidth, squareHeight));
                square.setOutlineColor(sf::Color::White);
                square.setOutlineThickness(2);
                square.setPosition(100 + i * (squareWidth + 10), 400);  // Position squares
                answerSquares.push_back(square);
            }

            vector<sf::Text> letterTexts;
            for (size_t i = 0; i < correctAnswer.size(); ++i) {
                sf::Text letterText;
                letterText.setFont(billoFont);
                letterText.setCharacterSize(30);
                letterText.setFillColor(sf::Color::Black); // Set the text color to black for the letters
                letterText.setPosition(110 + i * (squareWidth + 10), 420);  // Position letters centered inside squares
                letterTexts.push_back(letterText);
            }

            // Create Hint and Skip buttons
            sf::RectangleShape hintButton(sf::Vector2f(100, 50));
            hintButton.setFillColor(sf::Color::Blue);
            hintButton.setPosition(100, 500);

            sf::RectangleShape skipButton(sf::Vector2f(100, 50));
            skipButton.setFillColor(sf::Color::Red);
            skipButton.setPosition(250, 500);

            while (waitingForInput) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window.close();
                    }

                    if (event.type == sf::Event::TextEntered) {
                        if (event.text.unicode == '\b' && !userAnswer.empty()) {
                            userAnswer.pop_back();
                        }
                        else if (isalpha(event.text.unicode)) {
                            userAnswer += static_cast<char>(event.text.unicode);
                        }

                        // Update answer textboxes
                        for (size_t i = 0; i < userAnswer.size(); ++i) {
                            letterTexts[i].setString(string(1, userAnswer[i]));
                        }
                    }

                    // Check if the player has guessed the answer correctly
                    if (userAnswer.size() == correctAnswer.size()) {
                        if (toLowerCase(userAnswer) == toLowerCase(correctAnswer)) {
                            answeredCorrectly = true;
                            score += 100;
                            waitingForInput = false;
                        }
                    }

                    // Display Hint button logic
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (hintButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !riddle.hints.empty()) {
                            score -= 15;
                            displayHint(window, text, riddle.hints[0]);
                            riddle.hints.erase(riddle.hints.begin()); // Erase the first hint from non-const riddle object
                        }
                    }

                    // Display Skip button logic
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (skipButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            waitingForInput = false;
                            score -= 20;
                        }
                    }
                }

                // Draw elements (buttons, textboxes, etc.)
                window.clear();
                window.draw(hintButton);
                window.draw(skipButton);
                for (auto& square : answerSquares) {
                    window.draw(square);
                }
                for (auto& letterText : letterTexts) {
                    window.draw(letterText);
                }
                drawScoreBar(window);  // Draw score bar again
                displayRiddle(window, text, question, billoFont);
                window.display();
            }

            if (answeredCorrectly) {
                // Check if the player has unlocked a letter after answering correctly
                char unlockedLetter = checkForUnlock();
                if (unlockedLetter != '\0') {
                    displayUnlockMessage(window, text, unlockedLetter);
                }
            }

            // Check win condition
            if (checkWinCondition()) {
                text.setString("You win! All letters unlocked!");
                text.setCharacterSize(50);
                text.setPosition(100, 300); // Position at the center
                text.setFillColor(sf::Color::White);
                window.clear();
                window.draw(text);
                window.display();
                sf::sleep(sf::seconds(2)); // Wait before closing the game
                break;
            }

            // Move to the next riddle
            ++currentRiddleIndex;
        }

        // If all riddles are completed, display the final score
        text.setString("Game Over. Your final score: " + to_string(score));
        text.setCharacterSize(50);
        text.setPosition(100, 300);  // Position at the center
        window.clear();
        window.draw(text);
        window.display();
        sf::sleep(sf::seconds(3)); // Wait before closing the game
    }
};

int main() {
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktopMode, "Fantasy Adventure Game", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("D:/arial.ttf")) {
        return -1;
    }

    // Load "Augusta.ot" font for game name
    sf::Font gameNameFont;
    if (!gameNameFont.loadFromFile("D:/SFML/The Augusta.otf")) {
        return -1;
    }

    // Load background music
    sf::Music music; // Ensure this is declared
    if (!music.openFromFile("C:/Users/HP Probook 640/Downloads/music.wav")) {
        return -1;
    }
    music.setLoop(true);
    music.setVolume(50);
    music.play();

    // Subtitle text
    sf::Text subtitle("Press Any Key to Begin...", font, 60);
    subtitle.setFillColor(sf::Color::White);
    subtitle.setPosition(desktopMode.width / 2 - subtitle.getGlobalBounds().width / 2, desktopMode.height - 100);

    // Load background texture for splash screen
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("C:/Users/HP Probook 640/Downloads/animation.jpeg")) {
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(desktopMode.width) / static_cast<float>(backgroundTexture.getSize().x),
        static_cast<float>(desktopMode.height) / static_cast<float>(backgroundTexture.getSize().y)
    );

    // Load menu background texture
    sf::Texture menuBackgroundTexture;
    if (!menuBackgroundTexture.loadFromFile("C:/Users/HP Probook 640/Downloads/texture.jpeg")) { // Update with actual path
        return -1;
    }
    sf::Sprite menuBackgroundSprite(menuBackgroundTexture);
    menuBackgroundSprite.setScale(
        static_cast<float>(desktopMode.width) / static_cast<float>(menuBackgroundTexture.getSize().x),
        static_cast<float>(desktopMode.height) / static_cast<float>(menuBackgroundTexture.getSize().y)
    );

    // Game states
    enum class GameState { SplashScreen, Menu, Settings, Quit, Play };
    GameState gameState = GameState::SplashScreen;
    sf::Text menuItems[3];
    std::string options[] = { "Play", "Settings", "Quit" };
    int selectedIndex = 0;

    for (int i = 0; i < 3; ++i) {
        menuItems[i].setFont(font);
        menuItems[i].setString(options[i]);
        menuItems[i].setCharacterSize(70);
        menuItems[i].setPosition(static_cast<float>(desktopMode.width) / 2 - menuItems[i].getGlobalBounds().width / 2,
            static_cast<float>(desktopMode.height) / 2 + i * 100);
        menuItems[i].setFillColor(i == selectedIndex ? sf::Color::Yellow : sf::Color::White);
    }

    // Game name text using "Augusta.ot" font
    sf::Text gameName("Fantasy Adventure Game", gameNameFont, 100);
    gameName.setFillColor(sf::Color::Cyan);
    gameName.setPosition(desktopMode.width / 2 - gameName.getGlobalBounds().width / 2, 50);

    // Brightness overlay
    sf::RectangleShape brightnessOverlay(sf::Vector2f(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height)));
    brightnessOverlay.setFillColor(sf::Color(0, 0, 0, 0)); // Initially transparent

    // Settings variables
    bool isFullscreen = true;
    int brightnessLevel = 100; // Initial brightness (100 = full brightness)
    int volumeLevel = 50; // Initial volume level (0 to 100)
    int selectedResolutionIndex = 0; // Default resolution index
    std::vector<sf::VideoMode> availableResolutions = {
        sf::VideoMode(1280, 720),
        sf::VideoMode(1920, 1080),
        sf::VideoMode(1366, 768)
    };

    // Settings menu
    sf::Text settingsOptions[4];
    std::string settingsLabels[] = { "Fullscreen: ", "Resolution: ", "Brightness: ", "Volume: " };
    int settingsIndex = 0;

    for (int i = 0; i < 4; ++i) {
        settingsOptions[i].setFont(font);
        settingsOptions[i].setCharacterSize(70);
        settingsOptions[i].setFillColor(i == settingsIndex ? sf::Color::Yellow : sf::Color::White);
        settingsOptions[i].setPosition(static_cast<float>(desktopMode.width) / 2 - settingsOptions[i].getGlobalBounds().width / 2,
            static_cast<float>(desktopMode.height) / 2 + i * 100);
    }

    sf::Text backToMenu("Press Backspace to Go Back", font, 50);
    backToMenu.setFillColor(sf::Color::White);
    backToMenu.setPosition(static_cast<float>(desktopMode.width) / 2 - backToMenu.getGlobalBounds().width / 2, static_cast<float>(desktopMode.height) - 150.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (gameState == GameState::SplashScreen && event.type == sf::Event::KeyPressed) {
                gameState = GameState::Menu;
            }
            else if (gameState == GameState::Menu) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        selectedIndex = (selectedIndex - 1 + 3) % 3;
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        selectedIndex = (selectedIndex + 1) % 3;
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (selectedIndex == 0) {
                            gameState = GameState::Play;
                        }
                        else if (selectedIndex == 1) {
                            gameState = GameState::Settings;
                        }
                        else if (selectedIndex == 2) {
                            window.close();
                        }
                    }

                    for (int i = 0; i < 3; ++i) {
                        menuItems[i].setFillColor(i == selectedIndex ? sf::Color::Yellow : sf::Color::White);
                    }
                }
            }
            else if (gameState == GameState::Settings) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        settingsIndex = (settingsIndex - 1 + 4) % 4;
                    }

                    else if (event.key.code == sf::Keyboard::Down) {
                        settingsIndex = (settingsIndex + 1) % 4;
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        switch (settingsIndex) {
                        case 0:
                            isFullscreen = !isFullscreen;
                            recreateWindow(window, isFullscreen);
                            break;
                        case 1:
                            selectedResolutionIndex = (selectedResolutionIndex + 1) % availableResolutions.size();
                            recreateWindow(window, isFullscreen);
                            break;
                        case 2:
                            brightnessLevel = (brightnessLevel + 10) % 110;
                            break;
                        case 3:
                            volumeLevel = (volumeLevel + 10) % 110;
                            music.setVolume(volumeLevel);
                            break;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Backspace) {
                        gameState = GameState::Menu;
                    }

                    for (int i = 0; i < 4; ++i) {
                        settingsOptions[i].setFillColor(i == settingsIndex ? sf::Color::Yellow : sf::Color::White);
                    }
                }

                // Update settings text
                settingsOptions[0].setString("Fullscreen: " + std::string(isFullscreen ? "On" : "Off"));
                settingsOptions[1].setString("Resolution: " + std::to_string(availableResolutions[selectedResolutionIndex].width) + "x" + std::to_string(availableResolutions[selectedResolutionIndex].height));
                settingsOptions[2].setString("Brightness: " + std::to_string(brightnessLevel) + "%");
                settingsOptions[3].setString("Volume: " + std::to_string(volumeLevel) + "%");

                int alphaValue = static_cast<int>(255 - (brightnessLevel / 100.0f) * 255);
                alphaValue = customclamp(alphaValue, 0, 255);
                brightnessOverlay.setFillColor(sf::Color(0, 0, 0, alphaValue));
            }
            else if (gameState == GameState::Play) {
                static bool storylineDisplayed = false;
                static int gamesCompleted = 0;    // Flag to check if storyline has been displayed

                if (!storylineDisplayed) {
                    // Load the background texture for the storyline screen
                    sf::Texture storylineBackgroundTexture;
                    if (!storylineBackgroundTexture.loadFromFile("C:/Users/HP Probook 640/Downloads/storyline.jpg")) {
                        return -1;
                    }
                    sf::Sprite storylineBackgroundSprite(storylineBackgroundTexture);
                    storylineBackgroundSprite.setScale(
                        static_cast<float>(desktopMode.width) / storylineBackgroundTexture.getSize().x,
                        static_cast<float>(desktopMode.height) / storylineBackgroundTexture.getSize().y
                    );


                    sf::Text storylineText;
                    storylineText.setFont(font);
                    storylineText.setCharacterSize(30);
                    storylineText.setFillColor(sf::Color::Black);
                    centerText(storylineText, window, window.getSize().y / 2 - 100);

                    std::string storyline =
                        "The treasure lies deep within the mystical forest...\n"
                        "To unlock its secrets, you must first find the password.\n"
                        "Only the worthy who conquer the trials of skill can succeed.\n"
                        "Your journey begins now!";

                    std::string displayedText = "";
                    sf::Clock typingClock;

                    // Typing effect
                    size_t currentChar = 0;
                    while (currentChar < storyline.size()) {
                        window.clear();
                        window.draw(storylineBackgroundSprite);

                        if (typingClock.getElapsedTime().asMilliseconds() > 50) {
                            displayedText += storyline[currentChar++];
                            typingClock.restart();
                        }
                        storylineText.setString(displayedText);
                        window.draw(storylineText);
                        window.display();
                    }

                    sf::sleep(sf::seconds(2));
                    sf::Text startText("Press Enter to Begin Your Quest!", font, 30);
                    startText.setFillColor(sf::Color::Yellow);
                    centerText(startText, window, window.getSize().y / 2 + 50);

                    sf::Clock pulseClock;
                    bool increasing = true;
                    float alpha = 0;

                    while (true) {
                        sf::Event event;
                        while (window.pollEvent(event)) {
                            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                                goto startGame; // Jump to startGame label
                            }
                        }

                        if (pulseClock.getElapsedTime().asMilliseconds() > 10) {
                            alpha += (increasing ? 5 : -5);
                            if (alpha > 255) {
                                alpha = 255;
                                increasing = false;
                            }
                            else if (alpha < 0) {
                                alpha = 0;
                                increasing = true;
                            }
                            pulseClock.restart();
                        }

                        startText.setFillColor(sf::Color(255, 255, 0, static_cast<sf::Uint8>(alpha)));
                        window.clear();
                        window.draw(storylineBackgroundSprite);
                        window.draw(storylineText);
                        window.draw(startText);
                        window.display();
                    }

                    storylineDisplayed = true;
                }

            startGame:
                std::string unlockedLetters = startBattle(); // Start the battle and get unlocked letters
                if (!unlockedLetters.empty()) {
                    // Display the unlocked letters
                    sf::Text unlockedText("Congratulations! You have unlocked the following letters: " + unlockedLetters, font, 30);
                    unlockedText.setFillColor(sf::Color::White);
                    centerText(unlockedText, window, window.getSize().y / 2 + 100);
                    sf::Clock displayClock;
                    bool lettersDisplayed = true;

                    while (lettersDisplayed) {
                        sf::Event event;
                        while (window.pollEvent(event)) {
                            if (event.type == sf::Event::Closed) {
                                closeConsole();
                                window.close();
                                lettersDisplayed = false;
                            }
                        }

              
                        if (displayClock.getElapsedTime().asSeconds() >= 5) {
                            lettersDisplayed = false;
                        }

                        window.clear();
                        window.draw(unlockedText);
                        window.display();
                    }
                    gamesCompleted++;
                }
                std::this_thread::sleep_for(std::chrono::seconds(3));
                SnakeGame game;
                game.run();
                gamesCompleted++;
               
                sf::RenderWindow window(sf::VideoMode(1200, 1000), "Riddle Game");

               
                sf::Font billoFont;
                if (!billoFont.loadFromFile("D:\SFML\\BILLO___.TTF")) {
                    cerr << "Error loading font!" << endl;
                    return -1;
                }

                sf::Font augustaFont;
                if (!augustaFont.loadFromFile("D:\SFML\\The Augusta.otf")) {
                    cerr << "Error loading font!" << endl;
                    return -1;
                }

  
                RiddleGame game2;
                sf::Text text;
                game2.playGame(window, text, augustaFont, billoFont);
                gamesCompleted++;
                if (gamesCompleted >= 3) {
                    std::string password;
                    sf::Text messageText;
                    messageText.setFont(font); // Use your loaded font
                    messageText.setCharacterSize(30);
                    messageText.setFillColor(sf::Color::White);
                    messageText.setPosition(100, 300); // Position the message

                    // Prepare the message
                    std::string message = "You have collected all letters!! Now arrange them to unlock the treasure. Letters are uatEeval";
                    messageText.setString(message);

                    // Create a text object for user input
                    sf::Text inputText;
                    inputText.setFont(font);
                    inputText.setCharacterSize(30);
                    inputText.setFillColor(sf::Color::White);
                    inputText.setPosition(100, 350); // Position the input text below the message

                    // Display the message
                    window.clear();
                    window.draw(messageText);
                    window.draw(inputText);
                    window.display();

                    // Input loop
                    bool waitingForInput = true;
                    while (waitingForInput) {
                        sf::Event event;
                        while (window.pollEvent(event)) {
                            if (event.type == sf::Event::Closed) {
                                window.close();
                            }

                            if (event.type == sf::Event::TextEntered) {
                                // Handle backspace
                                if (event.text.unicode == '\b' && !password.empty()) {
                                    password.pop_back();
                                }
                                
                                else if (event.text.unicode < 128) { 
                                    password += static_cast<char>(event.text.unicode);
                                }
                            
                                inputText.setString(password);
                            }

                         
                            if (event.type == sf::Event::KeyPressed) {
                                if (event.key.code == sf::Keyboard::Enter) {
                                    waitingForInput = false; 
                                }
                            }
                        }

                        window.clear();
                        window.draw(messageText);
                        window.draw(inputText);
                        window.display();
                    }

                   
                    if (password == "Evaluate") {
                        std::cout << "You have successfully unlocked the treasure (gold coins)." << std::endl;
                    }
                    else {
                        std::cout << "Incorrect password! You have failed to open the treasure." << std::endl;
                    }

                    
                    std::this_thread::sleep_for(std::chrono::seconds(10)); 
                }
            }
            window.clear();
        }


            if (gameState == GameState::SplashScreen) {
                window.draw(backgroundSprite);
                window.draw(subtitle);
            }
            else if (gameState == GameState::Menu) {
                window.draw(menuBackgroundSprite);
                for (const auto& item : menuItems) {
                    window.draw(item);
                }
            }
            else if (gameState == GameState::Settings) {
                window.draw(menuBackgroundSprite);
                for (const auto& option : settingsOptions) {
                    window.draw(option);
                }
                window.draw(backToMenu);
            }

            window.draw(brightnessOverlay);
            window.display();
        }

        return 0;
    }
  

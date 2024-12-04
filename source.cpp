#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

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

// Function to recreate window when changing fullscreen or resolution
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

// Function to center text within the window
void centerText(sf::Text& text, const sf::RenderWindow& window, float yOffset) {
    text.setPosition(window.getSize().x / 2 - text.getGlobalBounds().width / 2, yOffset);
}

int main() {
    // Get desktop resolution
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktopMode, "Fantasy Adventure Game", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("D:/SFML/BILLO___.TTF")) {
        return -1;
    }

    // Load "Augusta.ot" font for game name
    sf::Font gameNameFont;
    if (!gameNameFont.loadFromFile("D:/SFML/The Augusta.otf")) {
        return -1;
    }

    // Load background music
    sf::Music music;
    if (!music.openFromFile("C:/Users/HP Probook 640/Downloads/mixkit-medieval-show-fanfare-announcement-226.wav")) {
        return -1;
    }
    music.setLoop(true);
    music.setVolume(50);
    music.play();

    // Subtitle text
    sf::Text subtitle("Press Any Key to Begin...", font, 60);
    subtitle.setFillColor(sf::Color::White);
    subtitle.setPosition(desktopMode.width / 2 - subtitle.getGlobalBounds().width / 2, desktopMode.height - 100);

    // Background stars
    srand(static_cast<unsigned>(time(0)));
    std::vector<Star> stars;
    for (int i = 0; i < 100; ++i) {
        float radius = static_cast<float>(rand() % 3 + 1);
        stars.emplace_back(rand() % desktopMode.width, rand() % desktopMode.height, radius, sf::Vector2f((rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f));
    }

    // Load background texture for splash screen
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("C:/Users/HP Probook 640/Downloads/animation.png")) {
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(desktopMode.width) / static_cast<float>(backgroundTexture.getSize().x),
        static_cast<float>(desktopMode.height) / static_cast<float>(backgroundTexture.getSize().y)
    );

    // Load background texture for menu
    sf::Texture menuBackgroundTexture;
    if (!menuBackgroundTexture.loadFromFile("C:/Users/HP Probook 640/Downloads/bg texture.jpeg")) {
        return -1;
    }
    sf::Sprite menuBackgroundSprite(menuBackgroundTexture);
    menuBackgroundSprite.setScale(
        static_cast<float>(desktopMode.width) / static_cast<float>(menuBackgroundTexture.getSize().x),
        static_cast<float>(desktopMode.height) / static_cast<float>(menuBackgroundTexture.getSize().y)
    );

    // Game states
    enum class GameState { SplashScreen, Menu, Play, Settings, Quit, FullscreenSettings, BrightnessSettings, ResolutionSettings, VolumeSettings };
    GameState gameState = GameState::SplashScreen;

    // Menu items
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

    // Labels for settings
    std::string settingsLabels[] = { "Fullscreen: ", "Resolution: ", "Brightness: ", "Volume: " };
    sf::Text settingsOptions[4];
    int settingsIndex = 0;

    // Dynamically adjust Y offset to avoid overlapping
    float settingYOffset = static_cast<float>(desktopMode.height) / 2 - (4 * 100) / 2;

    // Initialize settings options text
    for (int i = 0; i < 4; ++i) {
        settingsOptions[i].setFont(font);
        settingsOptions[i].setCharacterSize(70);
        settingsOptions[i].setFillColor(i == settingsIndex ? sf::Color::Yellow : sf::Color::White);
        centerText(settingsOptions[i], window, settingYOffset + i * 120);  // Adjusted vertical spacing
    }

    sf::Text backToMenu("Press ESC to Go Back", font, 50);
    backToMenu.setFillColor(sf::Color::White);
    backToMenu.setPosition(static_cast<float>(desktopMode.width) / 2 - backToMenu.getGlobalBounds().width / 2, static_cast<float>(desktopMode.height) - 150.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (gameState == GameState::SplashScreen && event.type == sf::Event::KeyPressed) {
                gameState = GameState::Menu; // Move to menu screen
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
                        if (selectedIndex == 0) { // Play
                            gameState = GameState::Play;
                        }
                        else if (selectedIndex == 1) { // Settings
                            gameState = GameState::Settings;
                        }
                        else if (selectedIndex == 2) { // Quit
                            window.close();
                        }
                    }

                    // Update menu item colors
                    for (int i = 0; i < 3; ++i) {
                        menuItems[i].setFillColor(i == selectedIndex ? sf::Color::Yellow : sf::Color::White);
                    }
                }
            }
            else if (gameState == GameState::Settings) {
                // Handle settings menu navigation and updates
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        settingsIndex = (settingsIndex - 1 + 4) % 4;
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        settingsIndex = (settingsIndex + 1) % 4;
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        // Handle each setting change based on the index
                        switch (settingsIndex) {
                        case 0: // Fullscreen toggle
                            isFullscreen = !isFullscreen;
                            recreateWindow(window, isFullscreen);
                            break;
                        case 1: // Resolution change
                            selectedResolutionIndex = (selectedResolutionIndex + 1) % availableResolutions.size();
                            window.create(availableResolutions[selectedResolutionIndex], "Fantasy Adventure Game", sf::Style::Fullscreen);
                            break;
                        case 2: // Brightness change
                            brightnessLevel = (brightnessLevel + 10) % 110; // Loop back after 100
                            break;
                        case 3: // Volume change
                            volumeLevel = (volumeLevel + 10) % 110;
                            music.setVolume(volumeLevel);
                            break;
                        }
                    }

                    // Update color of selected option
                    for (int i = 0; i < 4; ++i) {
                        settingsOptions[i].setFillColor(i == settingsIndex ? sf::Color::Yellow : sf::Color::White);
                    }
                }

                // Dynamically update the text for the settings
                settingsOptions[0].setString("Fullscreen: " + std::string(isFullscreen ? "On" : "Off"));
                settingsOptions[1].setString("Resolution: " + std::to_string(availableResolutions[selectedResolutionIndex].width) + "x" + std::to_string(availableResolutions[selectedResolutionIndex].height));
                settingsOptions[2].setString("Brightness: " + std::to_string(brightnessLevel) + "%");
                settingsOptions[3].setString("Volume: " + std::to_string(volumeLevel) + "%");
            }
        }

        // Clear screen
        window.clear();

        if (gameState == GameState::SplashScreen) {
            window.draw(backgroundSprite); // Splash screen background
            window.draw(subtitle);
        }
        else if (gameState == GameState::Menu) {
            window.draw(menuBackgroundSprite); // Menu background
            window.draw(gameName);
            for (int i = 0; i < 3; ++i) {
                window.draw(menuItems[i]);
            }
        }
        else if (gameState == GameState::Settings) {
            window.draw(menuBackgroundSprite); // Settings background
            for (int i = 0; i < 4; ++i) {
                window.draw(settingsOptions[i]);
            }
            window.draw(backToMenu); // Back to menu text
        }

        window.display();
    }

    return 0;
}

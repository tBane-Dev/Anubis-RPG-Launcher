// for std::wcout 
#include <io.h>      // _setmode, _fileno
#include <fcntl.h>   // _O_U16TEXT
#include <stdio.h>   // stdout

#include "SFML/Window.hpp"
#include <SFML/Graphics.hpp>

#include "RenderWindow.hpp"
#include "DebugLog.hpp"
#include "Time.hpp"
#include "Intro.hpp"
#include "Theme.hpp"
#include "3D/OpenGLBuffers.hpp"
#include "Loader.hpp"
#include "TexturesManager.hpp"
#include "ModelsManager.hpp"
#include "ShadersManager.hpp"
#include "PrefabsManager.hpp"
#include "GUIManager.hpp"
#include "Cursor.hpp"
#include "ModulesManager.hpp"
#include "Modules/Game/Module.hpp"
#include "SoundsManager.hpp"

#include <windows.h>
#include <commdlg.h>
#include <iostream>

static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const char* message, const void* userParam) {
    constexpr GLenum debugTypeError = 0x824C;   // GL_DEBUG_TYPE_ERROR
    if (type != debugTypeError)
        return;                                 // tylko błędy, bez ostrzeżeń o wydajności

    std::string text(message, length > 0 ? std::size_t(length) : std::strlen(message));
    DebugError(L"GL DEBUG: " + std::wstring(text.begin(), text.end()));
}

int main() {
    (void)_setmode(_fileno(stdout), _O_U16TEXT); // wide char UTF-16 output

    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Debug;

    Anubis::render_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(sf::Vector2u(1920, 1080), 32u), "Anubis-RPG-Launcher", sf::State::Windowed, settings);
    
    ShowWindow(Anubis::render_window->getNativeHandle(), SW_MAXIMIZE);

    Anubis::render_window->setVerticalSyncEnabled(false);
    Anubis::render_window->setFramerateLimit(0);

    auto* version = glGetString(GL_VERSION);
    auto* renderer = glGetString(GL_RENDERER);

    if (version) {
        std::string text(reinterpret_cast<const char*>(version));
        DebugStat(L"OpenGL: " + std::wstring(text.begin(), text.end()));
    }

    if (renderer) {
        std::string text(reinterpret_cast<const char*>(renderer));
        DebugStat(L"Renderer: " + std::wstring(text.begin(), text.end()));
    }

    DebugStat(L"");

    // OpenGL Buffers initialize :-)
    if (!Anubis::GL::initialize())
        return EXIT_FAILURE;


    using DebugProc = void (APIENTRY*)(GLenum, GLenum, GLuint, GLenum, GLsizei, const char*, const void*);
    using DebugMessageCallbackFunction = void (APIENTRY*)(DebugProc, const void*);

    auto debugMessageCallback = reinterpret_cast<DebugMessageCallbackFunction>(
        sf::Context::getFunction("glDebugMessageCallback"));

    if (debugMessageCallback) {
        constexpr GLenum debugOutput = 0x92E0;             // GL_DEBUG_OUTPUT
        constexpr GLenum debugOutputSynchronous = 0x8242;  // GL_DEBUG_OUTPUT_SYNCHRONOUS
        glEnable(debugOutput);
        glEnable(debugOutputSynchronous);
        debugMessageCallback(glDebugCallback, nullptr);
        DebugStat(L"OpenGL debug output enabled");
    }
    else {
        DebugError(L"glDebugMessageCallback is unavailable");
    }

    Anubis::render_window->clear(sf::Color::Black);
    Anubis::render_window->display();

    while (const std::optional event = Anubis::render_window->waitEvent()) {

        if (event->is<sf::Event::Closed>()) {
            Anubis::render_window->close();
            exit(0);
        }

        if (const auto* kp = event->getIf<sf::Event::KeyPressed>(); kp && kp->code == sf::Keyboard::Key::Space) {
            break;
        }
    }

    loadTheme(); // TO-DO - must be a shared_ptr
    
    textures_manager = std::make_shared<TexturesManager>();
    Anubis::shaders_manager = std::make_shared<Anubis::ShadersManager>();
    Anubis::models_manager = std::make_shared<Anubis::ModelsManager>();
    Anubis::prefabs_manager = std::make_shared<Anubis::PrefabsManager>();

    Cursors::cursor = std::make_shared<Cursors::Cursor>();
    Anubis::GUI_manager = std::make_shared<Anubis::GUIManager>();
    Main::modules_manager = std::make_shared<Main::ModulesManager>();
    Anubis::sounds_manager = std::make_shared<Anubis::SoundsManager>();

    Anubis::renderIntro();

    Game::module = std::make_shared<Game::Module>();
    Main::modules_manager->push_back(Game::module);

    Game::module->createScene();
    Game::module->createMap(5, 3);
    Game::module->createGameObjects();
    Game::module->createCamera();
    Game::module->createCursorOnMap();
    Game::module->createPlayer();

    // cursor
    Anubis::cursor = std::make_shared<Anubis::Cursor>();
    Anubis::cursor->setScene(Game::module->_scene);

    Game::module->setVisibleChunks();

    // load the test-map
    const std::filesystem::path path = L"world.anubis";
    if (std::filesystem::exists(path)) {
        try {
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                DebugError(L"Cant Open File: " + path.wstring());
            } 
            else {
                file.exceptions(std::ios::failbit | std::ios::badbit);

                std::shared_ptr<Anubis::Loader> loader = std::make_shared<Anubis::Loader>(file);
                Anubis::models_manager->loadFromProject(file, loader);
                Anubis::prefabs_manager->loadFromProject(file, loader);
                Anubis::sounds_manager->clear();
                Game::module->_map->loadFromProject(file, loader);
                Game::module->_game_objects->loadFromProject(file, loader);
                loader->end();
                Game::module->createPlayer();
                Game::module->setVisibleChunks();
            }

            
        }
        catch (const std::exception& e) {
            std::string msg = e.what();
            DebugError(L"Incorrect data: " + std::wstring(msg.begin(), msg.end()));
        }
    }

    // Game::module->setVisibleChunks();

    // init FPS clock
    sf::Clock FPSClock;
    int frames = 0;
    float elapsed = 0.0f;

    while (Anubis::render_window->isOpen()) {

        Anubis::prevTime = Anubis::currentTime;
        Anubis::currentTime = Anubis::mainClock.getElapsedTime();
        Anubis::deltaTime = Anubis::currentTime - Anubis::prevTime;

        if(Anubis::deltaTime.asSeconds() > 0.1f) {
            Anubis::deltaTime = sf::seconds(0.1f);
		}

        Cursors::cursor->update();

        // FPS TIMER
        float dtFPS = FPSClock.restart().asSeconds();
        frames++;
        elapsed += dtFPS;

        if (elapsed >= 0.5f) {
            float FPS = frames / elapsed;
            float ms = 1000.0f / FPS;

            std::ostringstream ss;
            ss << "Anubis-RPG-Launcher - " << std::fixed << std::setprecision(2) << FPS << " FPS / " << std::setprecision(3) << ms << " ms";
            Anubis::render_window->setTitle(ss.str());
            frames = 0;
            elapsed = 0.0f;
        }

        Anubis::GUI_manager->Element_hovered = nullptr;
        Main::modules_manager->cursorHover();

        // handle events
        while (const std::optional event = Anubis::render_window->pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                Anubis::render_window->close();
                exit(0);
            }

            if(event->is<sf::Event::Resized>()) {
				sf::Vector2u newSize = event->getIf<sf::Event::Resized>()->size;
                Anubis::render_window->setSize(newSize);
                Anubis::GUI_manager->resize(sf::Vector2i(newSize));

			}

            Main::modules_manager->handleEvent(*event);

        }

        // update
        Main::modules_manager->update();
        Anubis::sounds_manager->update();

        // draw
        Anubis::render_window->clear(sf::Color(47, 47, 47));
        Main::modules_manager->draw();
        Anubis::render_window->display();
    }
}

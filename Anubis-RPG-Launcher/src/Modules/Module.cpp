#include "Modules/Game/Module.hpp"
#include "DebugLog.hpp"
#include "Objects/Monster.hpp"
#include "Objects/Nature.hpp"
#include "PrefabsManager.hpp"
#include <SFML/OpenGL.hpp>
#include "3D/Model3D.hpp"
#include "3D/Mesh3D.hpp"
#include "ShadersManager.hpp"
#include "3D/OpenGLBuffers.hpp"

namespace Game {

	Module::Module() {

	}

	Module::~Module() {

	}

	void Module::createScene() {

		_scene = std::make_shared<Anubis::Scene>();

		auto size = Anubis::render_window->getSize();
		if (!_scene->createPickingBuffer(size.x, size.y)) {
			DebugError(L"Failed to create Picking Buffer");
			exit(0);
		}

		if (!_scene->initializeModel3DShaders()) {
			DebugError(L"Failed to Initialize Model3D Shaders");
			exit(0);
		}
	}

	void Module::createMap(int width, int height) {

		_map = std::make_shared<Map>();

		_map->create(width, height);
		
	}

	void Module::createGameObjects() {
		_game_objects = std::make_shared<PlacedGameObjects>();
	}

	void Module::createCamera() {

		_camera = std::make_shared<CameraOnMap>();

		_camera->_position = sf::Vector2f((float)(_map->getRect().size.x / 2), (float)(_map->getRect().size.y / 2));
		_camera->update();
	}

	void Module::createCursorOnMap() {
		_cursor_on_map = std::make_shared<CursorOnMap>();
	}
	
	void Module::createPlayer() {
		_player = std::make_shared<Player>(Anubis::prefabs_manager->getPrefab(L"dziobak"));
		_player->setPosition(sf::Vector2f(0, 0));
		_game_objects->addGameObject(_player);
	}

	void Module::setVisibleChunks() {
		_map->setVisibleChunks();

		if (_player) {
			_game_objects->addGameObject(_player);
		}
			
	
		auto& visibleObjects = Game::module->_game_objects->_visiblePlacedGameObjects;

		for (const auto& selectedObject : Game::module->_cursor_on_map->_selectedObjects) {

			auto object = selectedObject->_object;
			if (!object)
				continue;

			if (std::find(visibleObjects.begin(), visibleObjects.end(), object) == visibleObjects.end()) {
				visibleObjects.emplace_back(selectedObject->_object);
			}
		}

		// Remove invisible models from scene
		std::erase_if(_scene->_models,
			[&](const std::shared_ptr<Anubis::Model3D>& model)
			{
				return std::find_if(visibleObjects.begin(), visibleObjects.end(),
					[&](const std::shared_ptr<PlacedGameObject>& object)
					{
						return object && object->_model == model;
					}) == visibleObjects.end();
			});
	}

	void Module::cursorHover() {

		Main::Module::cursorHover();

		_map->cursorHover();
	}

	void Module::handleEvent(const sf::Event& event) {

		// Middle-button panning keeps the grabbed terrain point under the cursor (also for y != 0).
		if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>(); mbp && mbp->button == sf::Mouse::Button::Middle) {
			_scene->_panPlaneY = _cursor_on_map->_isOnTerrain ? _cursor_on_map->_worldPosition.y : 0.f;
		}

		_camera->handleEvent(event);
		_scene->handleEvent(event);

		_map->handleEvent(event);
		_cursor_on_map->handleEvent(event);
	}

	void Module::update() {

		_camera->update();

		_cursor_on_map->update();

		_map->update();
		_game_objects->update();

		_scene->update();
	}

	void Module::draw() {

		_camera->setView();

		Anubis::render_window->resetGLStates();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		_scene->setViewport();
		_scene->setProjection();
		_scene->setCamera();

		_scene->renderShadowMap();
		_map->draw();

		// SFML in Map::draw() changed the states - the depth of the terrain stays (hills hide the models)
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);

		_scene->setViewport();
		_scene->setProjection();
		_scene->setCamera();

		_scene->draw();

		// waters at the end - transparent, they use the depth of the terrain and of the models
		_scene->setViewport();
		_scene->setProjection();
		_scene->setCamera();

		GLfloat viewMatrix[16];
		GLfloat projectionMatrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
		glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

		_map->drawWaters(sf::Glsl::Mat4(viewMatrix), sf::Glsl::Mat4(projectionMatrix));
		Anubis::GL::bindVertexArray(0);

		Anubis::render_window->resetGLStates();

		_cursor_on_map->draw();

		Anubis::GUI_manager->setView();
	}

	std::shared_ptr<Module> module = nullptr;
}
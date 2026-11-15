#include "Modules/Game/Map/CursorOnMap.hpp"
#include "Cursor.hpp"
#include "Modules/Game/Map/CameraOnMap.hpp"
#include "Modules/Game/Map/Map.hpp"
#include "ModulesManager.hpp"
#include "Modules/Game/Module.hpp"
#include "Objects/GameObject.hpp"
#include "Objects/Terrain.hpp"
#include "Objects/Monster.hpp"
#include "Objects/Nature.hpp"
#include "PrefabsManager.hpp"
#include "Modules/Game/Map/PlacedGameObjects.hpp"
#include <typeinfo>
#include "DebugLog.hpp"
#include <set>
#include <cmath>
#include <algorithm>
#include <SFML/OpenGL.hpp>
#include "3D/OpenGLBuffers.hpp"

namespace Game {

	CursorOnMap::CursorOnMap() : CursorWithObject() {

		_selectedObjects.clear();

		_prevPosition = sf::Vector2i(-1, -1);

		_pickClock = sf::Clock();
		_pickInterval = 100;

		_worldPosition = sf::Vector3f(0.0f, 0.0f, 0.0f);
		_isOnTerrain = false;
	}

	CursorOnMap::~CursorOnMap() {

	}

	void CursorOnMap::addToSelected(std::shared_ptr<PlacedGameObject> object, sf::Vector2i offset) {

		if (std::find_if(_selectedObjects.begin(), _selectedObjects.end(),
			[&](std::shared_ptr<SelectedPlacedGameObject>& selectedObject) {
				return selectedObject->_object == object;
			}
		) == _selectedObjects.end()) {
			std::shared_ptr<SelectedPlacedGameObject> selectedObject = std::make_shared<SelectedPlacedGameObject>(object, sf::Vector2i(object->getPosition()), offset);
			selectedObject->_object->_isSelected = true;
			_selectedObjects.push_back(selectedObject);
		}
	}

	void CursorOnMap::removeFromSelected(std::shared_ptr<GameObject> object) {
		std::erase_if(_selectedObjects, [&](std::shared_ptr<SelectedPlacedGameObject>& obj) {
			bool condition = obj->_object->_prefab.lock() == object;
			if (condition)
				obj->_object->_isSelected = false;
			return condition;
			});
	}

	std::shared_ptr<PlacedGameObject> CursorOnMap::getObjectUnderCursor() {

		std::vector<std::shared_ptr<PlacedGameObject>>& objects = Game::module->_game_objects->_visiblePlacedGameObjects;

		// Only 3D models are selectable. Picking returns the model nearest to the camera.
		int id = Game::module->_scene->getHoveredModelId(_position);

		if (id >= 0) {
			for (auto& object : objects) {
				if (object && object->_model && object->_model->_id == id)
					return object;
			}
		}

		return nullptr;
	}


	void CursorOnMap::syncSceneSelection() {

		std::shared_ptr<Anubis::Scene> scene = Game::module->_scene;

		// Selected objects are highlighted by Model3D::draw() through Scene::_selected.
		scene->_selected.clear();

		for (auto& selected : _selectedObjects) {
			if (selected && selected->_object && selected->_object->_model)
				scene->_selected.push_back(selected->_object->_model);
		}

		scene->_hovered.clear();

		if (Anubis::GUI_manager->Element_hovered != Game::module->_map)
			return;

		int id = scene->getHoveredModelId(_position);

		if (id < 0)
			return;

		for (auto& model : scene->_models) {
			if (model && model->_id == id) {
				scene->_hovered.push_back(model);
				break;
			}
		}
	}

	void CursorOnMap::update() {
		Cursor::update();

		if (_pickClock.getElapsedTime() > sf::milliseconds(_pickInterval))
			_isOnTerrain = Game::module->_map->pickTerrain(_position, _worldPosition);


		if (_isOnTerrain) {
			_globalPosition = sf::Vector2i(
				(int)std::floor(_worldPosition.x),
				(int)std::floor(_worldPosition.z)
			);
		}

		if (_pickClock.getElapsedTime() > sf::milliseconds(_pickInterval)) {
			syncSceneSelection();
			_pickClock.restart();
		}
	}

	void CursorOnMap::handleEvent(const sf::Event& event) {
		if (Anubis::GUI_manager->Element_pressed == Game::module->_map) {

			if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>(); mbp && mbp->button == sf::Mouse::Button::Left) {

				for (auto& object : _selectedObjects) {
					if (object->_object)
						object->_object->_isSelected = false;
				}

				_selectedObjects.clear();

				std::shared_ptr<PlacedGameObject> selectedGameObject = getObjectUnderCursor();

				if (selectedGameObject) {
					selectedGameObject->_isSelected = true;
					_selectedObjects.push_back(std::make_shared<SelectedPlacedGameObject>(selectedGameObject, sf::Vector2i(selectedGameObject->getPosition()), _globalPosition - sf::Vector2i(selectedGameObject->getPosition())));
				}

				syncSceneSelection();
			}
		}
	}

	sf::Vector2i CursorOnMap::getCursorTile() {
		return sf::Vector2i(
			(int)std::floor(_worldPosition.x / (float)Tile::tileSize),
			(int)std::floor(_worldPosition.z / (float)Tile::tileSize)
		);
	}

	void CursorOnMap::draw() {
	
	}
}

#pragma once
#include <SFML/Graphics.hpp>
#include "Objects/Object.hpp"
#include "Cursor.hpp"
#include "Objects/SelectedPlacedGameObject.hpp"
#include <map>
#include <set>

namespace Game {
	class Chunk;

	class CursorOnMap : public Cursors::CursorWithObject {
	public:
		std::vector<std::shared_ptr<SelectedPlacedGameObject>> _selectedObjects;
		sf::Vector2i _prevPosition = sf::Vector2i(-1, -1);
		sf::Clock _pickClock;				// limits how often hard functions are updated
		float _pickInterval = 100;			// 100 ms
		sf::Vector3f _worldPosition;		// point of the terrain under the cursor (any height)
		bool _isOnTerrain = false;			// false when the cursor does not hit the terrain

		CursorOnMap();
		~CursorOnMap();

		void addToSelected(std::shared_ptr<PlacedGameObject> object, sf::Vector2i offset);
		void removeFromSelected(std::shared_ptr<GameObject> object);

		sf::Vector2i getCursorTile();
		sf::Vector2i getPlacementPosition(std::shared_ptr<GameObject> prefab);

		std::shared_ptr<PlacedGameObject> getObjectUnderCursor();
		void syncSceneSelection();

		virtual void update();
		virtual void handleEvent(const sf::Event& event);
		virtual void draw();
	};

}
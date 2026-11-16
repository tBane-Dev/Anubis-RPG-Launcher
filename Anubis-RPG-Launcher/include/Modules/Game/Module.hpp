#pragma once
#include "ModulesManager.hpp"
#include "3D/Scene.hpp"
#include "Modules/Game/Map/Map.hpp"
#include "Modules/Game/Map/CameraOnMap.hpp"
#include "Modules/Game/Map/CursorOnMap.hpp"
#include "Modules/Game/Map/PlacedGameObjects.hpp"


namespace Game {
	class Module : public Main::Module {
	public:

		std::shared_ptr<Anubis::Scene> _scene; // only to render models3D
		std::shared_ptr<Map> _map;
		std::shared_ptr<PlacedGameObjects> _game_objects;
		std::shared_ptr<CameraOnMap> _camera;
		std::shared_ptr<CursorOnMap> _cursor_on_map;

		Module();
		~Module();

		void createScene();
		void createMap(int width, int height);
		void createGameObjects();
		void createCamera();
		void createCursorOnMap();
		void setVisibleChunks();

		void cursorHover();
		void handleEvent(const sf::Event& event);
		void update();
		void draw();

	};

	extern std::shared_ptr<Module> module;
}
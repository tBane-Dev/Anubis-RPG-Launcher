#pragma once
#include <SFML/Graphics.hpp>
#include "GUIManager.hpp"
#include "Modules/Game/Map/Chunk.hpp"
#include "Modules/Game/Map/Water.hpp"
#include "Loader.hpp"
#include <fstream>
#include "SoundsManager.hpp"

namespace Game {
	class Map : public Anubis::Element {
	public:

		std::shared_ptr<Texture> _texture;			// tileset.png - 8 terrains in one column
		std::vector<std::shared_ptr<Chunk>> _chunks;
		std::vector<std::shared_ptr<Water>> _waters;
		std::vector<std::shared_ptr<Chunk>> _visibleChunks;

		std::shared_ptr<sf::Sound> _oceanAmbience;					// loop sound of the ocean, stands on the nearest point of the ocean
		int _waveIndex;												// the next wave to play
		int _waveCycle;												// the cycle of the wave in the shader when the last wave was played
		sf::Time _waterSoundsTime;
		sf::Vector2f _oceanPoint;									// the nearest point of the ocean - the wave sound uses its phase

		Map();
		~Map();

		void addChunk(std::shared_ptr<Chunk> chunk, int terrain);
		void create(int width, int height);
		void addSounds();

		std::shared_ptr<Chunk> getChunkByCoords(int x, int y);
		std::shared_ptr<Chunk> getChunkByTileGlobalCoords(int x, int y);
		std::shared_ptr<Chunk> getChunkByGlobalPosition();
		std::shared_ptr<Chunk> getChunkByGlobalPosition(sf::Vector2i position);
		sf::IntRect getRect();

		// SMOOTH TERRAIN
		void stitchChunk(std::shared_ptr<Chunk> chunk);

		void setVisibleChunks();

		// Point of the terrain surface under a window pixel. Works for any terrain heights:
		// only the terrain is drawn into the scene's picking depth buffer and the depth is unprojected.
		bool pickTerrain(sf::Vector2i pixel, sf::Vector3f& hit);

		// Terrain height at a map point (0 outside the map).
		float getHeight(float x, float z);

		void drawWaters(sf::Glsl::Mat4 view, sf::Glsl::Mat4 projection);
		float getDistanceToWater(sf::Vector2f position, int kind, sf::Vector2f& nearest);

		void saveFromProject(std::ofstream& file);
		void loadFromProject(std::ifstream& file, std::shared_ptr<Anubis::Loader>& loader);

		void cursorHover();
		void handleEvent(const sf::Event& event);
		void update();
		void draw();
	};

}

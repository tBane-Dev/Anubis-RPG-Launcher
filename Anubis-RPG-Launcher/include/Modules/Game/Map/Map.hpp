#pragma once
#include <SFML/Graphics.hpp>
#include "GUIManager.hpp"
#include "Modules/Game/Map/Chunk.hpp"
#include "Loader.hpp"
#include <fstream>

namespace Game {
	class Map : public Anubis::Element {
	public:

		std::shared_ptr<Texture> _texture;
		std::vector<std::shared_ptr<Chunk>> _chunks;
		std::vector<std::shared_ptr<Chunk>> _visibleChunks;

		Map();
		~Map();

		void addChunk(std::shared_ptr<Chunk> chunk, int tileType);
		void create(int width, int height);

		std::shared_ptr<Chunk> getChunkByCoords(int x, int y);
		std::shared_ptr<Chunk> getChunkByTileGlobalCoords(int x, int y);
		std::shared_ptr<Chunk> getChunkByGlobalPosition();
		std::shared_ptr<Chunk> getChunkByGlobalPosition(sf::Vector2i position);
		std::shared_ptr<Tile> getTileByTileGlobalCoords(int x, int y);
		sf::IntRect getRect();

		void setVisibleChunks();

		bool pickTerrain(sf::Vector2i pixel, sf::Vector3f& hit);
		void drawTileOverlay(sf::Vector2i firstTile, sf::Vector2i size, const std::vector<float>& mask, sf::Color color);

		float getHeight(float x, float z);

		void saveFromProject(std::ofstream& file);
		void loadFromProject(std::ifstream& file, std::shared_ptr<Anubis::Loader>& loader);

		void cursorHover();
		void handleEvent(const sf::Event& event);
		void update();
		void draw();
	};

	int getNeighbourTypeLocal(
		int nx, int ny,
		const std::shared_ptr<Chunk>& chunk,
		const std::shared_ptr<Chunk>& leftTopChunk,
		const std::shared_ptr<Chunk>& topChunk,
		const std::shared_ptr<Chunk>& rightTopChunk,
		const std::shared_ptr<Chunk>& leftChunk,
		const std::shared_ptr<Chunk>& rightChunk,
		const std::shared_ptr<Chunk>& leftBottomChunk,
		const std::shared_ptr<Chunk>& bottomChunk,
		const std::shared_ptr<Chunk>& rightBottomChunk
	);

	int getTileType(
		int type,
		int x, int y,
		std::shared_ptr<Chunk> chunk,
		std::shared_ptr<Chunk> leftTopChunk,
		std::shared_ptr<Chunk> topChunk,
		std::shared_ptr<Chunk> rightTopChunk,
		std::shared_ptr<Chunk> leftChunk,
		std::shared_ptr<Chunk> rightChunk,
		std::shared_ptr<Chunk> leftBottomChunk,
		std::shared_ptr<Chunk> bottomChunk,
		std::shared_ptr<Chunk> rightBottomChunk
	);

}

#pragma once
#include <SFML/Graphics.hpp>
#include "3D/Mesh3D.hpp"
#include <cstdint>

namespace Game {

	// Part of a water in one chunk. Masks have one value per weight point of the chunk (65 x 65, every 4 world units).
	class WaterChunk {
	public:
		sf::Vector2i _coords;					// coords of the chunk
		std::vector<std::uint8_t> _fill;		// from the Fill tool of the editor
		std::vector<std::uint8_t> _added;		// painted with the water brush
		std::vector<std::uint8_t> _removed;		// painted with the eraser
		std::vector<std::uint8_t> _shore;		// distance to the shore / 2 (0 - 255 = 0 - 510 world units), for the foam
		std::shared_ptr<sf::Texture> _maskTexture;
		std::shared_ptr<Anubis::Mesh3D> _mesh;

		WaterChunk(sf::Vector2i coords);
		~WaterChunk();

		std::uint8_t getMask(int point);
	};

	// Data of a water in the map file.
	class WaterState {
	public:
		int _kind = 0;
		float _level = 0.f;
		std::vector<sf::Vector2f> _seeds;
		std::vector<sf::Vector2i> _coords;
		std::vector<std::vector<std::uint8_t>> _fill;
		std::vector<std::vector<std::uint8_t>> _added;
		std::vector<std::vector<std::uint8_t>> _removed;
	};

	// Flat water surface (ocean, swamp, lava) made in Anubis RPG Studio - in the game it is only drawn.
	class Water {
	public:
		int _kind;					// 0 = ocean, 1 = swamp, 2 = lava
		float _level;				// height of the water surface
		std::vector<sf::Vector2f> _seeds;	// kept only to save the map again
		std::vector<std::shared_ptr<WaterChunk>> _chunks;

		Water(int kind, float level);
		~Water();

		std::shared_ptr<WaterChunk> getWaterChunk(sf::Vector2i coords);
		void updateShoreDistances();
		void updateChunk(std::shared_ptr<WaterChunk> waterChunk);
		void updateAllChunks();

		float getDistance(sf::Vector2f position, sf::Vector2f& nearest);

		WaterState getState();
		void setState(WaterState state);

		void draw();
	};

}

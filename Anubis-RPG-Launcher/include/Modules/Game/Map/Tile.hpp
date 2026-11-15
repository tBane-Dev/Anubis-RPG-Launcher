#pragma once
#include <SFML/Graphics.hpp>

namespace Game {
	class Tile {
	public:
		const static int tileSize = 16;
		sf::Vector2i _coords;
		sf::Vector2f _position;
		int _type;

		Tile();
		~Tile();
	};

	sf::Vector2i worldToTile(sf::Vector2i position);
}
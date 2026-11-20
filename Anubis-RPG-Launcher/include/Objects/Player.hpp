#pragma once
#include "Objects/PlacedGameObject.hpp"
#include "Modules/Game/Map/Path.hpp"

enum class PlayerState { Idle, Moving, Attacking };
enum class PlayerDirection { Down, Left, Right, Up };

class Player : public PlacedGameObject {
public:

	PlayerState _state;
	PlayerDirection _direction;

	std::shared_ptr<Game::Path> _path;

	Player(std::weak_ptr<GameObject> prefab);
	~Player();

	virtual void update();
};
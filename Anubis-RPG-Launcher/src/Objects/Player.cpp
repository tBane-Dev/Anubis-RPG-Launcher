#include "Objects/Player.hpp"
#include "Time.hpp"
#include "RenderWindow.hpp"
#include <iostream>
#include "Modules/Game/Module.hpp"
#include "DebugLog.hpp"
#include "ShadersManager.hpp"

Player::Player(std::weak_ptr<GameObject> prefab) : PlacedGameObject(prefab) {

	_type = ObjectType::Player;
	_state = PlayerState::Moving;

	_direction = PlayerDirection::Down;

	_path = std::make_shared<Game::Path>();
	_path->setStartPoint(sf::Vector2i(_position));
	_path->setEndPoint(sf::Vector2i(_position));
	_path->generatePath();
}

Player::~Player() {

}

void Player::update() {

	// update 
	_model->update();

	if (_state == PlayerState::Moving) {

		float interval = 0.125f;
		float moveSpeed = 32.0f;

		if (!_model->_animations.empty()) {
			float stepLength =  4.0f;
			float moveStep = 32.f / 2.0f / stepLength;
			moveSpeed = moveStep / interval * 0.5f;
			float duration = _model->_animations[0]._duration;

			_model->_player._index = 0;
			_model->_player._loop = true;
			_model->_player._speed = duration / (4.0f * interval) * 0.5f;
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
			_path->setStartPoint(sf::Vector2i(_position));
			sf::Vector2i endPoint = sf::Vector2i(sf::Mouse::getPosition());
			_path->setEndPoint(endPoint);
			_path->generatePath();
		}

		float distance = moveSpeed * Anubis::deltaTime.asSeconds();
		int guard = 0;
		
		while (distance > 0.0f && guard < 16 && !_path->isEmpty() && _position != sf::Vector2f(_path->getEndPoint())) {
			guard++;
		
			sf::Vector2f target = sf::Vector2f(_path->getStartPoint());
			sf::Vector2f diff = target - _position;
			float step = 0.0f;
		
			if (diff.y > 0.0f) {
				step = std::min(diff.y, distance);
				_position.y += step;
				_direction = PlayerDirection::Down;
			}
			else if (diff.y < 0.0f) {
				step = std::min(-diff.y, distance);
				_position.y -= step;
				_direction = PlayerDirection::Up;
			}
			else if (diff.x > 0.0f) {
				step = std::min(diff.x, distance);
				_position.x += step;
				_direction = PlayerDirection::Right;
			}
			else if (diff.x < 0.0f) {
				step = std::min(-diff.x, distance);
				_position.x -= step;
				_direction = PlayerDirection::Left;
			}
		
			distance -= step;
		
			if (_position == target)
				_path->nextPoint();
		}
		
		float angle = 0.0f;
		if (_direction == PlayerDirection::Down)  angle = 0.0f;
		if (_direction == PlayerDirection::Up)    angle = 180.0f;
		if (_direction == PlayerDirection::Left)  angle = -90.0f;
		if (_direction == PlayerDirection::Right) angle = 90.0f;
		float rad = angle * 3.14159265f / 180.0f;
		
		_model->_rotation = Anubis::Quaternion{
			std::cos(rad * 0.5f),
			0.0f,
			std::sin(rad * 0.5f),
			0.0f
		};
		
		_model->_position.x = _position.x;
		_model->_position.z = _position.y;
	}
}

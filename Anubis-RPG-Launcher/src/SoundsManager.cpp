#include "SoundsManager.hpp"
#include "DebugLog.hpp"
#include "Modules/Game/Module.hpp"
#include "RenderWindow.hpp"
#include <algorithm>

namespace Anubis {

	SoundBuffer::SoundBuffer(std::wstring name, std::filesystem::path path) {
		_name = name;
		if (!_buffer.loadFromFile(path))
			DebugError(L"SoundsManager: can't load sound: " + path.wstring());
	}

	SoundBuffer::~SoundBuffer() {

	}

	SoundsManager::SoundsManager() {
		_buffers.clear();
		_sounds.clear();
	}

	SoundsManager::~SoundsManager() {
		clear();
	}

	void SoundsManager::addBuffer(std::wstring name, std::filesystem::path path) {
		if (getBuffer(name) != nullptr)
			return;

		_buffers.push_back(std::make_shared<SoundBuffer>(name, path));
	}

	std::shared_ptr<SoundBuffer> SoundsManager::getBuffer(std::wstring name) {
		for (auto& buffer : _buffers) {
			if (buffer->_name == name)
				return buffer;
		}

		return nullptr;
	}

	std::shared_ptr<sf::Sound> SoundsManager::addSound(std::wstring name, sf::Vector3f position) {
		std::shared_ptr<SoundBuffer> buffer = getBuffer(name);
		if (buffer == nullptr) {
			DebugError(L"SoundsManager: no sound: " + name);
			return nullptr;
		}

		std::shared_ptr<sf::Sound> sound = std::make_shared<sf::Sound>(buffer->_buffer);
		sound->setSpatializationEnabled(false);
		sound->setPosition(position);
		sound->setVolume(0);
		sound->play();
		_sounds.push_back(sound);
		return sound;
	}

	void SoundsManager::clear() {
		for (auto& sound : _sounds)
			sound->stop();

		_sounds.clear();
		_buffers.clear();
	}

	void SoundsManager::update() {
		std::shared_ptr<Anubis::Scene> scene = Game::module->_scene;
		sf::Vector2f player = Game::module->_player->_position;
		sf::Vector2u windowSize = Anubis::render_window->getSize();

		// is the player on the screen
		sf::Vector3f playerPosition(player.x, Game::module->_map->getHeight(player.x, player.y), player.y);
		sf::Vector2f pixel;
		bool playerOnScreen = scene->worldToScreen(playerPosition, pixel) && pixel.x >= 0.f && pixel.y >= 0.f && pixel.x < (float)windowSize.x && pixel.y < (float)windowSize.y;

		for (auto& sound : _sounds) {
			sf::Vector3f position = sound->getPosition();
			float distance = (sf::Vector2f(position.x, position.z) - player).length();
			float v = scene->_cameraRadius / (scene->_cameraRadius + distance);
			float volume = 100.f * v;

			// the camera moved away from the player - the camera counts too, but the player gives min 80%
			if (!playerOnScreen) {
				float cameraDistance = (position - scene->_camera->_position).length();
				float cameraV = scene->_cameraRadius / (scene->_cameraRadius + cameraDistance);
				volume = std::clamp(100.f * cameraV, volume * 0.8f, volume);
			}

			sound->setVolume(volume);
		}

		for (int i = (int)_sounds.size() - 1; i >= 0; i--) {
			if (_sounds[i]->getStatus() == sf::Sound::Status::Stopped)
				_sounds.erase(_sounds.begin() + i);
		}
	}

	std::shared_ptr<SoundsManager> sounds_manager = nullptr;

}

#include "ShadersManager.hpp"

namespace Anubis {
	ShadersManager::ShadersManager() {
		terrain3D_shader = std::make_shared<sf::Shader>(L"assets\\shr\\terrain3D.vert", L"assets\\shr\\terrain3D.frag");
	}

	ShadersManager::~ShadersManager() {

	}

	std::shared_ptr<ShadersManager> shaders_manager = nullptr;
}
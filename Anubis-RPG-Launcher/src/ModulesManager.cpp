#include "ModulesManager.hpp"
#include <iostream>
#include "Cursor.hpp"

namespace Main {

	Module::Module() {

	}

	Module::~Module() {

	}

	void Module::create() {

	}

	void Module::cursorHover() {
		Anubis::cursor->update();
	}

	void Module::handleEvent(const sf::Event& event) {

	}

	void Module::update() {

	}

	void Module::draw() {

	}

	ModulesManager::ModulesManager() {
		_modules.clear();
	}

	ModulesManager::~ModulesManager() {
		_modules.clear();
	}

	void ModulesManager::push_back(std::shared_ptr<Module> editor) {
		_modules.push_back(editor);
	}

	void ModulesManager::pop_back() {
		_modules.pop_back();
	}

	std::shared_ptr<Module> ModulesManager::get_back() {
		return _modules.back();
	}

	void ModulesManager::cursorHover() {
		if (get_back())
			get_back()->cursorHover();
	}

	void ModulesManager::handleEvent(const sf::Event& event) {
		if (get_back())
			get_back()->handleEvent(event);
	}

	void ModulesManager::update() {
		if (get_back())
			get_back()->update();
	}

	void ModulesManager::draw() {
		if (get_back())
			get_back()->draw();
	}

	std::shared_ptr<ModulesManager> modules_manager = nullptr;
}
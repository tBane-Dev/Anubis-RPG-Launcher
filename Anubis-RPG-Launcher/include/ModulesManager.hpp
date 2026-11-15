#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

namespace Main {
	class Module {
	public:

		Module ();
		~Module();

		virtual void create();
		virtual void cursorHover();
		virtual void handleEvent(const sf::Event& event);
		virtual void update();
		virtual void draw();
	};

	class ModulesManager {
	public:
		std::vector<std::shared_ptr<Module>> _modules;

		ModulesManager();
		~ModulesManager();

		void push_back(std::shared_ptr<Module> module);
		void pop_back();
		std::shared_ptr<Module> get_back();

		void cursorHover();
		void handleEvent(const sf::Event& event);
		void update();
		void draw();
	};

	extern std::shared_ptr<ModulesManager> modules_manager;
}


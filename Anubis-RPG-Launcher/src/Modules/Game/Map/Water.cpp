#include "Modules/Game/Map/Water.hpp"
#include "Modules/Game/Map/Chunk.hpp"
#include "Modules/Game/Module.hpp"
#include "DebugLog.hpp"
#include "ShadersManager.hpp"
#include <SFML/OpenGL.hpp>
#include "3D/OpenGLBuffers.hpp"
#include <algorithm>

namespace Game {

	// WATER CHUNK //////////////////////////////////////////////////////////////////////////////////////////////

	WaterChunk::WaterChunk(sf::Vector2i coords) {
		_coords = coords;
		_fill.assign(Chunk::weightsCols * Chunk::weightsCols, 0);
		_added.assign(Chunk::weightsCols * Chunk::weightsCols, 0);
		_removed.assign(Chunk::weightsCols * Chunk::weightsCols, 0);
		_maskTexture = nullptr;
		_mesh = nullptr;
	}

	WaterChunk::~WaterChunk() {

	}

	// water = (fill or brush) without the eraser
	std::uint8_t WaterChunk::getMask(int point) {
		int value = std::max(_fill[point], _added[point]);
		return (std::uint8_t)(value * (255 - _removed[point]) / 255);
	}

	// WATER ////////////////////////////////////////////////////////////////////////////////////////////////////

	Water::Water(int kind, float level) {
		_kind = kind;
		_level = level;
	}

	Water::~Water() {

	}

	// mask texture + flat mesh of one chunk. Vertex y = terrain height (the shader puts the vertex on the water level
	// and uses the difference as the depth). Only quads with water get triangles.
	void Water::updateChunk(std::shared_ptr<WaterChunk> waterChunk) {

		std::shared_ptr<Chunk> chunk = Game::module->_map->getChunkByCoords(waterChunk->_coords.x, waterChunk->_coords.y);

		if (!chunk)
			return;

		int wc = Chunk::weightsCols;
		int hc = Chunk::heightsCols;

		// MASK TEXTURE
		std::vector<std::uint8_t> pixels(wc * wc * 4, 0);

		for (int i = 0; i < wc * wc; i++) {
			std::uint8_t value = waterChunk->getMask(i);
			pixels[i * 4 + 0] = value;
			pixels[i * 4 + 1] = value;
			pixels[i * 4 + 2] = value;
			pixels[i * 4 + 3] = 255;
		}

		if (!waterChunk->_maskTexture) {
			std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();

			if (!texture->resize(sf::Vector2u((unsigned int)wc, (unsigned int)wc))) {
				DebugError(L"Cannot create a water mask texture");
				return;
			}

			texture->setSmooth(true);
			waterChunk->_maskTexture = texture;
		}

		waterChunk->_maskTexture->update(pixels.data());

		// MESH
		std::vector<Anubis::Vertex3D> vertices;
		std::vector<std::uint32_t> indices;
		vertices.reserve(hc * hc);

		sf::Vector2f position = chunk->getWorldPosition();
		float step = chunk->getHeightStep();
		float weightStep = chunk->getWeightStep();
		int ratio = (wc - 1) / (hc - 1);	// mask points per height point (2)

		for (int y = 0; y < hc; y++) {
			for (int x = 0; x < hc; x++) {
				float xx = position.x + float(x) * step;
				float zz = position.y + float(y) * step;
				float u = (float(x) * step / weightStep + 0.5f) / float(wc);
				float v = (float(y) * step / weightStep + 0.5f) / float(wc);
				vertices.emplace_back(xx, chunk->_heights[y * hc + x], zz, 0.f, 1.f, 0.f, u, v);
			}
		}

		for (int y = 0; y < hc - 1; y++) {
			for (int x = 0; x < hc - 1; x++) {

				bool hasWater = false;

				for (int my = y * ratio; my <= (y + 1) * ratio && !hasWater; my++)
					for (int mx = x * ratio; mx <= (x + 1) * ratio && !hasWater; mx++)
						if (waterChunk->getMask(my * wc + mx) > 0) hasWater = true;

				if (!hasWater)
					continue;

				std::uint32_t index = (std::uint32_t)(y * hc + x);
				std::uint32_t below = index + (std::uint32_t)hc;

				indices.push_back(index);
				indices.push_back(below);
				indices.push_back(below + 1);

				indices.push_back(index);
				indices.push_back(below + 1);
				indices.push_back(index + 1);
			}
		}

		if (indices.empty()) {
			waterChunk->_mesh = nullptr;
			return;
		}

		std::shared_ptr<Anubis::Mesh3D> mesh = std::make_shared<Anubis::Mesh3D>();
		mesh->uploadToGPU(vertices, indices);
		waterChunk->_mesh = mesh;
	}

	void Water::updateAllChunks() {
		for (auto& waterChunk : _chunks)
			updateChunk(waterChunk);
	}

	WaterState Water::getState() {

		WaterState state;
		state._kind = _kind;
		state._level = _level;
		state._seeds = _seeds;

		for (auto& waterChunk : _chunks) {
			state._coords.push_back(waterChunk->_coords);
			state._fill.push_back(waterChunk->_fill);
			state._added.push_back(waterChunk->_added);
			state._removed.push_back(waterChunk->_removed);
		}

		return state;
	}

	void Water::setState(WaterState state) {

		_kind = state._kind;
		_level = state._level;
		_seeds = state._seeds;
		_chunks.clear();

		for (int i = 0; i < (int)state._coords.size(); i++) {
			std::shared_ptr<WaterChunk> waterChunk = std::make_shared<WaterChunk>(state._coords[i]);
			waterChunk->_fill = state._fill[i];
			waterChunk->_added = state._added[i];
			waterChunk->_removed = state._removed[i];
			_chunks.push_back(waterChunk);
		}

		updateAllChunks();
	}

	// Map::drawWaters() binds the shader and sets the common uniforms.
	void Water::draw() {

		std::shared_ptr<sf::Shader> shader = Anubis::shaders_manager->water3D_shader;
		shader->setUniform("uLevel", _level);
		shader->setUniform("uWaterType", _kind);
		shader->setUniform("uSelected", false);
		sf::Shader::bind(shader.get());

		for (auto& waterChunk : _chunks) {

			if (!waterChunk->_mesh || !waterChunk->_mesh->_geometry || !waterChunk->_maskTexture)
				continue;

			std::shared_ptr<Chunk> chunk = Game::module->_map->getChunkByCoords(waterChunk->_coords.x, waterChunk->_coords.y);

			if (!chunk || !chunk->_isVisible)
				continue;

			const auto& geometry = *waterChunk->_mesh->_geometry;

			if (geometry._vertexArray == 0 || geometry._indexCount == 0)
				continue;

			Anubis::GL::activeTexture(0x84C2); // GL_TEXTURE2
			glBindTexture(GL_TEXTURE_2D, waterChunk->_maskTexture->getNativeHandle());
			Anubis::GL::activeTexture(0x84C0); // GL_TEXTURE0

			Anubis::GL::bindVertexArray(geometry._vertexArray);
			glDrawElements(GL_TRIANGLES, geometry._indexCount, GL_UNSIGNED_INT, nullptr);
		}
	}

}

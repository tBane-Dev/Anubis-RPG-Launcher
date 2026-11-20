#pragma once
#include <SFML/Graphics.hpp>

enum class ObjectType { 
	None, Terrain, Monster, Nature, Player
};

class Object {
public:
	ObjectType _type;

	Object();
	virtual ~Object();
};

std::wstring ObjectTypeToWString(ObjectType type);
ObjectType WStringToObjectType(std::wstring type);
#pragma once
#include <any>
#include <vector>
#include <sstream>
#include <algorithm>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

struct QtRect {
	double x, y, width, height;

	bool contains(const QtRect &other) const noexcept;
	bool intersects(const QtRect &other) const noexcept;
	double getLeft() const noexcept;
	double getTop() const noexcept;
	double getRight() const noexcept;
	double getBottom() const noexcept;

	QtRect(const QtRect&);
	QtRect(double _x = 0, double _y = 0, double _width = 0, double _height = 0);
};

struct QtCollidable {
	friend class QtQuadTree;
public:
	QtRect bound;
	std::any data;

	QtCollidable(const QtRect &_bounds = {}, std::any _data = {});
private:
	QtQuadTree *qt = nullptr;
	QtCollidable(const QtCollidable&) = delete;
};

class QtQuadTree {
public:
	QtQuadTree(const QtRect &_bound, unsigned _capacity, unsigned _maxLevel);
	QtQuadTree(const QtQuadTree&);
	QtQuadTree();

	bool insert(QtCollidable *obj);
	bool remove(QtCollidable *obj);
	bool update(QtCollidable *obj);
	std::vector<QtCollidable*> &getObjectsInBound_unchecked(const QtRect &bound);
	unsigned totalChildren() const noexcept;
	unsigned totalObjects() const noexcept;
	void setFont(const sf::Font &font) noexcept;
	void draw(sf::RenderTarget &canvas) noexcept;
	void clear() noexcept;
	QtQuadTree *getLeaf(const QtRect &bound);

	~QtQuadTree();
private:
	bool      isLeaf = true;
	unsigned  level = 0;
	unsigned  capacity;
	unsigned  maxLevel;
	QtRect      bounds;
	QtQuadTree* parent = nullptr;
	QtQuadTree* children[4] = { nullptr, nullptr, nullptr, nullptr };
	sf::Text  text;
	sf::RectangleShape	     shape;
	std::vector<QtCollidable*> objects, foundObjects;

	void subdivide();
	void discardEmptyBuckets();
	inline QtQuadTree *getChild(const QtRect &bound) const noexcept;
};



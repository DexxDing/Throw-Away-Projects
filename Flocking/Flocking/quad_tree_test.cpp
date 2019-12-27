#include "quad_tree_test.h"

//** QtRect **//
QtRect::QtRect(double _x, double _y, double _width, double _height) :
	x(_x),
	y(_y),
	width(_width),
	height(_height) { }
QtRect::QtRect(const QtRect &other) : QtRect(other.x, other.y, other.width, other.height) { }

bool QtRect::contains(const QtRect &other) const noexcept {
	if (x > other.x) return false;
	if (y > other.y) return false;
	if (x + width < other.x + other.width) return false;
	if (y + height < other.y + other.height) return false;
	return true; // within bounds
}
bool QtRect::intersects(const QtRect &other) const noexcept {
	if (x > other.x + other.width)  return false;
	if (x + width < other.x)        return false;
	if (y > other.y + other.height) return false;
	if (y + height < other.y)       return false;
	return true; // intersection
}
double QtRect::getLeft()   const noexcept { return x - (width  * 0.5f); }
double QtRect::getTop()    const noexcept { return y + (height * 0.5f); }
double QtRect::getRight()  const noexcept { return x + (width  * 0.5f); }
double QtRect::getBottom() const noexcept { return y - (height * 0.5f); }

//** QtCollidable **//
QtCollidable::QtCollidable(const QtRect &_bounds, std::any _data) :
	bound(_bounds),
	data(_data) {
};

//** QtQuadTree **//
QtQuadTree::QtQuadTree() : QtQuadTree({}, 0, 0) { }
QtQuadTree::QtQuadTree(const QtQuadTree &other) : QtQuadTree(other.bounds, other.capacity, other.maxLevel) { }
QtQuadTree::QtQuadTree(const QtRect &_bound, unsigned _capacity, unsigned _maxLevel) :
	bounds(_bound),
	capacity(_capacity),
	maxLevel(_maxLevel) {

	objects.reserve(_capacity);
	foundObjects.reserve(_capacity);
	shape.setOutlineThickness(1.f);
	shape.setSize(sf::Vector2f((float)bounds.width - shape.getOutlineThickness(), (float)bounds.height));
	shape.setOutlineColor(sf::Color(42, 42, 42));
	shape.setPosition((float)_bound.x, (float)_bound.y);
	shape.setFillColor(sf::Color(242, 242, 242));
	text.setFillColor(sf::Color(42, 42, 42));
}

// Inserts an object into this quadtree
bool QtQuadTree::insert(QtCollidable *obj) {
	if (obj->qt != nullptr) return false;

	if (!isLeaf) {
		// insert object into leaf
		if (QtQuadTree *child = getChild(obj->bound))
			return child->insert(obj);
	}
	objects.push_back(obj);
	obj->qt = this;

	// Subdivide if required
	if (isLeaf && level < maxLevel && objects.size() >= capacity) {
		subdivide();
		update(obj);
	}
	return true;
}

// Removes an object from this quadtree
bool QtQuadTree::remove(QtCollidable *obj) {
	if (obj->qt == nullptr) return false; // Cannot exist in vector
	if (obj->qt != this) return obj->qt->remove(obj);

	objects.erase(std::find(objects.begin(), objects.end(), obj));
	obj->qt = nullptr;
	discardEmptyBuckets();
	return true;
}

// Removes and re-inserts object into quadtree (for objects that move)
bool QtQuadTree::update(QtCollidable *obj) {
	if (!remove(obj)) return false;

	// Not contained in this node -- insert into parent
	if (parent != nullptr && !bounds.contains(obj->bound))
		return parent->insert(obj);
	if (!isLeaf) {
		// Still within current node -- insert into leaf
		if (QtQuadTree *child = getChild(obj->bound))
			return child->insert(obj);
	}
	return insert(obj);
}

// Searches quadtree for objects within the provided boundary and returns them in vector
std::vector<QtCollidable*> &QtQuadTree::getObjectsInBound_unchecked(const QtRect &bound) {
	foundObjects.clear();
	for (const auto &obj : objects) {
		// Only check for intersection with OTHER boundaries
		foundObjects.emplace_back(obj);
	}
	if (!isLeaf) {
		// Get objects from leaves
		if (QtQuadTree *child = getChild(bound)) {
			child->getObjectsInBound_unchecked(bound);
			foundObjects.insert(foundObjects.end(), child->foundObjects.begin(), child->foundObjects.end());
		}
		else for (QtQuadTree *leaf : children) {
			if (leaf->bounds.intersects(bound)) {
				leaf->getObjectsInBound_unchecked(bound);
				foundObjects.insert(foundObjects.end(), leaf->foundObjects.begin(), leaf->foundObjects.end());
			}
		}
	}
	return foundObjects;
}

// Returns total children count for this quadtree
unsigned QtQuadTree::totalChildren() const noexcept {
	unsigned total = 0;
	if (isLeaf) return total;
	for (QtQuadTree *child : children)
		total += child->totalChildren();
	return 4 + total;
}

// Returns total object count for this quadtree
unsigned QtQuadTree::totalObjects() const noexcept {
	unsigned total = (unsigned)objects.size();
	if (!isLeaf) {
		for (QtQuadTree *child : children)
			total += child->totalObjects();
	}
	return total;
}

void QtQuadTree::setFont(const sf::Font &font) noexcept {
	text.setFont(font);
	text.setCharacterSize(40 / (level ? level : 1));
	text.setPosition(
		(float)bounds.getRight() - (text.getLocalBounds().width  * 0.5f),
		(float)bounds.getTop() - 1 - (text.getLocalBounds().height * 0.5f)
	);
	if (isLeaf) return;
	for (QtQuadTree *child : children)
		child->setFont(font);
}

void QtQuadTree::draw(sf::RenderTarget &canvas) noexcept {
	shape.setFillColor(sf::Color::Transparent);

	canvas.draw(shape);
	if (!isLeaf) {
		setFont(*text.getFont());
		for (QtQuadTree *child : children)
			child->draw(canvas);
	}
	else {
		std::stringstream ss;
		ss << level;
		text.setString(ss.str());
		setFont(*text.getFont());
		canvas.draw(text);
	}
}

// Removes all objects and children from this quadtree
void QtQuadTree::clear() noexcept {
	if (!objects.empty()) {
		for (auto&& obj : objects)
			obj->qt = nullptr;
		objects.clear();
	}
	if (!isLeaf) {
		for (QtQuadTree *child : children)
			child->clear();
		isLeaf = true;
	}
}

// Subdivides into four quadrants
void QtQuadTree::subdivide() {
	double width = bounds.width  * 0.5f;
	double height = bounds.height * 0.5f;
	double x = 0, y = 0;
	for (unsigned i = 0; i < 4; ++i) {
		switch (i) {
		case 0: x = bounds.x + width; y = bounds.y; break; // Top right
		case 1: x = bounds.x;         y = bounds.y; break; // Top left
		case 2: x = bounds.x;         y = bounds.y + height; break; // Bottom left
		case 3: x = bounds.x + width; y = bounds.y + height; break; // Bottom right
		}
		children[i] = new QtQuadTree({ x, y, width, height }, capacity, maxLevel);
		children[i]->level = level + 1;
		children[i]->parent = this;
	}
	isLeaf = false;
}

// Discards buckets if all children are leaves and contain no objects
void QtQuadTree::discardEmptyBuckets() {
	if (!objects.empty()) return;
	if (!isLeaf) {
		for (QtQuadTree *child : children)
			if (!child->isLeaf || !child->objects.empty())
				return;
	}
	if (clear(), parent != nullptr)
		parent->discardEmptyBuckets();
}

QtQuadTree *QtQuadTree::getLeaf(const QtRect &bound) {
	QtQuadTree *leaf = this;
	if (!isLeaf) {
		if (QtQuadTree *child = getChild(bound))
			leaf = child->getLeaf(bound);
	}
	return leaf;
}

// Returns child that contains the provided boundary
QtQuadTree *QtQuadTree::getChild(const QtRect &bound) const noexcept {
	bool left = bound.x + bound.width < bounds.getRight();
	bool right = bound.x > bounds.getRight();

	if (bound.y + bound.height < bounds.getTop()) {
		if (left)  return children[1]; // Top left
		if (right) return children[0]; // Top right
	}
	else if (bound.y > bounds.getTop()) {
		if (left)  return children[2]; // Bottom left
		if (right) return children[3]; // Bottom right
	}
	return nullptr; // Cannot contain boundary -- too large
}

QtQuadTree::~QtQuadTree() {
	clear();
	if (children[0]) delete children[0];
	if (children[1]) delete children[1];
	if (children[2]) delete children[2];
	if (children[3]) delete children[3];
}
#pragma once
#include <SFML/Graphics.hpp>
#include <list>

class boid : public sf::Drawable
{
public:
	sf::Rect<float> bound_box;
	QtCollidable hit_box { {0,0,0,0}, this };
	float bound_dist;
	sf::Vector2f pos;
	sf::Vector2f vel;
	sf::ConvexShape shape;
	std::vector<sf::Vertex> trail;

	float scale = 1;

	float max_speed = 0.2* scale;
	float sep_max = 0.009* scale;
	float coh_max = 0.005* scale;
	float ali_max = 0.003* scale;
	float lead_max = 0.010* scale;
	float bound_var = 0.00003* scale;

	
	float sep_var = 20* scale;
	float coh_var = 70* scale;
	float ali_var = 50 * scale;
	float max_var;

	

	float periphery = 3.14 / 3;
	bool debug = 0;

	int team;


	boid(sf::Vector2f set_pos = { 0,0 }, sf::Vector2f set_vel = { 0,0 })
	{

		trail.resize(50);
		max_var = fmaxf(sep_var, fmaxf(coh_var, ali_var));

		bound_dist = 100;

		pos = set_pos;
		vel = set_vel;

		shape.setPointCount(3);
		shape.setPoint(0, sf::Vector2f(0.f, 0.f));
		shape.setPoint(1, sf::Vector2f(-10.f*scale, 5.f*scale));
		shape.setPoint(2, sf::Vector2f(-10.f*scale, -5.f*scale));
		shape.setOrigin(sf::Vector2f(-5.f*scale, 0.f));
		shape.setPosition(pos);
		shape.setRotation(vel.getAngle());

		team = rand() % 2;

		
		for (int i = 0; i < trail.size() - 1; i += 2)
		{
			trail[i] = (shape.getTransform().transformPoint(shape.getPoint(1)));
			trail[i + 1] = (shape.getTransform().transformPoint(shape.getPoint(2)));
		}
	}

	void update(float time)
	{

		vel += bound();
		if (vel.getLength() > max_speed)
		{
			vel.normalize();
			vel *= max_speed;
		}
		
		pos += vel * time;
		shape.setPosition(pos);
		shape.setRotation(vel.getAngle());
		if (team == 0) shape.setFillColor({ 0,(sf::Uint8)(vel.getLengthSq() / (max_speed*max_speed) * 255), 255 });
		if (team == 1) shape.setFillColor({ 255,(sf::Uint8)(vel.getLengthSq() / (max_speed*max_speed) * 255), 0 });

		trail.resize(1000.f / time, sf::Vertex({ NAN,NAN }));

		for (int i = trail.size() - 1; i > 1; --i)
			trail[i] = trail[i - 2];

		trail[0] = (shape.getTransform().transformPoint(shape.getPoint(1)));
		trail[1] = (shape.getTransform().transformPoint(shape.getPoint(2)));


		for (int i = 0; i < trail.size(); ++i)
			trail[i].color = { shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b , (sf::Uint8)(((float)(trail.size() - i) / (trail.size()) * 255)) };
		
		hit_box.bound = { pos.x - max_var / 2.f, pos.y - max_var / 2.f, max_var, max_var};
	}

	void tail_reset()
	{
		shape.setPosition(pos);
		shape.setRotation(vel.getAngle());
		for (int i = 0; i < trail.size() - 1; i += 2)
		{
			trail[i] = (shape.getTransform().transformPoint(shape.getPoint(1)));
			trail[i + 1] = (shape.getTransform().transformPoint(shape.getPoint(2)));
		}
	}

	void force(std::list<boid*> &bds)
	{
		vel += separation(bds);
		vel += alignment(bds);
		vel += cohesion(bds);
		
		//vel += leadership(bds);
	}

	void border_control(sf::Rect<float> bounding_box)
	{
		bound_box = bounding_box;
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		//if (debug)
		//{
		//	target.draw(rec, states);

		//	sf::VertexArray line(sf::LinesStrip, 2);
		//	line[0].color = sf::Color::White;
		//	line[0].position = pos;

		//	line[1].color = sf::Color::Blue;
		//	line[1].position = (sep_s / sep_max) * 50.f + pos;
		//	target.draw(line);

		//	line[1].color = sf::Color::Red;
		//	line[1].position = (coh_s / coh_max) * 50.f + pos;
		//	target.draw(line);

		//	line[1].color = sf::Color::Green;
		//	line[1].position = (ali_s / ali_max) * 50.f + pos;
		//	target.draw(line);

		//	line[1].color = sf::Color::Yellow;
		//	line[1].position = (lead_s / lead_max) * 50.f + pos;
		//	target.draw(line);
		//}
		sf::RectangleShape rec;
		rec.setOutlineThickness(1);
		rec.setPosition({ (float)hit_box.bound.x, (float)hit_box.bound.y });
		rec.setSize({ (float)hit_box.bound.width, (float)hit_box.bound.height });
		rec.setFillColor(sf::Color::Transparent);
		rec.setOutlineColor(sf::Color::Red);

		target.draw(rec, states);
		target.draw(trail.data(), trail.size(), sf::TriangleStrip, states);
		target.draw(shape, states);
	}

	

	bool view(sf::Vector2f &target, float radius, float angle);
	sf::Vector2f arrive(sf::Vector2f target, float max_force);
	sf::Vector2f seek(sf::Vector2f target, float max_force);
	sf::Vector2f cohesion(std::list<boid*> &bds);
	sf::Vector2f separation(std::list<boid*> &bds);
	sf::Vector2f alignment(std::list<boid*> &bds);
	sf::Vector2f leadership(std::list<boid*> &bds);
	sf::Vector2f bound();
};


bool boid::view(sf::Vector2f &target, float radius, float angle)
{
	sf::Vector2f compare = target - pos;
	float d = compare.getLength();
	float diff = acos((compare.x * vel.x + compare.y * vel.y) / (d * vel.getLength()));
	return ((diff < angle) && (d > 0) && (d < radius));

}

sf::Vector2f boid::leadership(std::list<boid*> &bds)
{

	sf::Vector2f steer{ 0, 0 };
	int count = 0;


	sf::Vector2f right = vel;
	right.normalize();
	right.rotateBy(90);

	float prod = 0;
	for (auto &bd : bds) {
		if (view(bd->pos, 70, 3.14/2))
		{


			sf::Vector2f diff(0, 0);
			sf::Vector2f des = bd->pos - pos;
			des.normalize();
			float dot = (right.x * des.x) + (right.y * des.y);
			dot = 1.f / dot;
			steer -= (right * dot + vel * dot);
			++count;
		}
	}

	steer /= 300.f;

	if (count > 0) steer /= ((float)count);

	if (steer.getLength() > lead_max)
	{
		steer.normalize();
		steer *= lead_max;
	}
	
	return steer;


}

sf::Vector2f boid::arrive(sf::Vector2f target, float max_force)
{
	sf::Vector2f des = (target - pos);
	des /= 300.f;
	if (des.getLengthSq() > max_speed*max_speed)
	{
		des.normalize();
		des *= max_speed;
	}
	des -= vel;

	if (des.getLengthSq() > max_force*max_force)
	{
		des.normalize();
		des *= max_force;
	}

	return des;
}


sf::Vector2f boid::seek(sf::Vector2f target, float max_force)
{
	sf::Vector2f des = (target - pos).normalize();
	des *= max_speed;
	des -= vel;

	if (des.getLengthSq() > max_force*max_force)
	{
		des.normalize();
		des *= max_force;
	}
	return des;
}


sf::Vector2f boid::cohesion(std::list<boid*> &bds)
{
	sf::Vector2f sum(0, 0);

	int count = 0;
	if (bds.size() > 2)
	for (auto &bd : bds) {
		if (view(bd->pos,coh_var, periphery) && (bd->team == team) )
		{
			sum += bd->pos;
			++count;
		}
	}

	if (count > 0) 
	{
		sum /= (float)count;
		return seek(sum, coh_max);
	}
	else return { 0,0 };
}

sf::Vector2f boid::separation(std::list<boid*> &bds)
{

	sf::Vector2f steer(0, 0);
	int count = 0;
	 
	for (auto &bd : bds) {

		float d = (bd->pos - pos).getLengthSq();

		if ((d > 0) && (d < sep_var*sep_var)) {
			sf::Vector2f diff(0, 0);
			diff -= bd->pos - pos;
			diff.normalize();
			diff /= sqrt(d);
			steer += diff;
			++count;
		}
		// If current boid is a predator and the boid we're looking at is also
		// a predator, then separate only slightly
		//if ((d > 0) && (d < desiredseparation) && predator == true
		//	&& boids[i].predator == true) {
		//	Pvector pred2pred(0, 0);
		//	pred2pred = pred2pred.subTwoVector(location, boids[i].location);
		//	pred2pred.normalize();
		//	pred2pred.divScalar(d);
		//	steer += pred2pred;
		//	++count;
		//}
		// If current boid is not a predator, but the boid we're looking at is
		// a predator, then create a large separation Pvector
		//if (view(bd->pos, sep_var*3, periphery) && (bd->team != team))
		//{
		//	sf::Vector2f pred(0, 0);
		//	pred -= bd->pos - pos;
		//	pred *= 900.f;
		//	steer += pred;
		//	++count;
		//}
	}

	if (count > 0) steer /= ((float)count);

	if (steer.getLengthSq() > 0) {
		
		steer.normalize();

		steer *= max_speed;
		steer -= vel;

		if (steer.getLength() > sep_max)
		{
			steer.normalize();
			steer *= sep_max;
		}
	}
	return steer;
}

sf::Vector2f boid::alignment(std::list<boid*> &bds)
{

	sf::Vector2f sum(0, 0);
	int count = 0;
	if(bds.size() > 2)
	for (auto &bd : bds) {
		if (view(bd->pos, ali_var, periphery) && (bd->team == team)) { // 0 < d < 50
			sum += bd->vel;
			count++;
		}
	}

	if (count > 0) {
		sum /= ((float)count);
		sum.normalize();
		sum *= (max_speed);
		
		sf::Vector2f steer;
		steer -= vel - sum;


		if (steer.getLengthSq() > ali_max*ali_max)
		{
			steer.normalize();
			steer *= ali_max;
		}

		return steer;
	}
	else return { 0,0 };
}

sf::Vector2f boid::bound()
{
	sf::Vector2f steer(0, 0);

	if (pos.x < bound_box.left + bound_dist) steer.x += -(pos.x - (bound_box.left + bound_dist))*bound_var;
	if (pos.y < bound_box.top + bound_dist) steer.y += -(pos.y - (bound_box.top + bound_dist))*bound_var;
	if (pos.x > bound_box.width - bound_dist) steer.x += -(pos.x - (bound_box.width - bound_dist))*bound_var;
	if (pos.y > bound_box.height - bound_dist) steer.y += -(pos.y - (bound_box.height - bound_dist))*bound_var;
	

	return steer;
}
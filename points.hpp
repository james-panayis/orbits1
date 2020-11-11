#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <map>

#include <iomanip>

#include "vec3.hpp"

extern float projectionMatrix_[16];
extern int pixel_width_;
extern int pixel_height_;

namespace james {

	class points {

	public:

		struct point
		{

			point(int i, vec3 p, vec3 v, double m, double r, vec3 pa) noexcept :
				id(i),
				position(p),
				velocity(v),
				mass(m),
				radius(r),
				pacceleration(pa)
			{
			}

			int id;
			vec3 position;
			vec3 velocity;
			double mass;
			double radius;
			vec3 pacceleration;
		};

		struct pointui
		{
			vec3 position;
			vec3 velocity;
			double mass;
			double radius;
		};

		points()
		{
		};

		~points()
		{
		};

		int add_point(pointui p)
		{
			//auto& np = points_.emplace_back({id_counter_++, p.position, p.velocity, p.mass, p.radius, {0, 0, 0} });
			auto& np = points_.emplace_back(point(id_counter_++, p.position, p.velocity, p.mass, p.radius, {0, 0, 0} ));
			return np.id;
		}

		int set_point(pointui p, int i)
		{
			//auto& np = points_.emplace_back({id_counter_++, p.position, p.velocity, p.mass, p.radius, {0, 0, 0} });
			points_[i].position = p.position;
			points_[i].velocity = p.velocity;
			points_[i].mass = p.mass;
			points_[i].radius = p.radius;
			return points_[i].id;
		}

		const std::vector<point>& get_points()
		{
			return points_;
		}
		
		void set_zero_net_momentum()
		{
			vec3 tmomentum;
			double tmass;
			for (const auto& p : points_) {
				tmass += p.mass;
				tmomentum = tmomentum + (p.velocity * p.mass);
			}
			vec3 vadjustment = tmomentum / tmass;
			for (auto& p : points_) {
				p.velocity = p.velocity - vadjustment;
			}
		}

		vec3 get_position_offset()
		{
			vec3 tposition;
			double tmass;
			for (const auto& p : points_) {
				tmass += p.mass;
				tposition = tposition + (p.position * p.mass);
			}
			vec3 padjustment = tposition / tmass;

			return padjustment;
		}

		void print_net_momentum()
		{
			vec3 tmomentum;
			for (const auto& p : points_) {
				tmomentum = tmomentum + (p.velocity * p.mass);
			}
			//std::cout << std::fixed << std::setprecision(5) << tmomentum.x << "  " << tmomentum.y << "  " << tmomentum.z << std::endl;;

		}

		void print_net_energy()
		{
			double tenergy;
			for (const auto& p : points_) {
				tenergy += 0.5*p.mass/(6.67408*0.00000000001)*std::pow(p.velocity.length(), 2);
			}
			for (int i = 0; i < points_.size(); i++) {
				for (int j = i + 1; j < points_.size(); j++) {
				tenergy -= points_[i].mass * points_[j].mass / (6.67408*0.00000000001) / points_[i].position.dist(points_[j].position);	
				}
			}
			//std::cout << tenergy << std::endl;;
		}

		void iterations(double timeinc, unsigned int steps = 1)
		{
			double timestep = timeinc / steps;
			for (int i = 0; i < steps; i++) {
				iterate(timestep);
			}
		}

		void iterate(double timeinc)
		{
			const int psize = points_.size();
			const double cutoff = 20;
			const double hti = 0.5 * timeinc;
			// calculate displacement vectors multiplied by masses between every pair of points
			std::vector<vec3> forces;
			forces.resize(psize);
			vec3 tempdisp;
			double templen;
			vec3 tempforce;
			for (int i = 0; i < psize; i++) {
				auto& point_i = points_[i];
				//auto& v = forces.emplace_back();
				for (int j = i + 1; j < psize; j++) {
					auto& point_j = points_[j];
					tempdisp = (point_j.position - point_i.position);
					if (!tempdisp.empty()){
						templen = tempdisp.length();
						if (templen < point_i.radius + point_j.radius) {
							double tmass = point_i.mass + point_j.mass;
							pointui temp = pointui( { {(point_i.position.x * point_i.mass + point_j.position.x * point_j.mass)/tmass, (point_i.position.y * point_i.mass + point_j.position.y * point_j.mass)/tmass, (point_i.position.z * point_i.mass + point_j.position.z * point_j.mass)/tmass}, {(point_i.velocity.x * point_i.mass + point_j.velocity.x * point_j.mass)/tmass, (point_i.velocity.y * point_i.mass + point_j.velocity.y * point_j.mass)/tmass, (point_i.velocity.z * point_i.mass + point_j.velocity.z * point_j.mass)/tmass}, tmass, std::pow(std::pow(point_i.radius, 3.0) + std::pow(point_j.radius, 3.0), 1.0/3.0)} );
							set_point(temp, std::min(i,j));
							//points_.erase(points_.begin()+ j);
							points_.erase(points_.begin()+ std::max(i, j));
							iterations(timeinc, 1);
							return;
						}
						tempdisp = ( tempdisp / (templen * templen * templen) ) * timeinc;
						templen = timeinc / templen / templen; //no-longer a length, now a force
						if ((point_j.mass * templen > cutoff) || (point_i.mass * templen > cutoff)) {
							//std::cout << "r" << i << "," << j << std::endl;
							iterations(timeinc, 2);
							return;
						}
						templen = point_i.position.dist(point_j.position);
						/*if (templen < point_i.radius + point_j.radius) {
							if (((point_j.mass+point_i.mass) * (((point_j.position - point_i.position) / templen) * (point_i.radius + point_j.radius - templen) * point_j.mass).length()) > cutoff * 10000000000000000000000000000000000000.0) {
							//std::cout << "r" << i << "," << j << std::endl;
							iterations(timeinc, 2);
							return;
							}
							forces[i] = forces[i] - ((point_j.position - point_i.position) / templen) * (point_i.radius + point_j.radius - templen) * point_j.mass / (point_i.mass + point_j.mass)/1000000.0;
							forces[j] = forces[j] - ((point_i.position - point_j.position) / templen) * (point_j.radius + point_i.radius - templen) * point_i.mass / (point_i.mass + point_j.mass)/1000000.0;
							//point_i.velocity = point_i.velocity - (((point_j.position - point_i.position) / templen) * (point_i.radius + point_j.radius - templen) * (point_j.mass/(point_i.mass+point_j.mass)));
							//point_j.velocity = point_j.velocity - (((point_i.position - point_j.position) / templen) * (point_j.radius + point_i.radius - templen) * (point_i.mass/(point_i.mass+point_j.mass)));
							//continue;
						} else {*/
						forces[i] = forces[i] + (tempdisp * point_j.mass);
						forces[j] = forces[j] - (tempdisp * point_i.mass);
						/*}*/
					}
				}
			}
			// increment positions and then velocities of all points 
			int i = 0;
			for (auto& p : points_) {
				p.position = p.position + (p.velocity * timeinc) + (forces[i] * hti);
				p.velocity = p.velocity + forces[i];
				i++;
			}
		}

		int size()
		{
			return points_.size();
		}


	private:

		inline static int id_counter_ = 0;
		
		std::vector<point> points_;
	
	}; // class points


} // namespace

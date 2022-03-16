#pragma once

#include <Windows.h>
#include <vector>
#include <map>

#include "WorkDistributor.h"
#include "vmath.h"

#include "interface.h"

namespace GS {
	class VoxelParticleSystem {
		enum {
			NOT_REGISTERED = UINT32_MAX,
			EMPTY = UINT32_MAX
		};
	private:
		//Design
		UINT kMaxVoxelSearchRange = 5;

		//::Object
		WorkDistributor* o_wd_;

		struct {
			UINT num_threads;
			UINT num_works;
			UINT iteration;
			FLOAT tick;
			FLOAT limit;
			struct {
				FLOAT tick_sq;
				FLOAT tick_recip;
				FLOAT limit_c;
			}p;
			struct {
				FLOAT unit;
				UINT scale_x;
				UINT scale_y;
				UINT scale_z;
				vmath::vec3 pos_center;
				UINT num_elements;
				struct {
					FLOAT intr_distance;
					FLOAT intr_offset;
					struct {
						FLOAT max_intr_distance;
						FLOAT intr_offset_recip;
					}p;
				}density;
				struct {
					FLOAT unit_recip;
					UINT scale_xy;
					vmath::vec3 pos_offset;
					vmath::vec3 pos_origin;
					UINT num;
					UINT num_total_contents;
					std::vector<INT> search_offsets;
					std::vector<INT> search_offsets_2;
				}p;
			}voxel;
			struct {
				FLOAT interval;
				FLOAT diameter;
				FLOAT max_intr_distance;
				FLOAT mass;
				UINT num;
				UINT num_neighbors;
				struct {
					FLOAT diameter_recip;
					FLOAT mass_recip;
					FLOAT radius;
				}p;
			}particles;
		}v_;

		struct {
			vmath::vec3 g_gravity;
			FLOAT g_reduction;
			struct {
				vmath::vec3 g_gravity_t;
				FLOAT g_reduction_t;

				vmath::vec3 g_gravity_tsqm;
			}p;
			struct {
				FLOAT spring;
				FLOAT damping;
				FLOAT shear;
				FLOAT drag;
				FLOAT attraction;
				struct {
					FLOAT spring_t;

					FLOAT spring_tsqm;
					FLOAT damping_tsqm;
					FLOAT shear_tsqm;
					FLOAT drag_tsqm;
					FLOAT attraction_tsqm;
				}p;
			}particle_coef;
		}c_;

		struct {
			struct {
				std::vector<UINT> contents;
				std::vector<UINT> registered;
				std::vector<FLOAT> density;
			}voxel;
			struct {
				std::vector<vmath::vec3> pos_curr;
				std::vector<vmath::vec3> pos_past;
				std::vector<vmath::vec3> vel_curr;
				std::vector<vmath::vec3> vel_past;
			}particles;
		}data_;

	public:
		VoxelParticleSystem() {}
		~VoxelParticleSystem() {}

		void Init(options_vps* opt_vps);
		void Startup();
		void Control(controls_vps* ctrl_vps);
		void Update();
		void Terminate();

		UINT get_particles_num();
		UINT get_particles_pos_buffer_size();
		void* get_particles_pos();
		void* get_density_array();
	private:
		//Init
		void _InitSearchOffset();
		void _InitSearchOffset2();

		//Startup
		void _SetDefaultData();

		//Control

		//Update
		void _UpdateParticles(UINT index_begin, UINT index_end);
		void _UpdateParticles2(UINT index_begin, UINT index_end);
		void _SolveCollision(UINT index_begin, UINT index_end);
		void _EmptyVoxel(UINT index_begin, UINT index_end);
		void _UpdateVoxel(UINT index_begin, UINT index_end);


		//Terminate

		//::AUX
		void _UpdateDensityArray(UINT index_begin, UINT index_end);
		void _UpdateDensityArray2(UINT index_begin, UINT index_end);
	};
}




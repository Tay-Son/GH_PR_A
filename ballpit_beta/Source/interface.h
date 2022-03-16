#pragma once

namespace GS {
	struct options_vps {
		UINT num_threads;
		UINT num_works;
		UINT iteration;
		FLOAT tick;
		FLOAT limit;
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
			}density;
		}voxel;
		struct {
			FLOAT interval;
			FLOAT diameter;
			FLOAT max_intr_distance;
			FLOAT mass;
			UINT num;
			UINT num_neighbors;
		}particles;
	};

	struct controls_vps {
		vmath::vec3 g_gravity;
		FLOAT g_reduction;

		struct {
			FLOAT spring;
			FLOAT damping;
			FLOAT shear;
			FLOAT drag;
			FLOAT attraction;

		}particle_coef;
	};
	struct {

	}opt_camera;

	struct {

	}ctrl_camera;
}







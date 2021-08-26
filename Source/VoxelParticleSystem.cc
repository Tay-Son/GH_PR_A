#pragma once

#include "VoxelParticleSystem.h"

namespace GS {
	//::Public
	void VoxelParticleSystem::Init(options_vps* opt_vps) {
		//::Last Edited At 2020-08-24-20:59 by STH.

		//::
		v_.num_threads = opt_vps->num_threads;
		v_.num_works = opt_vps->num_works;
		v_.iteration = opt_vps->iteration;
		v_.tick = opt_vps->tick;
		v_.limit = opt_vps->limit;

		v_.voxel.unit = opt_vps->voxel.unit;
		v_.voxel.scale_x = opt_vps->voxel.scale_x;
		v_.voxel.scale_y = opt_vps->voxel.scale_y;
		v_.voxel.scale_z = opt_vps->voxel.scale_z;
		v_.voxel.pos_center = opt_vps->voxel.pos_center;
		v_.voxel.num_elements = opt_vps->voxel.num_elements;

		v_.voxel.density.intr_distance = opt_vps->voxel.density.intr_distance;
		v_.voxel.density.intr_offset = opt_vps->voxel.density.intr_offset;

		v_.particles.interval = opt_vps->particles.interval;
		v_.particles.diameter = opt_vps->particles.diameter;
		v_.particles.max_intr_distance = opt_vps->particles.max_intr_distance;
		v_.particles.mass = opt_vps->particles.mass;
		v_.particles.num = opt_vps->particles.num;
		v_.particles.num_neighbors = opt_vps->particles.num_neighbors;

		//::Check


		//::
		v_.p.tick_sq = v_.tick * v_.tick;
		v_.p.tick_recip = 1.0f / v_.tick;
		v_.p.limit_c = v_.limit - v_.particles.diameter * 0.5f;

		v_.voxel.p.unit_recip = 1.0f / v_.voxel.unit;
		v_.voxel.p.scale_xy = v_.voxel.scale_x * v_.voxel.scale_y;
		v_.voxel.p.pos_offset = vmath::vec3((FLOAT)v_.voxel.scale_x, (FLOAT)v_.voxel.scale_y, (FLOAT)v_.voxel.scale_z) * v_.voxel.unit;
		v_.voxel.p.pos_origin = v_.voxel.p.pos_offset * -0.5f;
		v_.voxel.p.num = v_.voxel.p.scale_xy * v_.voxel.scale_z;
		v_.voxel.p.num_total_contents = v_.voxel.p.num * v_.voxel.num_elements;
		v_.voxel.density.p.intr_offset_recip = 1.0f / v_.voxel.density.intr_offset;
		v_.voxel.density.p.max_intr_distance = v_.voxel.density.intr_distance + v_.voxel.density.intr_offset;

		v_.particles.p.diameter_recip = 1.0f / v_.particles.diameter;
		v_.particles.p.mass_recip = 1.0f / v_.particles.mass;
		v_.particles.p.radius = v_.particles.diameter * 0.5f;

		_InitSearchOffset();
		_InitSearchOffset2();

		//::
		o_wd_ = new WorkDistributor(v_.num_threads);
	}
	void VoxelParticleSystem::Startup() {
		//::Data - Voxel
		std::vector<UINT>(v_.voxel.p.num_total_contents, EMPTY).swap(data_.voxel.contents);
		std::vector<UINT>(v_.particles.num, NOT_REGISTERED).swap(data_.voxel.registered);
		data_.voxel.density.resize(v_.voxel.p.num);

		//::Data - Particles
		data_.particles.pos_curr.resize(v_.particles.num);
		data_.particles.vel_curr.resize(v_.particles.num);
		data_.particles.pos_past.resize(v_.particles.num);
		data_.particles.vel_past.resize(v_.particles.num);

		_SetDefaultData();

		for (UINT work_index = 0; work_index < v_.num_works; work_index++) {
			UINT index_begin = (v_.particles.num * work_index) / v_.num_works;
			UINT index_end = (v_.particles.num * (work_index + 1)) / v_.num_works;
			o_wd_->ReserveWork([this, index_begin, index_end]() {_UpdateVoxel(index_begin, index_end); });
		}
		o_wd_->ExecuteWork();

	}
	void VoxelParticleSystem::Control(controls_vps* ctrl_vps) {

		c_.g_gravity = ctrl_vps->g_gravity;
		c_.g_reduction = ctrl_vps->g_reduction;
		c_.particle_coef.spring = ctrl_vps->particle_coef.spring;
		c_.particle_coef.damping = ctrl_vps->particle_coef.damping;
		c_.particle_coef.shear = ctrl_vps->particle_coef.shear;
		c_.particle_coef.drag = ctrl_vps->particle_coef.drag;
		c_.particle_coef.attraction = ctrl_vps->particle_coef.attraction;



		//::
		FLOAT temp = v_.p.tick_sq / v_.particles.mass;
		c_.p.g_gravity_tsqm = c_.g_gravity * temp;
		c_.p.g_gravity_t = c_.g_gravity * v_.tick;
		//c_.p.g_reduction_t = c_.g_reduction * v_.tick;
		c_.p.g_reduction_t = 0.95f;

		c_.particle_coef.p.spring_t = c_.particle_coef.spring * v_.tick;

		c_.particle_coef.p.spring_tsqm = c_.particle_coef.spring * temp;
		c_.particle_coef.p.damping_tsqm = c_.particle_coef.damping * temp;
		c_.particle_coef.p.shear_tsqm = c_.particle_coef.shear * temp;
		c_.particle_coef.p.drag_tsqm = c_.particle_coef.drag * temp;
		c_.particle_coef.p.attraction_tsqm = c_.particle_coef.attraction * temp;
	}
	void VoxelParticleSystem::Update() {
		for (UINT count = 0; count < v_.iteration; count++) {
			data_.particles.pos_past.swap(data_.particles.pos_curr);
			data_.particles.vel_past.swap(data_.particles.vel_curr);

			for (UINT work_index = 0; work_index < v_.num_works; work_index++) {
				UINT index_begin = (v_.particles.num * work_index) / v_.num_works;
				UINT index_end = (v_.particles.num * (work_index + 1)) / v_.num_works;
				o_wd_->ReserveWork([this, index_begin, index_end]() {_UpdateParticles(index_begin, index_end); });
			}
			o_wd_->ExecuteWork();

			for (UINT work_index = 0; work_index < v_.num_works; work_index++) {
				UINT index_begin = (v_.particles.num * work_index) / v_.num_works;
				UINT index_end = (v_.particles.num * (work_index + 1)) / v_.num_works;
				o_wd_->ReserveWork([this, index_begin, index_end]() {_SolveCollision(index_begin, index_end); });
			}
			o_wd_->ExecuteWork();

			for (UINT work_index = 0; work_index < v_.num_works; work_index++) {
				UINT index_begin = (v_.particles.num * work_index) / v_.num_works;
				UINT index_end = (v_.particles.num * (work_index + 1)) / v_.num_works;
				o_wd_->ReserveWork([this, index_begin, index_end]() {_EmptyVoxel(index_begin, index_end); });
			}
			o_wd_->ExecuteWork();

			for (UINT work_index = 0; work_index < v_.num_works; work_index++) {
				UINT index_begin = (v_.particles.num * work_index) / v_.num_works;
				UINT index_end = (v_.particles.num * (work_index + 1)) / v_.num_works;
				o_wd_->ReserveWork([this, index_begin, index_end]() {_UpdateVoxel(index_begin, index_end); });
			}
			o_wd_->ExecuteWork();
		}
	}
	void VoxelParticleSystem::Terminate() {}

	UINT VoxelParticleSystem::get_particles_num() {
		return v_.particles.num;
	}
	UINT VoxelParticleSystem::get_particles_pos_buffer_size() {
		return sizeof(vmath::vec3) * v_.particles.num;
	}
	void* VoxelParticleSystem::get_particles_pos() {
		return data_.particles.pos_curr.data();
	}
	void* VoxelParticleSystem::get_density_array() {
		//::Last Edited At 2020-08-24-08:24 by STH.

		for (UINT work_index = 0; work_index < v_.num_works; ++work_index) {
			UINT index_begin = (v_.voxel.p.num * work_index) / v_.num_works;
			UINT index_end = (v_.voxel.p.num * (work_index + 1)) / v_.num_works;
			o_wd_->ReserveWork([this, index_begin, index_end]() {_UpdateDensityArray(index_begin, index_end); });
		}
		o_wd_->ExecuteWork();

		return (void*)data_.voxel.density.data();
	}

	//::Init
	void VoxelParticleSystem::_InitSearchOffset() {
		//::Last Edited At 2020-08-24-08:24 by STH.

		//::V
		std::multimap<FLOAT, INT> temp_map;

		INT pot_valid_range;

		FLOAT rep_v;
		FLOAT rep_x;
		FLOAT rep_y;
		FLOAT rep_z;
		vmath::vec3 rep_vec3;
		FLOAT rep_dist;
		INT rep_offset;

		FLOAT max_dist;

		//::L
		rep_v = 1.0f;
		max_dist = v_.particles.max_intr_distance;
		pot_valid_range = (INT)ceilf(max_dist / v_.voxel.unit);

		for (INT z = -pot_valid_range; z <= pot_valid_range; ++z) {
			for (INT y = -pot_valid_range; y <= pot_valid_range; ++y) {
				for (INT x = -pot_valid_range; x <= pot_valid_range; ++x) {
					rep_x = x == 0 ? 0.0f : (x > 0 ? (FLOAT)x - rep_v : (FLOAT)x + rep_v);
					rep_y = y == 0 ? 0.0f : (y > 0 ? (FLOAT)y - rep_v : (FLOAT)y + rep_v);
					rep_z = z == 0 ? 0.0f : (z > 0 ? (FLOAT)z - rep_v : (FLOAT)z + rep_v);

					rep_vec3 = { rep_x,rep_y,rep_z };
					rep_vec3 *= v_.voxel.unit;
					rep_dist = vmath::length(rep_vec3);

					if (rep_dist < max_dist) {
						rep_offset = x + y * v_.voxel.scale_x + z * v_.voxel.p.scale_xy;
						temp_map.insert(std::make_pair(rep_dist, rep_offset));
					}
				}
			}
		}
		std::vector<INT>().swap(v_.voxel.p.search_offsets);
		INT temp;
		for (auto iter = temp_map.begin(); iter != temp_map.end(); ++iter) {
			if ((temp = iter->second) != 0)
				v_.voxel.p.search_offsets.emplace_back(temp);
		}
	}
	void VoxelParticleSystem::_InitSearchOffset2() {
		//::Last Edited At 2020-08-24-08:24 by STH.

		//::V
		std::multimap<FLOAT, INT> temp_map;

		INT pot_valid_range;

		FLOAT rep_v;
		FLOAT rep_x;
		FLOAT rep_y;
		FLOAT rep_z;
		vmath::vec3 rep_vec3;
		FLOAT rep_dist;
		INT rep_offset;

		FLOAT max_dist;

		//::L
		rep_v = 0.5f;
		max_dist = v_.voxel.density.p.max_intr_distance;

		pot_valid_range = (INT)ceilf((max_dist / v_.voxel.unit) - 0.5f);

		for (INT z = -pot_valid_range; z <= pot_valid_range; ++z) {
			for (INT y = -pot_valid_range; y <= pot_valid_range; ++y) {
				for (INT x = -pot_valid_range; x <= pot_valid_range; ++x) {
					rep_x = x == 0 ? 0.0f : (x > 0 ? (FLOAT)x - rep_v : (FLOAT)x + rep_v);
					rep_y = y == 0 ? 0.0f : (y > 0 ? (FLOAT)y - rep_v : (FLOAT)y + rep_v);
					rep_z = z == 0 ? 0.0f : (z > 0 ? (FLOAT)z - rep_v : (FLOAT)z + rep_v);

					rep_vec3 = { rep_x,rep_y,rep_z };
					rep_vec3 *= v_.voxel.unit;
					rep_dist = vmath::length(rep_vec3);

					if (rep_dist < max_dist) {
						rep_offset = x + y * v_.voxel.scale_x + z * v_.voxel.p.scale_xy;
						temp_map.insert(std::make_pair(rep_dist, rep_offset));
					}
				}
			}
		}
		std::vector<INT>().swap(v_.voxel.p.search_offsets_2);
		for (auto iter = temp_map.begin(); iter != temp_map.end(); ++iter) {
			v_.voxel.p.search_offsets_2.emplace_back(iter->second);
		}
	}

	//::Startup
	void VoxelParticleSystem::_SetDefaultData() {
		//::2020-08-24-04:43 STH

		//::V
		UINT cbrt_par_num = (UINT)ceilf(cbrtf((FLOAT)v_.particles.num));
		UINT cbrt_par_num_sq = cbrt_par_num * cbrt_par_num;
		UINT temp_x;
		UINT temp_y;
		UINT temp_z;
		vmath::vec3 temp_vec3;

		//::L
		vmath::vec3 array_center =
			vmath::vec3(1.0f, 1.0f, 1.0f) * (FLOAT)(cbrt_par_num - 1) * 0.5f * v_.particles.diameter * v_.particles.interval;

		for (UINT index = 0; index < v_.particles.num; ++index) {
			temp_x = (UINT)(index % cbrt_par_num);
			temp_y = (UINT)((index / cbrt_par_num) % cbrt_par_num);
			temp_z = (UINT)(index / cbrt_par_num_sq);

			temp_vec3 = { (FLOAT)temp_x , (FLOAT)temp_y, (FLOAT)temp_z };
			temp_vec3 *= v_.particles.interval * v_.particles.diameter;
			temp_vec3 -= array_center;
			temp_vec3 += v_.voxel.pos_center;

			data_.particles.pos_curr[index] = temp_vec3;
			data_.particles.pos_past[index] = temp_vec3;
			data_.particles.vel_curr[index] = { 0.0f, 0.0f, 0.0f };
			data_.particles.vel_past[index] = { 0.0f, 0.0f, 0.0f };
		}
	}

	//::Control

	//::Update
	void VoxelParticleSystem::_UpdateParticles(UINT index_begin, UINT index_end) {
		//::2020-08-26-09:00 STH

		//::A Cheack Parameter
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::B
		vmath::vec3 pos_subject;

		UINT idx_subject;
		UINT idx_offsets;
		UINT idx_target;

		UINT v_adrs_subject;
		UINT v_adrs_target;

		vmath::vec3 temp_vec3;
		UINT temp_x;
		UINT temp_y;
		UINT temp_z;

		vmath::vec3 total_force_t;

		vmath::vec3 rel_pos;
		FLOAT rel_distance;
		vmath::vec3 rel_nor;

		//::C
		for (idx_subject = index_begin; idx_subject < index_end; idx_subject++) {
			total_force_t = c_.p.g_gravity_t;
			pos_subject = data_.particles.pos_past[idx_subject];
			temp_vec3 = pos_subject - v_.voxel.p.pos_origin;
			temp_x = (UINT)(temp_vec3[0] * v_.voxel.p.unit_recip);
			temp_y = (UINT)(temp_vec3[1] * v_.voxel.p.unit_recip);
			temp_z = (UINT)(temp_vec3[2] * v_.voxel.p.unit_recip);

			v_adrs_subject = temp_x;
			v_adrs_subject += temp_y * v_.voxel.scale_x;
			v_adrs_subject += temp_z * v_.voxel.p.scale_xy;

			for (idx_offsets = 0; idx_offsets < v_.voxel.p.search_offsets.size(); ++idx_offsets) {
				v_adrs_target = v_adrs_subject + v_.voxel.p.search_offsets[idx_offsets];

				if (0 <= v_adrs_target && v_adrs_target < v_.voxel.p.num) {
					idx_target = data_.voxel.contents[v_adrs_target];
					if (idx_target != EMPTY) {
						rel_pos = data_.particles.pos_past[idx_target] - pos_subject;
						rel_distance = vmath::length(rel_pos);
						rel_nor = rel_pos / rel_distance;
						if (rel_distance <= v_.particles.max_intr_distance) {
							total_force_t += rel_nor * c_.particle_coef.p.spring_t * (rel_distance - v_.particles.diameter) * v_.particles.p.diameter_recip;
							if (vmath::dot(rel_nor, c_.p.g_gravity_t) >= 0.0f) {
								total_force_t += c_.p.g_gravity_t * -0.0833333;
							}
						}
					}
				}
				data_.particles.vel_curr[idx_subject] += total_force_t;
				data_.particles.vel_curr[idx_subject] *= c_.p.g_reduction_t;
				data_.particles.pos_curr[idx_subject] = data_.particles.pos_past[idx_subject] + data_.particles.vel_curr[idx_subject];
			}
		}
	}
	void VoxelParticleSystem::_UpdateParticles2(UINT index_begin, UINT index_end) {
		//A Cheack Parameter
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::B
		vmath::vec3 pos_subject;
		vmath::vec3 pos_target;
		vmath::vec3 rand_vec3;
		//vmath::vec3 vel_subject;
		//vmath::vec3 vel_target;

		UINT idx_subject;
		UINT idx_offsets;
		UINT idx_elements;
		UINT idx_target;

		UINT v_adrs_subject;
		UINT v_adrs_target;

		//UINT e_adrs_subject;
		UINT e_adrs_target;

		vmath::vec3 temp_vec3;
		UINT temp_x;
		UINT temp_y;
		UINT temp_z;

		vmath::vec3 total_force_tsqm;
		vmath::vec3 total_force;

		vmath::vec3 rel_pos;
		FLOAT rel_distance;
		vmath::vec3 rel_nor;
		vmath::vec3 rel_vel;
		vmath::vec3 rel_tan;

		//Code
		for (idx_subject = index_begin; idx_subject < index_end; idx_subject++) {
			//total_force_tsqm = c_.p.g_gravity_tsqm;
			total_force = c_.g_gravity;
			//srand(idx_subject * time(NULL));
			//rand_vec3 = vmath::random<vmath::vec3>();
			//total_force_tsqm += (rand_vec3 / vmath::length(rand_vec3))*0.000005;

			pos_subject = data_.particles.pos_past[idx_subject];
			temp_vec3 = pos_subject - v_.voxel.p.pos_origin;
			//temp_vec3 = pos_subject;
			temp_x = (UINT)(temp_vec3[0] * v_.voxel.p.unit_recip);
			temp_y = (UINT)(temp_vec3[1] * v_.voxel.p.unit_recip);
			temp_z = (UINT)(temp_vec3[2] * v_.voxel.p.unit_recip);

			v_adrs_subject = temp_x;
			v_adrs_subject += temp_y * v_.voxel.scale_x;
			v_adrs_subject += temp_z * v_.voxel.p.scale_xy;

			for (idx_offsets = 0; idx_offsets < v_.voxel.p.search_offsets.size(); idx_offsets++) {
				v_adrs_target = v_adrs_subject + v_.voxel.p.search_offsets[idx_offsets];

				if (0 <= v_adrs_target && v_adrs_target < v_.voxel.p.num) {
					e_adrs_target = v_adrs_target * v_.voxel.num_elements;
					//v_adrs_target *= v_.voxel.num_elements;
					for (idx_elements = 0; idx_elements < v_.voxel.num_elements; idx_elements++, e_adrs_target++) {
						idx_target = data_.voxel.contents[e_adrs_target];
						if (idx_target == EMPTY) {
							break;
						}
						else if (idx_target != idx_subject) {
							pos_target = data_.particles.pos_past[idx_target];
							rel_pos = pos_target - pos_subject;
							rel_distance = vmath::length(rel_pos);
							if (rel_distance <= v_.particles.max_intr_distance) {
								rel_nor = rel_pos / rel_distance;
								if (rel_distance <= v_.particles.diameter) {
									//rel_vel = data_.particles.vel_past[idx_target] - data_.particles.vel_past[idx_subject];
									//rel_tan = rel_vel - (vmath::dot(rel_vel, rel_nor) * rel_nor);

									total_force += rel_nor * c_.particle_coef.spring * (rel_distance - v_.particles.diameter) / v_.particles.diameter;
									//total_force += rel_vel * c_.particle_coef.damping;
									//total_force += rel_tan * c_.particle_coef.shear;

									//if (vmath::dot(rel_nor, c_.g_gravity_) >= 0.0f) {
										//total_force += c_.g_gravity_ * -0.0833333;
									//}
									//total_force_tsqm += rel_nor * c_.particle_coef.p.spring_tsqm * (rel_distance - v_.unit);
									//total_force_tsqm += rel_vel * c_.particle_coef.p.damping_tsqm;
									//total_force_tsqm += rel_tan * c_.particle_coef.p.shear_tsqm;
								}
								else {
									//total_force_tsqm += rel_nor * c_.particle_coef.p.attraction_tsqm;
									total_force += rel_nor * c_.particle_coef.attraction * (rel_distance - v_.particles.diameter) / (v_.particles.max_intr_distance - v_.particles.diameter);

								}
							}
						}
					}
				}
			}
			//total_force *= v_.particles.p.mass_recip;
			total_force *= v_.tick;

			data_.particles.vel_curr[idx_subject] += total_force;
			data_.particles.vel_curr[idx_subject] *= 0.95f;
			data_.particles.pos_curr[idx_subject] = data_.particles.pos_past[idx_subject] + data_.particles.vel_curr[idx_subject];

			//temp_vec3 = data_.particles.pos_curr[idx_subject];

			//data_.particles.pos_curr[idx_subject] = 2.0f * data_.particles.pos_past[idx_subject] - temp_vec3 + total_force_tsqm;
			//data_.particles.vel_curr[idx_subject] = data_.particles.pos_curr[idx_subject] - data_.particles.pos_past[idx_subject];
			//data_.particles.vel_curr[idx_subject] *= v_.p.tick_recip;
		}
	}
	void VoxelParticleSystem::_SolveCollision(UINT index_begin, UINT index_end) {
		//::A Cheack Parameter
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::
		FLOAT temp_length;
		UINT idx_subject;
		vmath::vec3 normal;

		//임시코드
		for (idx_subject = index_begin; idx_subject < index_end; idx_subject++) {
			temp_length = vmath::length(data_.particles.pos_curr[idx_subject]);
			if (temp_length > v_.p.limit_c) {
				normal = data_.particles.pos_curr[idx_subject] / temp_length;
				data_.particles.pos_curr[idx_subject] = v_.p.limit_c * normal;


				//data_.particles.vel_curr[idx_subject] = vmath::reflect(data_.particles.vel_curr[idx_subject], normal);
				//data_.particles.vel_curr[idx_subject] *= c_.particle_coef.spring;
				//data_.particles.pos_past[idx_subject] =
					//data_.particles.pos_curr[idx_subject] - c_.particle_coef.spring * data_.particles.vel_curr[idx_subject];
				//	data_.particles.pos_curr[idx_subject] - 0.5f * data_.particles.vel_curr[idx_subject];
			}
		}
	}
	void VoxelParticleSystem::_EmptyVoxel(UINT index_begin, UINT index_end) {
		//::Last Edited At 2020-08-24-05:26 by STH.

		//::Otimization Target.

		//::C
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::V
		UINT index_subject;
		UINT v_adrs_subject;

		//::L
		for (index_subject = index_begin; index_subject < index_end; ++index_subject) {
			v_adrs_subject = data_.voxel.registered[index_subject];
			if (v_adrs_subject != EMPTY) {
				data_.voxel.contents[v_adrs_subject] = NOT_REGISTERED;
			}
		}
	}
	void VoxelParticleSystem::_UpdateVoxel(UINT index_begin, UINT index_end) {
		//::Last Edited At 2020-08-24-05:22 by STH.

		//::Otimization Target.

		//::C
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::V
		vmath::vec3 pos_subject;

		UINT idx_subject;
		UINT idx_elements;
		UINT idx_temp;

		UINT v_adrs_subject;
		UINT e_adrs_subject;

		vmath::vec3 temp_vec3;
		UINT temp_x;
		UINT temp_y;
		UINT temp_z;

		//::L
		for (idx_subject = index_begin; idx_subject < index_end; ++idx_subject) {
			pos_subject = data_.particles.pos_curr[idx_subject];
			temp_vec3 = pos_subject - v_.voxel.p.pos_origin;
			temp_x = (UINT)(temp_vec3[0] * v_.voxel.p.unit_recip);
			temp_y = (UINT)(temp_vec3[1] * v_.voxel.p.unit_recip);
			temp_z = (UINT)(temp_vec3[2] * v_.voxel.p.unit_recip);

			v_adrs_subject = temp_x;
			v_adrs_subject += temp_y * v_.voxel.scale_x;
			v_adrs_subject += temp_z * v_.voxel.p.scale_xy;
			e_adrs_subject = v_adrs_subject * v_.voxel.num_elements;

			for (idx_elements = 0; idx_elements < v_.voxel.num_elements; ++idx_elements, ++e_adrs_subject) {
				idx_temp = data_.voxel.contents[e_adrs_subject];
				if (idx_temp == NOT_REGISTERED) {
					data_.voxel.contents[e_adrs_subject] = idx_subject;
					data_.voxel.registered[idx_subject] = e_adrs_subject;
					break;
				}
			}
		}
	}

	//::
	void VoxelParticleSystem::_UpdateDensityArray(UINT index_begin, UINT index_end) {
		//::Last Edited At 2020-08-24-08:24 by STH.

		//::Top Priority Otimization Target.

		//::C
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::V
		FLOAT total_density;

		UINT idx_offsets;
		UINT idx_elements;
		UINT idx_target;

		UINT temp_x;
		UINT temp_y;
		UINT temp_z;

		vmath::vec3 pos_subject_voxel_center;
		vmath::vec3 pos_target_particle;

		UINT v_adrs_subject;
		UINT v_adrs_target;
		UINT e_adrs_target;

		vmath::vec3 rel_pos;
		FLOAT rel_distance;

		//::L
		for (v_adrs_subject = index_begin; v_adrs_subject < index_end; ++v_adrs_subject) {
			temp_x = v_adrs_subject % v_.voxel.scale_x;
			temp_y = v_adrs_subject / v_.voxel.scale_x % v_.voxel.scale_y;
			temp_z = v_adrs_subject / v_.voxel.p.scale_xy;
			pos_subject_voxel_center = { (FLOAT)temp_x + 0.5f,(FLOAT)temp_y + 0.5f ,(FLOAT)temp_z + 0.5f };
			pos_subject_voxel_center *= v_.voxel.unit;
			pos_subject_voxel_center += v_.voxel.p.pos_origin;

			if (vmath::length(pos_subject_voxel_center) <= v_.limit) {
				continue;
			}

			total_density = 0.0f;
			for (idx_offsets = 0; idx_offsets < v_.voxel.p.search_offsets_2.size() && total_density < 1.0f; ++idx_offsets) {
				v_adrs_target = v_adrs_subject + v_.voxel.p.search_offsets_2[idx_offsets];

				if (0 <= v_adrs_target && v_adrs_target < v_.voxel.p.num) {
					e_adrs_target = v_adrs_target * v_.voxel.num_elements;
					for (idx_elements = 0; idx_elements < v_.voxel.num_elements; ++idx_elements, ++e_adrs_target) {
						idx_target = data_.voxel.contents[e_adrs_target];
						if (idx_target == EMPTY) {
							break;
						}
						pos_target_particle = data_.particles.pos_past[idx_target];
						rel_pos = pos_target_particle - pos_subject_voxel_center;
						rel_distance = vmath::length(rel_pos);

						if (rel_distance <= v_.voxel.density.intr_distance) {
							total_density += 1.0f;
							break;
						}
						else if (rel_distance <= v_.voxel.density.p.max_intr_distance) {
							total_density += (rel_distance - v_.voxel.density.intr_distance) * v_.voxel.density.p.intr_offset_recip;
						}
					}
				}
			}
			data_.voxel.density[v_adrs_subject] = total_density;
		}
	}
	void VoxelParticleSystem::_UpdateDensityArray2(UINT index_begin, UINT index_end) {
		//::Last Edited At 2020-08-24-08:24 by STH.

		//::Top Priority Otimization Target.

		//::C
		if (index_end <= index_begin || index_end == 0) {
			return;
		}

		//::V
		FLOAT total_density;

		UINT idx_offsets;
		UINT idx_elements;
		UINT idx_target;

		UINT temp_x;
		UINT temp_y;
		UINT temp_z;

		vmath::vec3 pos_subject_voxel_center;
		vmath::vec3 pos_target_particle;

		UINT v_adrs_subject;
		UINT v_adrs_target;
		UINT e_adrs_target;

		vmath::vec3 rel_pos;
		FLOAT rel_distance;

		//::임시
		rel_distance = 1.0f;


		//::L
		for (v_adrs_subject = index_begin; v_adrs_subject < index_end; ++v_adrs_subject) {
			temp_x = v_adrs_subject % v_.voxel.scale_x;
			temp_y = v_adrs_subject / v_.voxel.scale_x % v_.voxel.scale_y;
			temp_z = v_adrs_subject / v_.voxel.p.scale_xy;
			pos_subject_voxel_center = { (FLOAT)temp_x + 0.5f,(FLOAT)temp_y + 0.5f ,(FLOAT)temp_z + 0.5f };
			pos_subject_voxel_center *= v_.voxel.unit;
			pos_subject_voxel_center += v_.voxel.p.pos_origin;

			if (rel_distance <= v_.limit) {
				continue;
			}

			total_density = 0.0f;
			for (idx_offsets = 0; idx_offsets < v_.voxel.p.search_offsets_2.size() && total_density < 1.0f; ++idx_offsets) {
				v_adrs_target = v_adrs_subject + v_.voxel.p.search_offsets_2[idx_offsets];

				if (0 <= v_adrs_target && v_adrs_target < v_.voxel.p.num) {
					e_adrs_target = v_adrs_target * v_.voxel.num_elements;
					for (idx_elements = 0; idx_elements < v_.voxel.num_elements; ++idx_elements, ++e_adrs_target) {
						idx_target = data_.voxel.contents[e_adrs_target];
						if (idx_target == EMPTY) {
							break;
						}
						pos_target_particle = data_.particles.pos_past[idx_target];
						rel_pos = pos_target_particle - pos_subject_voxel_center;
						rel_distance = vmath::length(rel_pos);

						if (rel_distance <= v_.voxel.density.intr_distance) {
							total_density += 1.0f;
							break;
						}
						else if (rel_distance <= v_.voxel.density.p.max_intr_distance) {
							total_density += (rel_distance - v_.voxel.density.intr_distance) * v_.voxel.density.p.intr_offset_recip;
						}
					}
				}
			}
			data_.voxel.density[v_adrs_subject] = total_density;
		}
	}
}
#pragma once

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>		
#include <GLFW/glfw3.h> 
//::
#include "VoxelParticleSystem.h"
#include "Camera.h"

#include "vmath.h"

namespace GS {
	struct {
		const GLfloat black[3] = { 0.0f, 0.0f, 0.0f };
		const GLfloat gray[3] = { 0.5f, 0.5f, 0.5f };
		const GLfloat white[3] = { 1.0f, 1.0f, 1.0f };
		const GLfloat red[3] = { 1.0f, 0.0f, 0.0f };
		const GLfloat green[3] = { 0.0f, 1.0f, 0.0f };
		const GLfloat blue[3] = { 0.0f, 0.0f, 1.0f };
		const GLfloat yellow[3] = { 1.0f, 1.0f, 0.0f };
		const GLfloat magenta[3] = { 1.0f, 0.0f, 1.0f };
		const GLfloat cyan[3] = { 0.0f, 1.0f, 1.0f };
	}colors_;

	struct ICOSHEDRON {
		const FLOAT ex0 = 0.52573110f * 0.5f;
		const FLOAT ex1 = 0.85065081f * 0.5f;

		FLOAT vertices[36] = {
			-ex1, +ex0, 0.0f,
			+ex1, +ex0, 0.0f,
			-ex1, -ex0, 0.0f,
			+ex1, -ex0, 0.0f,

			0.0f, -ex1, +ex0,
			0.0f, +ex1, +ex0,
			0.0f, -ex1, -ex0,
			0.0f, +ex1, -ex0,

			+ex0, 0.0f, -ex1,
			+ex0, 0.0f, +ex1,
			-ex0, 0.0f, -ex1,
			-ex0, 0.0f, +ex1
		};
		INT indices[60] = {
			0, 11, 5,
			0, 5, 1,
			0, 1, 7,
			0, 7, 10,
			0, 10, 11,
			1, 5, 9,
			5, 11, 4,
			11, 10, 2,
			10, 7, 6,
			7, 1, 8,
			3, 9, 4,
			3, 4, 2,
			3, 2, 6,
			3, 6, 8,
			3, 8, 9,
			4, 9, 5,
			2, 4, 11,
			6, 2, 10,
			8, 6, 7,
			9, 8, 1
		};

		VOID Init(FLOAT diameter) {
			for (int i = 0; i < 36; i++) {
				vertices[i] *= diameter;
			}
		}
	};
}
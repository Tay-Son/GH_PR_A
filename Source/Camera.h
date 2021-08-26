#pragma once

#include "vmath.h"

#include "interface.h"

namespace GS {
	class Camera {
	public:
		Camera() {};
		~Camera() {};

		void Init() {}
		void Startup() {}
		void Control() {}
		void Update() {}
		void Terminate() {}
	};
}
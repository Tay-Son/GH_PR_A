#pragma once
#include "Ballpit.h"
// 메인 진입 함수
int main() {
	BallPit* app = new BallPit;
	app->run(app);
	delete app;
	return 0;
}
#pragma once
#include "Ballpit.h"
// ���� ���� �Լ�
int main() {
	BallPit* app = new BallPit;
	app->run(app);
	delete app;
	return 0;
}
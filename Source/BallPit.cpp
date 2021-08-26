#pragma once
#include "Ballpit.h"

void BallPit::Init()
{
	GLApp::Init();
	printf("DEBUG::Init\n");

	InitTitle();
	InitBenchmark();
	InitVI();
	InitGravity();
	InitBalls();
	InitVoxels();
}

void BallPit::Startup()
{
	LoadProgram();

	glGenBuffers(1, &mPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertex_positions),
		vertex_positions,
		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(vertex_indices),
		vertex_indices,
		GL_STATIC_DRAW);

	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glGenBuffers(1, &mBPBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBPBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vec3) * BALL_NUM,
		NULL,
		GL_DYNAMIC_READ
	);
}

void BallPit::Update() {
	UpdateTime();
	UpdateGravity();
	UpdateBalls();
	UpdateVoxels();
	UpdateBenchmark();
}

void BallPit::Render()
{
	static const GLfloat backgrounColor[] = { .8f, 0.8f, 0.8f, 1.0f };
	static const GLfloat one = 1.0f;
	GLuint Tick = mTick % 2;

	glViewport(0, 0, info.windowWidth, info.windowHeight);
	glClearBufferfv(GL_COLOR, 0, backgrounColor);
	glClearBufferfv(GL_DEPTH, 0, &one);

	glUseProgram(mMainProgram);

	vmath::mat4 proj_matrix = vmath::perspective(30.0f,
		(float)info.windowWidth / (float)info.windowHeight,
		0.1f,
		1000.0f
	);

	glUniformMatrix4fv(mPJLocation, 1, GL_FALSE, proj_matrix);

	mCameraPos = vmath::vec3(0.0f, 0.0f, 10.0f);
	vmath::vec3 up = vmath::vec3(0.0f, 1.0f, 0.0f);

	vmath::mat4 wcTr = vmath::mat4::identity();
	wcTr *= vmath::translate(mCameraPos);
	wcTr *= vmath::rotate(-11.0f, vec3(1.0f, 0.0f, 0.0f));

	vmath::vec3 dir = vmath::vec3(0.0f, 0.0f, -1.0f);
	vmath::quaternion q = vmath::quaternion(45.0f, vmath::vec3(0.0f, 1.0f, 0.0f));

	vmath::vec3 rx = vec3(wcTr[0][0], wcTr[0][1], wcTr[0][2]);
	vmath::vec3 ry = vec3(wcTr[1][0], wcTr[1][1], wcTr[1][2]);
	vmath::vec3 rz = vec3(wcTr[2][0], wcTr[2][1], wcTr[2][2]);
	vmath::vec3 tv = vec3(wcTr[3][0], wcTr[3][1], wcTr[3][2]);

	vmath::mat4 cwTr = {
		vec4(rx,-vmath::dot(rx,tv)),
		vec4(ry,-vmath::dot(ry,tv)),
		vec4(rz,-vmath::dot(rz,tv)),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	cwTr = cwTr.transpose();

	//for (GLuint i = 0; i < BALL_NUM; i++)
	for (GLuint i = 0; i < 1; i++)
	{
		GLfloat t = (GLfloat)glfwGetTime();
		float f = (float)i + t * 0.3f;

		vmath::mat4 woTr = vmath::mat4::identity();
		woTr *= translate(mBallPos[Tick][i]);
		vmath::mat4 coTr = cwTr * woTr;
		vmath::mat4 mv_matrix = translate(0.0f, 0.0f, -5.0f) *
			vmath::rotate(t * 45.0f, 0.0f, 1.0f, 0.0f) *
			vmath::rotate(t * 21.0f, 1.0f, 0.0f, 0.0f) *
			vmath::translate(sinf(2.1f * f) * 2.0f,
				cosf(1.7f * f) * 2.0f,
				sinf(1.3f * f) * cosf(1.5f * f) * 2.0f);

		glUniformMatrix4fv(mMVLocation, 1, GL_FALSE, coTr);
		glUniform3fv(mUniform.diffuseAlbedo, 1, gBallColors[i % 9]);
		glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_SHORT, 0);
	}
	mTick++;
}

void BallPit::Shutdown()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteProgram(mMainProgram);
	glDeleteBuffers(1, &mPositionBuffer);
}

void BallPit::LoadProgram() {
	mMainProgram = glCreateProgram();

	GLuint fs = LoadShader("Shader/fs.glsl", GL_FRAGMENT_SHADER);
	GLuint vs = LoadShader("Shader/vs.glsl", GL_VERTEX_SHADER);

	glAttachShader(mMainProgram, fs);
	glAttachShader(mMainProgram, vs);

	glLinkProgram(mMainProgram);

	glDeleteShader(fs);
	glDeleteShader(vs);

	mMVLocation = glGetUniformLocation(mMainProgram, "mv_matrix");
	mPJLocation = glGetUniformLocation(mMainProgram, "proj_matrix");

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	mUniform.diffuseAlbedo = glGetUniformLocation(mMainProgram, "diffuse_albedo");
	mUniform.specularAlbedo = glGetUniformLocation(mMainProgram, "specular_albedo");
	mUniform.specularPower = glGetUniformLocation(mMainProgram, "specular_power");
}

void BallPit::InitVI() {
	int index = 0;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				mVI[index] =
					i * VOXEL_CELL * VOXEL_CELL +
					j * VOXEL_CELL +
					k;
				index++;
			}
		}
	}
}
void BallPit::InitTime() {
	QueryPerformanceCounter(&mCurrTime);
	QueryPerformanceFrequency(&mPreq);
	mPrevTime = mCurrTime;
	mDeltaTime = .05f;
}
void BallPit::InitGravity() {
	mGravity = GRAVITY;
}
void BallPit::InitBalls() {
	mTick = 0;

	for (int i = 0; i < BALL_NUM; i++) {
		mBallPos[0][i] = vec3(
			(GLfloat)(i % BALL_CELL - (BALL_CELL / 2)) * BALL_UNIT,
			(GLfloat)(i / (BALL_CELL * BALL_CELL) - (BALL_CELL / 2)) * BALL_UNIT + 1.0f,
			(GLfloat)(i / BALL_CELL % BALL_CELL - (BALL_CELL / 2)) * BALL_UNIT
		);
		mBallVel[0][i] = vec3(0.0f, 0.0f, 0.0f);
	}
	mTick++;
}
void BallPit::InitVoxels() {
	memset(mVoxel, 255, VOXEL_CELL * VOXEL_CELL * VOXEL_CELL * VOXEL_LEAF * sizeof(GLushort));
	for (int i = 0; i < BALL_NUM; i++) {
		GLuint idx =
			(int)((mBallPos[0][i][0] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL * VOXEL_CELL +
			(int)((mBallPos[0][i][1] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL +
			(int)((mBallPos[0][i][2] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT);

		for (int j = 0; j < VOXEL_LEAF; j++) {
			if (mVoxel[idx][j] == USHRT_MAX) {
				mVoxel[idx][j] = i;
				break;
			}
		}
	}
}
void BallPit::InitTitle() {
	static char title[50] = "Ball Pit";
	memcpy(info.title, title, sizeof(title));
}
void BallPit::InitBenchmark() {
	for (int i = 0; i < 5; i++) {
		mBench[i] = 0.0f;
	}
	mBenchmark = 0.0f;
}

void BallPit::UpdateBenchmark() {
	//성능 체크용 디버그 함수
	GLuint Tick = mTick % 5;
	mBench[Tick] = mDeltaTime;

	GLfloat sum = 0;
	for (int i = 0; i < 5; i++) {
		sum += mBench[i];
	}
	sum /= 5.0f;

	mBenchmark = 1.0f / sum;
	if (mTick % 10 == 0) {
		printf("DEBUG::%06.2f\n", mBenchmark);
	}
}
void BallPit::UpdateTime() {
	// 애니메이션용 델타타임 함수
	mPrevTime = mCurrTime;
	QueryPerformanceCounter(&mCurrTime);
	QueryPerformanceFrequency(&mPreq);
	mDeltaTime = ((GLfloat)(mCurrTime.QuadPart - mPrevTime.QuadPart) / (GLfloat)mPreq.QuadPart);
	if (mDeltaTime > .05f)
		mDeltaTime = .05f;
}
void BallPit::UpdateGravity() {
	//중력 업데이트 함수
	mGravity = GRAVITY;
	if (mIs_SPC_Clicked) {
		mGravity = vec3(0.0f, 0.0f, 0.0f);
	}

	if (mIs_RMB_Clicked) {
		vec2 cv = mMouseCurrPos - mMouseAnchPos;
		GLfloat len = length(cv);
		if (len > 300.0f) {
			len = 300.f;
			cv = len * normalize(cv);
		}
		mGravity += (vec3(cv[0], 0.0f, cv[1]) / 300.0f) * GRAVITY_MAG;
	}
	
	vec3 rv = vec3::random();
	rv = normalize(rv);
	rv -= vec3(0.5f, 0.5f, 0.5f);
	rv *= 0.005;
	mGravity += rv;
	
}
void BallPit::UpdateBalls() {
	//Ball 업데이트 함수
	GLuint Tick = mTick % 2;
	GLuint CTick = (mTick + 1) % 2;

	CollideBalls(0, BALL_NUM);
	if (mIs_LMB_Clicked) {
		static GLuint cnt = 0;
		GLushort pcnt = cnt % BALL_NUM;
		cnt++;
	}
}
void BallPit::UpdateVoxels() {
	//Voxel 업데이트 함수
	GLuint Tick = mTick % 2;
	GLuint CTick = (mTick + 1) % 2;

	memset(mVoxel, 255, VOXEL_CELL * VOXEL_CELL * VOXEL_CELL * VOXEL_LEAF * sizeof(GLushort));
	for (int i = 0; i < BALL_NUM; i++) {
		GLuint idx =
			(int)((mBallPos[Tick][i][0] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL * VOXEL_CELL +
			(int)((mBallPos[Tick][i][1] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL +
			(int)((mBallPos[Tick][i][2] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT);

		for (int j = 0; j < VOXEL_LEAF; j++) {
			if (mVoxel[idx][j] == USHRT_MAX) {
				mVoxel[idx][j] = i;
				break;
			}
		}
	}
}

void BallPit::CollideBalls(GLuint start, GLuint offset) {
	//Ball 충돌 처리 함수
	GLuint Tick = mTick % 2;
	GLuint CTick = (mTick + 1) % 2;

	for (int i = start; i < start + offset; i++) {
		GLuint bn =
			(int)((mBallPos[CTick][i][0] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL * VOXEL_CELL +
			(int)((mBallPos[CTick][i][1] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT) * VOXEL_CELL +
			(int)((mBallPos[CTick][i][2] + (VOXEL_UNIT * (GLfloat)(VOXEL_CELL / 2))) / VOXEL_UNIT);

		vec3 force = vec3(0.0f, 0.0f, 0.0f);
		force += mGravity;

		for (int u = 0; u < 27; u++) {
			GLuint idx = bn + mVI[u];
			if (0 <= idx && idx < VOXEL_CELL * VOXEL_CELL * VOXEL_CELL) {
				for (int j = 0; j < VOXEL_LEAF; j++) {
					if (mVoxel[idx][j] == USHRT_MAX) {
						break;
					}
					else if (mVoxel[idx][j] != i) {
						GLushort bpidx = mVoxel[idx][j];

						vec3 relPos = mBallPos[CTick][bpidx] - mBallPos[CTick][i];


						GLfloat distance = length(relPos);
						GLfloat collideDist = BALL_UNIT;

						if (distance < collideDist)
						{
							vec3 normal = relPos / distance;
							vec3 relVel = mBallVel[CTick][bpidx] - mBallVel[CTick][i];
							vec3 tanVel = relVel - (vmath::dot(relVel, normal) * normal);
							force += -BALL_SPRING * (collideDist - distance) * normal;
							force += BALL_DAMPING * relVel;
							force += BALL_SHEAR * tanVel;
						
							if (dot(normal, mGravity) >= 0.0f) {
								force += -mGravity * 0.25;
							}
						}
					}
				}
			}
		}

		force *= mDeltaTime;
		mBallVel[Tick][i] = mBallVel[CTick][i] + force;
		mBallPos[Tick][i] = mBallPos[CTick][i] + mBallVel[Tick][i];

		if (mBallPos[Tick][i][1] - BALL_UNIT * .5f < CYLINDER_FLOOR) {
			mBallPos[Tick][i][1] = CYLINDER_FLOOR + BALL_UNIT * .5f;
			mBallVel[Tick][i][1] = -mBallVel[Tick][i][1] * BALL_SPRING;
		}

		if (mBallPos[Tick][i][1] + BALL_UNIT * .5f > CYLINDER_CEILING) {
			mBallPos[Tick][i][1] = CYLINDER_CEILING - BALL_UNIT * .5f;
			mBallVel[Tick][i][1] = -mBallVel[Tick][i][1] * BALL_SPRING;
		}

		GLfloat posh = sqrtf(
			mBallPos[Tick][i][0] * mBallPos[Tick][i][0] +
			mBallPos[Tick][i][2] * mBallPos[Tick][i][2]
		);
		if (posh + BALL_UNIT * .5f > CYLINDER_RADIUS) {
			mBallPos[Tick][i][0] = (mBallPos[Tick][i][0] / posh) * (CYLINDER_RADIUS - BALL_UNIT * .5f);
			mBallPos[Tick][i][2] = (mBallPos[Tick][i][2] / posh) * (CYLINDER_RADIUS - BALL_UNIT * .5f);

			vec3 cylNor = normalize(vec3(
				mBallPos[Tick][i][0],
				0.0f,
				mBallPos[Tick][i][2]
			));
			mBallVel[Tick][i] = reflect(mBallVel[Tick][i], -cylNor) * 0.9f;
		}
	}
}

void BallPit::onKey(int key, int action) {
	//입력처리 함수 - 키보드
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_W:
			mIs_W_Clicked = TRUE;
			break;
		case GLFW_KEY_A:
			mIs_A_Clicked = TRUE;
			break;
		case GLFW_KEY_S:
			mIs_S_Clicked = TRUE;
			break;
		case GLFW_KEY_D:
			mIs_D_Clicked = TRUE;
			break;
		case GLFW_KEY_SPACE:
			mIs_SPC_Clicked = TRUE;
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W:
			mIs_W_Clicked = FALSE;
			break;
		case GLFW_KEY_A:
			mIs_A_Clicked = FALSE;
			break;
		case GLFW_KEY_S:
			mIs_S_Clicked = FALSE;
			break;
		case GLFW_KEY_D:
			mIs_D_Clicked = FALSE;
			break;
		case GLFW_KEY_SPACE:
			mIs_SPC_Clicked = FALSE;
			break;
		default:
			break;
		}
	}
}
void BallPit::onMouseButton(int button, int action) {
	//입력처리 함수 - 마우스 버튼
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			mIs_LMB_Clicked = TRUE;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mMouseAnchPos = mMouseCurrPos;
			mIs_RMB_Clicked = TRUE;
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			mIs_LMB_Clicked = FALSE;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mMouseAnchPos = mMouseCurrPos;
			mIs_RMB_Clicked = FALSE;
			break;
		default:
			break;
		}
	}
}
void BallPit::onMouseMove(int x, int y){
	//입력처리 함수 - 마우스 이동
	mMouseCurrPos = vec2(x, y);
}

#pragma once

#ifndef __BALL_PIT_H__
#define __BALL_PIT_H__

#include "GLApp.h"

//Ball의 크기, 개수, 배열 정의
#define BALL_UNIT 0.1f
#define BALL_NUM (32*32)
#define BALL_CELL 32

//Ball간의 물리효과 계수 : 스프링력, 제동력, 전단력 정의
#define BALL_SPRING .7f
//#define BALL_SPRING 1.f
#define BALL_DAMPING 0.1f
//#define BALL_DAMPING 0.0f
#define BALL_SHEAR 0.1f

//Voxel의 크기, 배열, 단말의 수 정의
#define VOXEL_UNIT (1.0f*BALL_UNIT)
#define VOXEL_CELL 64
#define VOXEL_LEAF 8

//Cylinder(Ball들이 상호작용 할 공간)의 크기 정의, 반지름, 천장, 바닥
#define CYLINDER_RADIUS ((GLfloat)(VOXEL_CELL/2)*VOXEL_UNIT)
#define CYLINDER_CEILING ((GLfloat)(VOXEL_CELL/2)*VOXEL_UNIT)
#define CYLINDER_FLOOR -((GLfloat)(VOXEL_CELL/2)*VOXEL_UNIT)

//기본 중력 정의 
#define GRAVITY_MAG  0.025f
#define GRAVITY vec3(0.0f, -GRAVITY_MAG, 0.0f)

//벡터 관련 수학함수가 정의된 네임스페이스
using namespace vmath;

//정20면체를 테셀레이션 하면 구(Sphere)를 가장 적절하게 근사한 모델을 얻을 수 있음.
const GLfloat tmp = 1.61803401;
const GLfloat tmp2 = sqrtf(4.0f + powf(2 * tmp, 2.0f)) / BALL_UNIT;
static const GLushort vertex_indices[60] =
{
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
static const vec3 vertex_positions[12] =
{
vec3(-1.0f, +tmp, 0.0f) / tmp2,
vec3(+1.0f, +tmp, 0.0f) / tmp2,
vec3(-1.0f, -tmp, 0.0f) / tmp2,
vec3(+1.0f, -tmp, 0.0f) / tmp2,

vec3(0.0f, -1.0f, +tmp) / tmp2,
vec3(0.0f, +1.0f, +tmp) / tmp2,
vec3(0.0f, -1.0f, -tmp) / tmp2,
vec3(0.0f, +1.0f, -tmp) / tmp2,

vec3(+tmp, 0.0f, -1.0f) / tmp2,
vec3(+tmp, 0.0f, +1.0f) / tmp2,
vec3(-tmp, 0.0f, -1.0f) / tmp2,
vec3(-tmp, 0.0f, +1.0f) / tmp2,
};

//Ball Color Index
const vec3 gBallColors[] = {
	vec3(1.0f,0.2f,0.2f),
	vec3(0.2f,1.0f,0.2f),
	vec3(0.2f,0.2f,1.0f),
	vec3(0.2f,1.0f,1.0f),
	vec3(1.0f,1.0f,0.2f),
	vec3(1.0f,0.2f,1.0f),
	vec3(1.0f,0.7f,0.7f),
	vec3(0.7f,0.7f,1.0f),
	vec3(0.7f,1.0f,0.7f)
};

//프로젝트 메인 클래스
class BallPit :public GLApp
{
public:
	void Init();

	virtual void Startup();
	virtual void Update();
	virtual void Render();
	virtual void Shutdown();

	void LoadProgram();

	void InitVI();
	void InitTime();
	void InitGravity();
	void InitBalls();
	void InitVoxels();
	void InitTitle();
	void InitBenchmark();

	void UpdateBenchmark();
	void UpdateTime();
	void UpdateGravity();
	void UpdateBalls();
	void UpdateVoxels();

	void CollideBalls(GLuint start, GLuint offset);

	virtual void onKey(int key, int action);
	virtual void onMouseButton(int button, int action);
	virtual void onMouseMove(int x, int y);

private:
	//입력처리용 변수
	GLboolean		mIs_LMB_Clicked;
	GLboolean		mIs_RMB_Clicked;

	GLboolean		mIs_W_Clicked;
	GLboolean		mIs_A_Clicked;
	GLboolean		mIs_S_Clicked;
	GLboolean		mIs_D_Clicked;
	GLboolean		mIs_SPC_Clicked;

	vec2			mMouseAnchPos;
	vec2			mMouseCurrPos;

	//셰이더 관련 변수
	struct
	{
		GLint           diffuseAlbedo;
		GLint           specularAlbedo;
		GLint           specularPower;
	} mUniform;

	GLuint          mMainProgram;
	GLuint          mVAO;
	GLuint          mPositionBuffer;
	GLuint          mIndexBuffer;
	GLuint          mBPBuffer;

	GLint           mMVLocation;
	GLint           mPJLocation;

	//카메라 위치 관련 변수
	vec3			mCameraPos;

	//Ball 관련 변수 - 위치,속도
	vec3			mBallPos[2][BALL_NUM];
	vec3			mBallVel[2][BALL_NUM];

	//Voxel 관련 변수
	GLushort		mVoxel[VOXEL_CELL * VOXEL_CELL * VOXEL_CELL][VOXEL_LEAF];
	GLint			mVI[27] = {};

	//시간, 초당 처리율 관련 변수,
	LARGE_INTEGER	mPrevTime;
	LARGE_INTEGER	mCurrTime;
	LARGE_INTEGER	mPreq;
	GLfloat			mDeltaTime;
	GLuint			mTick;

	GLfloat			mBenchmark;
	GLfloat			mBench[5];

	//중력 관련 변수
	vec3			mGravity;
	vec2			mGravityH;
};


#endif /* __BALL_PIT_H__ */


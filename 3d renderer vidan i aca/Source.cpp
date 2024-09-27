#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;

	vec3d operator + (const vec3d& v2) const {
		return { this->x + v2.x, this->y + v2.y, this->z + v2.z};
	}
	vec3d operator - (const vec3d& v2) const {
		return { this->x - v2.x, this->y - v2.y, this->z - v2.z};
	}
	vec3d operator * (const float& k) const {
		return { this->x * k,this->y * k ,this->z * k };
	}
	vec3d operator / (const float& k) const  {
		return { this->x / k,this->y / k ,this->z / k };
	}
	float Length()
	{
		return sqrtf(x*x+y*y+z*z);
	}
	vec3d Normalise()
	{
		float l = Length();
		return { x / l, y / l, z / l };
	}
};

float Vector_DotProduct(const vec3d& v1, const vec3d& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3d Vector_CrossProduct(const vec3d& v1, const vec3d& v2)
{
	vec3d v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

struct triangle
{
	vec3d p[3];

	vec3d Normal() {
		vec3d line1 = p[1] - p[0];
		vec3d line2 = p[2] - p[0];
		vec3d normal = Vector_CrossProduct(line1, line2);
		normal = normal.Normalise();

		return normal;
	}
};

struct mesh
{
	vector<triangle> tris;


	bool LoadObjectFromFile(string fileName)
	{
		ifstream f(fileName);
		if (!f.is_open())
		{
			return false;
		}
		vector<vec3d> verts;
		while (!f.eof())
		{
			std::string line;
			std::getline(f, line);			

			stringstream s;
			s << line;
			char junk;


			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x>> v.y>> v.z;
				verts.push_back(v);
			}
			if (line[0] == 'f') 
			{
				triangle t;
				int i[3];
				s >> junk >> i[0] >> i[1] >> i[2];
				t.p[0] = verts[i[0]-1];
				t.p[1] = verts[i[1]-1];
				t.p[2] = verts[i[2]-1];
				tris.push_back(t);
			}
		}
		f.close();
		return true;
	}
};

struct mat4x4
{
	float m[4][4] = { 0 };
};

vec3d Matrix_MultiplyVector(const mat4x4& m, const vec3d& i)
{
	vec3d v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

mat4x4 Matrix_MakeIdentity()
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationX(const float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
{
	mat4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}

mat4x4 Matrix_PointAt(const vec3d& pos, const vec3d& target,const vec3d& up)
{
	vec3d newForward = target - pos;

	newForward = newForward.Normalise();

	vec3d a = newForward * Vector_DotProduct(up, newForward);
	vec3d newUp = up - a;
	newUp = newUp.Normalise();

	vec3d newRight = Vector_CrossProduct(newUp, newForward);
	


	mat4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
{
	mat4x4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}




int main()
{
	int screenWidth = 1000;
	int screenHeight = 700;
	float fTheta = 0;
	bool isRotating = 0;
	vec3d vCamera = { 0.f,0.f,0.f };
	float cameraVelocity = 150.0f;
	float cameraLookDirChangeVelocity = 20.0f;
	vec3d vUp = { 0.f,1.0f,0.f };
	vec3d vForward = { 0.f, 0.f, 1.0f };
	vec3d vLookDir = { 0.f,0.f,1.0f };
	float angleX = 0;
	float angleY = 0;

	vec3d vTarget = vCamera + vLookDir;

	vector <triangle> vecTrianglesToRaster;


	SDL_SetMainReady();
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_Window* win = SDL_CreateWindow("Aca i vidan", 50, 50, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	
	mesh meshCube;
	mat4x4 matProj;

	//meshCube.tris = {
	//	//SOUTH
	//	{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
	//	{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

	//	// EAST                                                      
	//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
	//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

	//	// NORTH                                                     
	//	{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
	//	{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

	//	// WEST                                                      
	//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
	//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

	//	// TOP                                                       
	//	{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
	//	{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

	//	// BOTTOM                                                    
	//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
	//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },
	//};

	meshCube.LoadObjectFromFile("teapot.obj");

	float fAspectRatio = (float)screenHeight / (float)screenWidth;
	float fFov = 90.0f;
	float fNear = 0.1f;
	float fFar = 1000.0f;

	matProj = Matrix_MakeProjection(fFov, fAspectRatio, fNear, fFar);


	bool running = true;
	float currentTime = SDL_GetTicks();
	float deltaTime = 0;
	while (running)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		mat4x4 matRotZ, matRotX, matRotY;
		deltaTime = SDL_GetTicks() - currentTime;
		currentTime = SDL_GetTicks();
		fTheta += 1.0f * deltaTime / 1000 * isRotating;

		// Rotation Z

		matRotZ = Matrix_MakeRotationZ(fTheta);

		// Rotation X
		
		matRotX = Matrix_MakeRotationX(fTheta * 0.5);

		// Translation 

		mat4x4 matTrans = Matrix_MakeTranslation(0.0f,0.0f,12.0f);

		mat4x4 matWorld = Matrix_MakeIdentity();
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		float r = 0;// abs((float)rand() / RAND_MAX);
		float g = 0;// abs((float)rand() / RAND_MAX);
		float b = 0;// abs((float)rand() / RAND_MAX);


		for (auto tri : meshCube.tris)
		{
			triangle Transformed, Moved, Viewed;

			Transformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
			Transformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
			Transformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);
		
			/*mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

			mat4x4 matView = Matrix_QuickInverse(matCamera);


			Viewed.p[0] = Matrix_MultiplyVector(matView, Transformed.p[0]);
			Viewed.p[1] = Matrix_MultiplyVector(matView, Transformed.p[1]);
			Viewed.p[2] = Matrix_MultiplyVector(matView, Transformed.p[2]);
			*/
			Moved.p[0] = Transformed.p[0] - vCamera;
			Moved.p[1] = Transformed.p[1] - vCamera;
			Moved.p[2] = Transformed.p[2] - vCamera;
			
			//mat4x4 matCameraRotX = Matrix_MakeRotationX(-acosf(Vector_DotProduct(vForward, { 0, vLookDir.y, vLookDir.z})));
			//mat4x4 matCameraRotY = Matrix_MakeRotationY(-acosf(Vector_DotProduct(vForward, { vLookDir.x, 0, vLookDir.z })));

			mat4x4 matCameraRotX = Matrix_MakeRotationX(-angleX);
			mat4x4 matCameraRotY = Matrix_MakeRotationY(-angleY);

			mat4x4 matCameraRotXY = Matrix_MultiplyMatrix(matCameraRotX, matCameraRotY);

			Viewed.p[0] = Matrix_MultiplyVector(matCameraRotXY, Moved.p[0]);
			Viewed.p[1] = Matrix_MultiplyVector(matCameraRotXY, Moved.p[1]);
			Viewed.p[2] = Matrix_MultiplyVector(matCameraRotXY, Moved.p[2]);


			vec3d normal = Viewed.Normal();

			if (Vector_DotProduct(normal, Transformed.p[0]) < 0.f)
			{

				vecTrianglesToRaster.push_back(Viewed);
			}
		}

		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle t1, triangle t2)
			{
				float z1 = t1.p[0].z + t1.p[1].z + t1.p[2].z;
				float z2 = t2.p[0].z + t2.p[1].z + t2.p[2].z;
				return z1 > z2;
			});

		vec3d lightDirection = { 0.f, 0.f, -1.0f };

		lightDirection = lightDirection.Normalise();


		for (auto viewed : vecTrianglesToRaster)
		{
			triangle Projected, Transformed;

			mat4x4 matCameraRotX = Matrix_MakeRotationX(angleX);
			mat4x4 matCameraRotY = Matrix_MakeRotationY(angleY);

			mat4x4 matCameraRotYX = Matrix_MultiplyMatrix(matCameraRotY, matCameraRotX);

			Transformed.p[0] = Matrix_MultiplyVector(matCameraRotYX, viewed.p[0]) + vCamera;
			Transformed.p[1] = Matrix_MultiplyVector(matCameraRotYX, viewed.p[1]) + vCamera;
			Transformed.p[2] = Matrix_MultiplyVector(matCameraRotYX, viewed.p[2]) + vCamera;

			vec3d normal = Transformed.Normal();

			float dp = Vector_DotProduct(normal, lightDirection);
			
			if (dp < 0) dp = 0;

			Projected.p[0] = Matrix_MultiplyVector(matProj, viewed.p[0]);
			Projected.p[1] = Matrix_MultiplyVector(matProj, viewed.p[1]);
			Projected.p[2] = Matrix_MultiplyVector(matProj, viewed.p[2]);


			Projected.p[0] = Projected.p[0] / Projected.p[0].w;
			Projected.p[1] = Projected.p[1] / Projected.p[1].w;
			Projected.p[2] = Projected.p[2] / Projected.p[2].w;


			Projected.p[0].x += 1.0f; Projected.p[0].y += 1.0f;
			Projected.p[1].x += 1.0f; Projected.p[1].y += 1.0f;
			Projected.p[2].x += 1.0f; Projected.p[2].y += 1.0f;

			Projected.p[0].y *= 0.5f * (float)screenHeight;
			Projected.p[0].x *= 0.5f * (float)screenWidth;
			Projected.p[1].y *= 0.5f * (float)screenHeight;
			Projected.p[1].x *= 0.5f * (float)screenWidth;
			Projected.p[2].y *= 0.5f * (float)screenHeight;
			Projected.p[2].x *= 0.5f * (float)screenWidth;

			const vector< SDL_Vertex > verts =
			{
				{ SDL_FPoint{ Projected.p[0].x, Projected.p[0].y}, SDL_Color{ (Uint8)(0 * dp),(Uint8)(255 * dp),(Uint8)(255 * dp),255}, SDL_FPoint{0},},
				{ SDL_FPoint{ Projected.p[1].x, Projected.p[1].y}, SDL_Color{ (Uint8)(255 * dp),(Uint8)(255 * dp),(Uint8)(255 * dp),255}, SDL_FPoint{ 0 }, },
				{ SDL_FPoint{ Projected.p[2].x, Projected.p[2].y}, SDL_Color{ (Uint8)(255 * dp),(Uint8)(255 * dp),(Uint8)(255 * dp),255}, SDL_FPoint{ 0 }, },
			};

			SDL_SetRenderDrawColor(renderer, 255 * r, 255 * g, 255 * b, 255);
			SDL_RenderGeometry(renderer, nullptr, verts.data(), verts.size(), nullptr, 0);
			/*SDL_RenderDrawLineF(renderer, Projected.p[0].x, Projected.p[0].y, Projected.p[1].x, Projected.p[1].y);
			SDL_RenderDrawLineF(renderer, Projected.p[1].x, Projected.p[1].y, Projected.p[2].x, Projected.p[2].y);
			SDL_RenderDrawLineF(renderer, Projected.p[2].x, Projected.p[2].y, Projected.p[0].x, Projected.p[0].y);*/
		}
					
		
		SDL_RenderPresent(renderer);
		vecTrianglesToRaster.clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{


			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					running = false;
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode)
				{
				case SDL_SCANCODE_S:
					vCamera = vCamera - vLookDir * cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_W:
					vCamera = vCamera + vLookDir * cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_SPACE:
					vCamera.y -= cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_LSHIFT:
					vCamera.y += cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_A:
					vCamera = vCamera - Vector_CrossProduct(vUp, vLookDir) * cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_D:
					vCamera = vCamera + Vector_CrossProduct(vUp, vLookDir) * cameraVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_UP:
					vLookDir = Matrix_MultiplyVector(Matrix_MakeRotationX(cameraLookDirChangeVelocity * deltaTime / 1000), vLookDir);
					angleX += cameraLookDirChangeVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_DOWN:
					vLookDir = Matrix_MultiplyVector(Matrix_MakeRotationX(-cameraLookDirChangeVelocity * deltaTime / 1000), vLookDir);
					angleX -= cameraLookDirChangeVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_LEFT:
					vLookDir = Matrix_MultiplyVector(Matrix_MakeRotationY(cameraLookDirChangeVelocity * deltaTime / 1000), vLookDir);
					angleY += cameraLookDirChangeVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_RIGHT:
					vLookDir = Matrix_MultiplyVector(Matrix_MakeRotationY(-cameraLookDirChangeVelocity * deltaTime / 1000), vLookDir);
					angleY -= cameraLookDirChangeVelocity * deltaTime / 1000;
					break;
				case SDL_SCANCODE_R:
					isRotating = !isRotating;
					
					break;
				default:
					break;
				}
			}
		}
	}

	SDL_DestroyWindow(win);

	SDL_Quit();
	return 0;
}
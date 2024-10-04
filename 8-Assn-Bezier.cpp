// SmoothMesh: smooth, Phong-shaded .obj mesh

#include <glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "Draw.h"
#include "GLXtras.h"
#include "IO.h"
#include "Misc.h"
#include "VecMat.h"
#include "Widgets.h"
#include "time.h"


// window, camera
int          winWidth = 800, winHeight = 800;
Camera		 camera(0, 0, winWidth, winHeight, vec3(0, 0, 0), vec3(0, 0, -4.5f), 30, 0.001f, 500);
vec3		 grn(.1f, .6f, .1f), yel(1, 1, 0), blu(0, 0, 1);

// for animation
time_t startTime = clock();
float duration = 4; // seconds for animated dot back and forth


// OpenGL IDs
GLuint		 vBuffer = 0, program = 0;


// lighting

class bezierclass {

public: 
	vec4 points[4] = { {.1, .1, 0, 0},
					   {.25, 0.7, 0, 0}, 
					   {.75, 0.7, 0, 0}, 
					   {.9, .1, 0, 0} };
	//compute point method
	
	mat4 Mate = mat4(
		-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0
	);
	vec3 color1 = { 1, 0, 0 };


	vec4 computePoint(float t) {
		vec4 tVec = { t * t * t, t * t, t, 1 };
		mat4 pointsm = {
			points[0], points[1], points[2], points[3],
		};
		return (Transpose(pointsm) * Mate * tVec);
	}

	void drawPoint(float t) {
		UseDrawShader(camera.fullview);
		vec3 myP = (vec3)computePoint(t);
		Disk(myP, 10, color1);
	}

	//draw curve
	void drawCurve() {
		vec4 first = computePoint(0);
		for (int i = 0; i < 100; i++) {
			vec4 onPoint = computePoint((float)i / 100.0);
			vec2 onP = { onPoint[0], onPoint[1] };
			vec4 nextPoint = computePoint((float)(i+1) / 100.0);
			vec2 nextP = { nextPoint[0], nextPoint[1] };
			UseDrawShader(camera.fullview);
			Line(onP, nextP, 3, color1, color1);
		}
		
		//draw line
	}
	
	//draw polygon
	void drawPolygon() {
		vec4 first = computePoint(0);
		for (int i = 0; i < 3; i++) {
			vec3 onPoint = (vec3)points[i];
			vec3 nextPoint = (vec3)points[i+1];
			vec3 color2 = { 0, 1, 0 };
			UseDrawShader(camera.fullview);
			LineDash(onPoint, nextPoint, 3, color2, color2);
			Disk(onPoint, 15, color2);
			if (i == 2) {
				Disk(nextPoint, 15, color2);
			}
		}
	}



};


bezierclass bClass;

// interaction
void	   *picked = NULL;
Mover		mover;


void Display(GLFWwindow *w) {
	// clear screen, enable blend, z-buffer
	Quad(vec3(2, 2, 0), vec3(-2, 2, 0), vec3(-2, -2, 0), vec3(2, -2, 0), true, vec3(1, 1, 1));
	float elapsedTime = (float)(clock() - startTime) / CLOCKS_PER_SEC;
	float alpha = (float)(sin(2 * 3.1415f * elapsedTime / duration) + 1) / 2;
	bClass.drawCurve();
	bClass.drawPolygon();
	bClass.drawPoint(alpha);
	glFlush();
}

// Mouse Handlers

void MouseButton(float x, float y, bool left, bool down) {
	picked = NULL;
	if (left && down) {
		// light picked?
		
		for (size_t i = 0; i < 4; i++)
			if (MouseOver(x, y, (vec2)bClass.points[i], camera.fullview)) {
				picked = &mover;
				mover.Down((vec3*)&bClass.points[i], (int) x, (int) y, camera.modelview, camera.persp);
			}
		if (picked == NULL) {
			picked = &camera;
			camera.Down(x, y, Shift());
		}
		
	}
	else camera.Up();
}

void MouseMove(float x, float y, bool leftDown, bool rightDown) {
	if (leftDown) {
		if (picked == &mover)
			mover.Drag((int) x, (int) y, camera.modelview, camera.persp);
		if (picked == &camera)
			camera.Drag(x, y);
	}
}

void MouseWheel(float spin) {
	camera.Wheel(spin, Shift());
}


void Resize(int width, int height) {
	glViewport(0, 0, width, height);
	camera.Resize(width, height);
}




int main(int argc, char **argv) {
	// enable anti-alias, init app window and GL context
	GLFWwindow *w = InitGLFW(100, 100, winWidth, winHeight, "Beziér Curve");
	
	// callbacks
	RegisterMouseMove(MouseMove);
	RegisterMouseButton(MouseButton);
	RegisterMouseWheel(MouseWheel);
	RegisterResize(Resize);

	// event loop
	while (!glfwWindowShouldClose(w)) {
		Display(w);
		glfwPollEvents();
		glfwSwapBuffers(w);
	}
	// cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vBuffer);
	glfwDestroyWindow(w);
	glfwTerminate();
}

// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once


#include "Camera.h"
#include "Event.h"
#include "Raycaster.h"
#include <map>
#include <vector>



//define type of rendering framework used
#define OGL

#ifdef OGL
#include <GL/freeglut.h>
#define MOUSE_DOWN GLUT_DOWN
#define MOUSE_UP GLUT_UP
#define LEFT_BUTTON GLUT_LEFT_BUTTON
#define KEY_LEFT GLUT_KEY_LEFT
#define KEY_RIGHT GLUT_KEY_RIGHT
#define KEY_DOWN GLUT_KEY_DOWN
#define KEY_UP GLUT_KEY_UP
#endif

namespace µTec
{

typedef Event<void(unsigned char,int ,int)> KeyboardEvent;
typedef Event<void(unsigned char, int, int)> KeyboardSpecialEvent;
typedef Event<void(int, int, int,int)> MouseEvent;
typedef Event<void(int, int)> MouseMovementEvent;
typedef Event<void()> InputEvent;

static bool breakLoop;

class IInput
{
public:
	static void KeyboardCB(unsigned char Key, int x, int y)
	{
		onKeyboard.Invoke(Key,x,y);
	}

	static void mouseCB (int button, int state,int x, int y)
	{
		onMouse.Invoke(button, state, x, y);
	}

	static void SpecialKeyboardCB(int key, int x, int y)
	{
		onKeyboardSpecial.Invoke(key, x, y);
	}

	static void PassiveMouseCB(int x, int y)
	{
		onMouseMove.Invoke(x, y);
	}

	static bool breakLoop;

	static void exitApp()
	{
		breakLoop = true;
	}

	virtual ~IInput(void);

	void update()
	{
		glutMainLoopEvent();
	}

	bool registerCallback(unsigned char key,InputEvent callback);
	bool registerMouseCallback(unsigned int key, InputEvent callback);
protected:

	IInput(void);

	static MouseEvent onMouse;
	static MouseMovementEvent onMouseMove;
	static KeyboardEvent onKeyboard;
	static KeyboardSpecialEvent onKeyboardSpecial;

	virtual void mouseMove(int x, int y)=0;
	virtual void mouseClick(int button, int state, int x, int y)=0;
	virtual void keyboardDown(unsigned char Key, int x, int y)=0;
	virtual void keyboardSpecialDown(unsigned char Key, int x, int y)=0;

	std::map<unsigned char,std::vector<InputEvent>> keyboardEventCallbacks;
	std::map<unsigned int,std::vector<InputEvent>> mouseEventCallbacks;

};

class InputHandler3D : public IInput 
{
public:

	InputHandler3D() : horizontalAngle(0.0f),verticalAngle(0.0f),speed(30.0f),mouseSpeed(0.1f), mainCamera(nullptr) ,freeLook(false), leftMouseDown(false)
	{}

	void setMainCamera(Camera* cam)
	{
		mainCamera=cam;
	}
	void bindActions();
	void mouseMove(int x, int y);
	void mouseClick(int button, int state, int x, int y);
	void keyboardDown(unsigned char Key, int x, int y);

	void riseCamera();
	Ray castRay(int x, int y);
	void keyboardSpecialDown(unsigned char Key, int x, int y);
	void breakMainLoop();
	void lowerCamera();
	void toggleFreeLook();
	Ray currentRay;
	bool leftMouseDown;
	bool freeLook;
private:
	float horizontalAngle;
	float verticalAngle;
	float speed; 
	float mouseSpeed;
	Camera* mainCamera;

	

};

}
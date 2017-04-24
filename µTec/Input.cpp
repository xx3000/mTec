// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "Input.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "Gametime.h"

using namespace µTec;

MouseEvent IInput::onMouse;
MouseMovementEvent IInput::onMouseMove;
KeyboardEvent IInput::onKeyboard;
KeyboardSpecialEvent IInput::onKeyboardSpecial;


bool IInput::breakLoop;

IInput::IInput(void) 
{
	
}



IInput::~IInput(void)
{
}

bool IInput::registerCallback( unsigned char key,InputEvent callback )
{
	keyboardEventCallbacks[key].push_back(callback);
	return true;
}


void InputHandler3D::bindActions()
{
	onMouseMove.Bind<InputHandler3D, &InputHandler3D::mouseMove>(this);
	onMouse.Bind<InputHandler3D, &InputHandler3D::mouseClick>(this);
	onKeyboard.Bind<InputHandler3D, &InputHandler3D::keyboardDown>(this);
	onKeyboardSpecial.Bind<InputHandler3D, &InputHandler3D::keyboardSpecialDown>(this);
	InputEvent callback;
	callback.Bind<InputHandler3D,&InputHandler3D::breakMainLoop>(this);
	registerCallback(27,callback);
	registerCallback('q',callback);
	callback.Bind<InputHandler3D,&InputHandler3D::riseCamera>(this);
	registerCallback('v',callback);
	callback.Bind<InputHandler3D,&InputHandler3D::lowerCamera>(this);
	registerCallback(' ',callback);
}

Ray InputHandler3D::castRay(int mouse_x, int mouse_y) 
{
	//TODO refactor to raycaster and don't do this shit here
	float x = (2.0f * mouse_x) / mainCamera->WINDOW_WIDTH - 1.0f;
	float y = 1.0f - (2.0f * mouse_y) / mainCamera->WINDOW_HEIGHT;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3 (x, y, z);
	glm::vec4 ray_clip = glm::vec4 (ray_nds.x,ray_nds.y, -1.0, 1.0);
	glm::vec4 ray_eye = glm::inverse(mainCamera->getProjectionMatrix()) * ray_clip;
	ray_eye = glm::vec4 (ray_eye.x,ray_eye.y, -1.0, 0.0);
	glm::vec4 ray_wor =(glm::inverse(mainCamera->getViewMatrix()) * ray_eye);
	glm::vec3 ray_dir(ray_wor.x,ray_wor.y,ray_wor.z);
	ray_dir = glm::normalize(ray_dir);
	Ray ray;
	ray.direction=ray_dir;
	ray.origin=mainCamera->getPosition();
	return ray;
}

//TODO Mouse look is delta time invariant
void InputHandler3D::mouseMove( int x, int y )
{
	if(freeLook)
	{

		static bool warped = true;
		if( warped )
		{
			warped = false;
			return;
		}

		warped=true;
		// Get mouse position
		double xpos, ypos;
		//glfwGetCursorPos(window, &xpos, &ypos);
		xpos=x;
		ypos=y;
		// Reset mouse position for next frame
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)/2, glutGet(GLUT_WINDOW_HEIGHT)/2);

		// Compute new orientation
		horizontalAngle += mouseSpeed*0.01f * float(glutGet(GLUT_WINDOW_WIDTH) / 2 - xpos);
		verticalAngle += mouseSpeed*0.01f * float(glutGet(GLUT_WINDOW_HEIGHT) / 2 - ypos);

		// Direction : Spherical coordinates to Cartesian coordinates conversion
		glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle),
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
			);

		// Right vector
		glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f/2.0f),
			0,
			cos(horizontalAngle - 3.14f/2.0f)
			);

		// Up vector
		glm::vec3 up = glm::cross( right, direction );

		if(mainCamera != nullptr)
		{
			mainCamera->setOrientation(direction,up,right);
		}
	}
	else
	{
		currentRay=castRay(x,y);
	}
}

void InputHandler3D::mouseClick( int button, int state, int x, int y )
{
	if(state == GLUT_DOWN)
	if(mouseEventCallbacks.find(button) != mouseEventCallbacks.end())
	{
		for (unsigned i = 0; i < mouseEventCallbacks[button].size(); i++)
		{
			mouseEventCallbacks[button][i].Invoke();
		}
	}
	if(button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			leftMouseDown=true;
		}
		else
		{
			leftMouseDown=false;
		}
	}
	if(button== GLUT_RIGHT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			freeLook=true;
			glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)/2, glutGet(GLUT_WINDOW_HEIGHT)/2);
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		else
		{
			freeLook=false;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
	}
	else
	{
		currentRay=castRay(x,y);
	}
}

void InputHandler3D::keyboardDown( unsigned char Key, int x, int y )
{
	if(keyboardEventCallbacks.find(Key) != keyboardEventCallbacks.end())
	{
		for (unsigned i = 0; i < keyboardEventCallbacks[Key].size(); i++)
		{
			keyboardEventCallbacks[Key][i].Invoke();
		}
	}
}

void InputHandler3D::keyboardSpecialDown( unsigned char Key, int x, int y )
{
	switch (Key) {

	case KEY_UP:
		{
			mainCamera->addToPosition(mainCamera->getTarget() * 0.01f * speed);
		}
		break;

	case KEY_DOWN:
		{
			mainCamera->addToPosition(mainCamera->getTarget() * 0.01f * speed*-1.0f);
		}
		break;

	case KEY_LEFT:
		{
			mainCamera->addToPosition(mainCamera->getRight() * 0.01f * speed*-1.0f);
			
		}
		break;

	case KEY_RIGHT:
		{
			mainCamera->addToPosition(mainCamera->getRight() * 0.01f * speed);
		}
		break;
	}
}

void InputHandler3D::riseCamera()
{
	mainCamera->addToPosition(mainCamera->getUp() * (float)GameTime::deltaTime * speed);
}

void InputHandler3D::lowerCamera()
{
	mainCamera->addToPosition(mainCamera->getUp() * (float)GameTime::deltaTime * speed*-1.0f);
}

void InputHandler3D::breakMainLoop()
{
	IInput::breakLoop=true;
}


void InputHandler3D::toggleFreeLook()
{
	if(freeLook)freeLook=false;
	else
	{
		freeLook=true;
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)/2, glutGet(GLUT_WINDOW_HEIGHT)/2);
	}
}

bool µTec::IInput::registerMouseCallback( unsigned int key, InputEvent callback )
{
	mouseEventCallbacks[key].push_back(callback);
	return true;
}

#include <iostream>
#include <irrlicht.h>
#include <irrKlang.h>
#include <random>


using namespace std;
using namespace irr;
using namespace irrklang;

#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "IrrKlang.lib")

#define STARTING_SPEED 0.3f;

// Global variables 
int screenWidth = 640;
int screenHeight = 480;

int player1Score = 0;
int player2Score = 0;

const int PADDLE_OFFSET_FROM_SIDE = 10;

core::vector2d<f32> ballPosition(0.0f, 0.0f);
core::vector2d<f32> ballDirection(0.0f, 0.0f);
core::vector2d<f32> ballMouseTarget(0.0f, 0.0f);

core::vector2d<f32> paddle1Position(PADDLE_OFFSET_FROM_SIDE, screenHeight/2);
core::vector2d<f32> paddle2Position(screenWidth - PADDLE_OFFSET_FROM_SIDE, screenHeight / 2);

f32 ballSpeed = STARTING_SPEED;
f32 paddle1Speed = 0.8f;
f32 paddle2Speed = 0.8f;

video::ITexture* ball = NULL;
video::ITexture* paddle1 = NULL;
video::ITexture* paddle2 = NULL;

// Global engine pointers
IrrlichtDevice* device = NULL;
video::IVideoDriver* driver = NULL;
ISoundEngine* soundEngine = NULL;




class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
		
		// Process any mouse input events including button presses and movement in the game window.
		if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
		{
			// If we press the leftmouse button, assign the xy coords of the press as a new direction for the ball
			if (event.MouseInput.isLeftPressed())
			{
				// Take the xy coords of the mouse and assign to a vector
				ballMouseTarget.X = (f32)(event.MouseInput.X);
				ballMouseTarget.Y = (f32)(event.MouseInput.Y);

				// Take the mouse xy coords and subtract them from the balls current position to create a vector to the new
				// position
				ballDirection = ballMouseTarget - ballPosition;
				ballDirection.normalize();
			}
		}
		return false;
	}


	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}

	MyEventReceiver()
	{
		for (u32 i = 0; i<KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	}

private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

MyEventReceiver pongEventReceiver;

// Animates the ball
void updateBallPositiion(f32 dt)
{
	// position = x + k*dt
	ballPosition += ballDirection*(ballSpeed*dt);

	// The ball reaches the rightmost and leftmost sections of the game window, reset the position and increase the score.
	if ((ballPosition.X + ball->getSize().Width > driver->getScreenSize().Width && ballDirection.X > 0) || (ballPosition.X < 0) && ballDirection.X < 0)
	{
		// Update player scores
		if (ballPosition.X > (screenWidth / 2))
		{
			player1Score++;
		}
		else
		{
			player2Score++;
		}

		// Reset the ball position, direction, and starting speed.
		ballPosition.X = (screenWidth / 2) - (ball->getSize().Width / 2);
		ballPosition.Y = (screenHeight / 2) - (ball->getSize().Height / 2);
		ballDirection.set(0.0f, 0.0f);
		ballSpeed = STARTING_SPEED;
		soundEngine->play2D("../../../../irrlicht-1.8.1/media/gong.mp3");
	}

	// The ball reaches the topmost and bottommost sections of the game window
	if (ballPosition.Y > driver->getScreenSize().Height - ball->getSize().Height && ballDirection.Y > 0 || ballPosition.Y < 0 && ballDirection.Y < 0)
	{
		ballDirection.Y *= -1;
		soundEngine->play2D("../../../../irrlicht-1.8.1/media/impact.wav");
	}
	
	// Collision detection for paddle 1
	if ((ballPosition.X < paddle1Position.X + paddle1->getSize().Width) // Check if the ball has moved past the right edge of the paddle
		&& (ballPosition.X > paddle1Position.X)	// Check if the ball has not moved past the left edge of the paddle
		
		&&

		((ballPosition.Y + ball->getSize().Height) > paddle1Position.Y) // Check if the ball has moved past the top edge of the paddle
		&& (ballPosition.Y < paddle1Position.Y + paddle1->getSize().Height)) // Check if the ball has not moved past the bottom edge of the paddle
	{
		ballDirection.X *= -1;
		ballSpeed *= 1.05f;
		soundEngine->play2D("../../../../irrlicht-1.8.1/media/ball.wav");
	}

	// Collision detection for paddle 2
	if ((ballPosition.X + ball->getSize().Width > paddle2Position.X) // Check if the ball has moved past the right edge of the paddle
		&& (ballPosition.X > paddle2Position.X - paddle2->getSize().Width)	// Check if the ball has not moved past the left edge of the paddle

		&&

		((ballPosition.Y + ball->getSize().Height) > paddle2Position.Y) // Check if the ball has moved past the top edge of the paddle
		&& (ballPosition.Y < paddle2Position.Y + paddle2->getSize().Height)) // Check if the ball has not moved past the bottom edge of the paddle
	{
		ballDirection.X *= -1;
		ballSpeed *= 1.05f;
		soundEngine->play2D("../../../../irrlicht-1.8.1/media/ball.wav");
	}

}

// Allow the paddles to move
void updatePaddlePositions(f32 dt)
{
	if (pongEventReceiver.IsKeyDown(KEY_KEY_W) && paddle1Position.Y > 0 )
	{
		paddle1Position.Y -= (paddle1Speed*dt);
	}
	else if (pongEventReceiver.IsKeyDown(KEY_KEY_D) && (paddle1Position.Y + paddle2->getSize().Height) < screenHeight)
	{
		paddle1Position.Y += (paddle1Speed*dt);
	}

	if (pongEventReceiver.IsKeyDown(KEY_KEY_O) && paddle2Position.Y > 0)
	{
		paddle2Position.Y -= (paddle2Speed*dt);
	}
	else if (pongEventReceiver.IsKeyDown(KEY_KEY_K) && (paddle2Position.Y + paddle2->getSize().Height) < screenHeight)
	{
		paddle2Position.Y += (paddle2Speed*dt);
	}

	paddle2Position.X = screenWidth - (paddle2->getSize().Width + PADDLE_OFFSET_FROM_SIDE);
}

int main()
{
	/* Create the irrlicht instance using the createDevice function,
	 and check to see if it was initialized properly, if it wasn't then close the program*/
	device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(screenWidth, screenHeight), 32, 0, 0, 1, 0);
	if (!device) return 1;

	// We are going to set the irrlicht window to be resizeable
	device->setResizable(true);

	// Create our custom event receiver and assign it to our irrlicht device.
	device->setEventReceiver(&pongEventReceiver);

	// Create the an irrklang instance using its create function
	soundEngine = createIrrKlangDevice();
	if (!soundEngine) return 1;

	driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* guiEnv = device->getGUIEnvironment();

	//soundEngine->play2D("explosion.wav",1);
	device->setWindowCaption(L"IEEE GDC Pong");

	gui::IGUIFont* font = guiEnv->getFont("../../../../irrlicht-1.8.1/media/bigfont.png");
	gui::IGUIFont* scoreFont = guiEnv->getFont("../../../../irrlicht-1.8.1/media/SF_SquareHead.bmp");


	f32 FPS = 0;
	f32 currentTime = 0;
	f32 endTime = 0;
	f32 deltaTime = 0;

	currentTime = device->getTimer()->getTime();

	ball = driver->getTexture("../../../../irrlicht-1.8.1/media/ball.png");
	paddle1 = driver->getTexture("../../../../irrlicht-1.8.1/media/paddle1.png");
	paddle2 = driver->getTexture("../../../../irrlicht-1.8.1/media/paddle2.png");
	

	ballDirection.set(1,1/*rand()%100, rand()%100*/);
	ballDirection.normalize();
	ballPosition.set(320, 240);

	int FPSStringSize = 0;
	core::stringc FPSString = "";

	//core::floor32()

	/* Here start generating viewable data for the game. Every iteration of this loop
	 is a "frame" that is displayed on the monitor. All code that must run in the game
	 past the initialization steps must be called inside this while loop, also known as
	 the game loop.*/
	while (device->run()) //GAME LOOP
	{
		/* getTime returns time in milliseconds, this doesn't matter for 
		 deltaTime calculations, but for convenience you can multiply by 1000 to get
		 the time in seconds. This way you can have all of your velocities in meters.*/
		endTime = device->getTimer()->getTime();
		deltaTime = (endTime - currentTime);	//deltaTime = (1000.0f / ((endTime - currentTime)));
		currentTime = endTime;

		FPS = 1000.0f / (f32)(deltaTime); // Caculate the frames per second.

		/* Here we call the functions necessary to create movement for our ball
		 and paddles.*/
		updateBallPositiion(deltaTime);
		updatePaddlePositions(deltaTime);

		// Reset screen width/height variables to whatever height is created during resizing.
		screenWidth = driver->getScreenSize().Width;
		screenHeight = driver->getScreenSize().Height;

		// 
		driver->beginScene(1, 1, video::SColor(255, 150, 150, 150));
		smgr->drawAll();
		
		// Draw a line across the middle of the screen
		driver->draw2DLine(core::vector2d<s32>(screenWidth / 2, 0),
							core::vector2d<s32>(screenWidth / 2, screenHeight),
							video::SColor(255, 255, 255, 255));

		driver->draw2DImage(paddle1,
							core::position2d<s32>((s32)paddle1Position.X, (s32)paddle1Position.Y),
							core::rect<s32>(0, 0, paddle1->getSize().Width, paddle1->getSize().Height),
							0,
							video::SColor(255, 255, 255, 255),
							1);

		driver->draw2DImage(paddle2,
							core::position2d<s32>((s32)paddle2Position.X, (s32)paddle2Position.Y),
							core::rect<s32>(0, 0, paddle2->getSize().Width, paddle2->getSize().Height),
							0,
							video::SColor(255, 255, 255, 255),
							1);


		driver->draw2DImage(ball, 
							core::position2d<s32>((s32)ballPosition.X, (s32)ballPosition.Y), 
							core::rect<s32>(0, 0, ball->getSize().Width, ball->getSize().Height),
							0, 
							video::SColor(255,255,255,255),
							1);


		//FPSStringSize = (core::stringc)(driver->getFPS())).c_str().size();
		cout << ((core::stringc)(driver->getFPS())).c_str() << "\b\b\b\b\b";
		


		//cout << "\n" << FPS << "\b\b";
		//device->setWindowCaption((driver->getFPS()));
		//cout << "\b\b" << (int)(driver->getFPS()) << endl;
		//cout << "\b\b\b\b\b\b" << deltaTime;
		//font->draw((core::stringc)driver->getFPS(), core::rect<s32> (2,2, 100,100), video::SColor(255, 0, 100, 0));
		font->draw((core::stringc)deltaTime, core::rect<s32>(2, 2, 100, 100), video::SColor(255, 0, 100, 0));
		scoreFont->draw((core::stringc)player1Score,
						core::rect<s32>(screenWidth / 4, screenHeight / 2, screenHeight, screenWidth),
						video::SColor(255, 255, 255, 255)
						);

		scoreFont->draw((core::stringc)player2Score,
						core::rect<s32>((screenWidth - screenWidth/4), screenHeight / 2, screenHeight, screenWidth),
						video::SColor(255, 255, 255, 255)
						);
		driver->endScene();
	}
	device->closeDevice();
	device->drop();

	return 0;
}
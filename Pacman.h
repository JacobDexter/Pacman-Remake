#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

#define MUNCHIECOUNT 50
#define GHOSTCOUNT 4

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

struct Player 
{
	bool isAlive;
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;
	int direction;
	int frame;
	int currentFrameTime;
	int score;
	float speedMultiplier;
};

struct Enemy 
{
	//ghost
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	Rect* invertedRect;
	int direction;
	float speed;
};

struct Pickup
{
	//Pickup data
	bool isCollected;
	int currentFrameTime;
	int frameCount;
	Rect* rect;
	Rect* invertedRect;
	Texture2D* texture;
	Vector2* position;//temp

	int frameTime;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:
	//Data to represent pacman
	const int _cPacmanFrameTime;
	const float _cPacmanSpeed;

	//Initialise Structs
	Player* _pacman;
	Pickup* _munchie;
	Pickup* _cherry;
	Enemy* _ghost;

	// Data to represent Munchies
	const int _cMunchieFrameTime;

	// UI Positioning
	Vector2* _playerScoreDisplay;
	Vector2* _playerPositionDisplay;
	Vector2* _gameOverDisplay;
	Vector2* _youWinDisplay;

	// Data for Menu
	Texture2D* _menuBackground; 
	Rect* _menuRectangle; 
	Vector2* _menuStringPosition; 
	bool _paused;
	bool _escKeyDown;

	// Sound effects
	SoundEffect* _pop;
	SoundEffect* _pacmanMoveSound;
	SoundEffect* _eatFruit;
	SoundEffect* _pacmanStartMusic;


	// Function prototypes
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckViewportCollision();
	void UpdatePacman(int elapsedTime);
	void UpdateMunchie(Pickup& munchie, int elapsedTime);
	void UpdateGhost(Enemy& ghost, int elapsedTime);
	void CheckGhostCollisions(Enemy& ghost);
	void CheckMunchieCollisions(Pickup& munchie);
	void CheckCherryCollisions(Pickup* cherry);
	void LoadChecks();
	void Restart();

public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};
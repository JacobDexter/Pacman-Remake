#include "Pacman.h"
#include <sstream>
#include "time.h"
#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250), _cMunchieFrameTime(500)
{
	srand(time(NULL));

	//struct initialisation
	_pacman = new Player();
	_cherry = new Pickup();
	_munchie = new Pickup[MUNCHIECOUNT];
	_ghost = new Enemy[GHOSTCOUNT];

	//initialise sounds
	_pop = new SoundEffect();
	_pacmanMoveSound = new SoundEffect();
	_eatFruit = new SoundEffect();
	_pacmanStartMusic = new SoundEffect();

	//loop to initialise all munchies
	for (int i = 0; i < MUNCHIECOUNT; i++) 
	{
		_munchie[i].currentFrameTime = 0;
		_munchie[i].frameCount = 0;
		_munchie[i].frameTime = rand() % 500 + 50;
		_munchie[i].isCollected = false;
	}

	//loop to initialise all ghosts
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghost[i].direction = 0;
		_ghost[i].speed = 1.4f;
	}
	
	_paused = false;

	// Pacman variables
	_pacman->currentFrameTime = 0; 
	_pacman->frame = 0;
	_pacman->direction = 0;
	_pacman->speedMultiplier = 1.0f;
	_pacman->score = 0;
	_pacman->isAlive = true;

	// Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
	
	LoadChecks();

}

Pacman::~Pacman()
{
	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->position;

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		//munchie
		delete _munchie[i].texture;
		delete _munchie[i].rect;
		delete _munchie[i].invertedRect;
		delete _munchie[i].position;
	}

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		delete _ghost[i].texture;
		delete _ghost[i].sourceRect;
		delete _ghost[i].invertedRect;
		delete _ghost[i].position;
	}

	//cherry
	delete _cherry->texture;
	delete _cherry->rect;
	delete _cherry->invertedRect;
	delete _cherry->position;

	delete _pacman;
	delete[] _munchie;
	delete[] _ghost;
	delete _cherry;

	//sounds
	delete _pop;
	delete _pacmanMoveSound;
	delete _eatFruit;
	delete _pacmanStartMusic;
}

void Pacman::LoadContent()
{
	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(512.0f, 384.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load all Munchies in array
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchie[i].texture = new Texture2D();
		_munchie[i].texture->Load("Textures/Munchie.png", true);
		_munchie[i].position = new Vector2((rand() % (Graphics::GetViewportWidth() - 20)), (rand() % (Graphics::GetViewportHeight() - 20)));
		_munchie[i].rect = new Rect(0.0f, 0.0f, 12, 12);
		_munchie[i].invertedRect = new Rect(12.0f, 0.0f, 12, 12);
	}

	// Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherry.png", true);
	_cherry->position = new Vector2((rand() % (Graphics::GetViewportWidth() - 20)), (rand() % (Graphics::GetViewportHeight() - 20)));
	_cherry->rect = new Rect(0.0f, 0.0f, 32, 32);
	_cherry->invertedRect = new Rect(32.0f, 0.0f, 32, 32);

	//load all Ghosts in array
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		// Load Ghost
		_ghost[i].texture = new Texture2D();
		_ghost[i].texture->Load("Textures/GhostBlue.png", false);
		_ghost[i].position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
		_ghost[i].sourceRect = new Rect(0.0f, 0.0f, 20, 20);
		_ghost[i].invertedRect = new Rect(20.0f, 0.0f, 20, 20);
	}

	// UI Positioning
	_playerPositionDisplay = new Vector2(800.0f, 25.0f);
	_playerScoreDisplay = new Vector2(10.0f, 25.0f);
	_gameOverDisplay = new Vector2(470.0f, 384.0f);
	_youWinDisplay = new Vector2(470.0f, 384.0f);

	// Menu
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	// Sound effects
	_pop->Load("Sounds/pop.wav");
	_pacmanMoveSound->Load("Sounds/pacman_chomp.wav");
	_eatFruit->Load("Sounds/pacman_eatfruit.wav");
	_pacmanStartMusic->Load("Sounds/pacman_beginning.wav");

	Audio::Play(_pacmanStartMusic);
}

void Pacman::Update(int elapsedTime)
{
	SoundEffectState _pacmanMoveSoundState = _pacmanMoveSound->GetState();
	SoundEffectState _pacmanStartMusicState = _pacmanStartMusic->GetState();

	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	Input::MouseState* mouseState = Input::Mouse::GetState();

	// Check if game paused 
	CheckPaused(keyboardState, Input::Keys::ESCAPE);


	if (!_paused && _pacman->isAlive)
	{
		CheckViewportCollision();
		CheckCherryCollisions(_cherry);

		if (_pacmanStartMusicState != SoundEffectState::PLAYING)
		{
			Input(elapsedTime, keyboardState, mouseState);
			UpdatePacman(elapsedTime);

			for (int i = 0; i < GHOSTCOUNT; i++)
			{
				UpdateGhost(_ghost[i], elapsedTime);
				CheckGhostCollisions(_ghost[i]);
			}
		}

		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			UpdateMunchie(_munchie[i], elapsedTime);
			CheckMunchieCollisions(_munchie[i]);
		}

		if (_pacmanMoveSoundState != SoundEffectState::PLAYING && _pacmanStartMusicState != SoundEffectState::PLAYING)
		{
			Audio::Play(_pacmanMoveSound);
		}
	}
}

void Pacman::Draw(int elapsedTime)
{
	Input::MouseState* mouseState = Input::Mouse::GetState();

	// Player Position
	std::stringstream stream;
	stream << "Pacman X: " << (truncf(_pacman->position->X * 10) / 10) << " Y: " << (truncf(_pacman->position->Y * 10) / 10) << endl;

	//Player Score
	std::stringstream score;
	score << _pacman->score;

	//Game over screen
	std::stringstream gameOver;
	gameOver << "Game Over" << endl << "Score: " << _pacman->score << endl;

	//you win screen
	std::stringstream playerWin;
	playerWin << "You Win!" << endl << "Score: " << _pacman->score << endl;

	SpriteBatch::BeginDraw(); // Starts Drawing

	if (_pacman->isAlive && _pacman->score != 600)
	{
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect); // Draws Pacman

		for (int i = 0; i < GHOSTCOUNT; i++)
		{
			SpriteBatch::Draw(_ghost[i].texture, _ghost[i].position, _ghost[i].sourceRect); // Draws Pacman
		}

		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			if (_munchie[i].frameCount == 0)
			{
				if (!_munchie[i].isCollected)
				{
					// Draws Red Munchie
					SpriteBatch::Draw(_munchie[i].texture, _munchie[i].position, _munchie[i].invertedRect);
				}
				if (!_cherry->isCollected)
				{
					SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->invertedRect);
				}
			}
			else
			{
				if (!_munchie[i].isCollected)
				{
					// Draw Blue Munchie
					SpriteBatch::Draw(_munchie[i].texture, _munchie[i].position, _munchie[i].rect);
				}
				
				if (!_cherry->isCollected) 
				{
					SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->rect);
				}

				if (_munchie[i].frameCount >= 60)
					_munchie[i].frameCount = 0;
			}
		}

		//Pause Menu
		if (_paused) {
			std::stringstream menuStream;
			menuStream << "PAUSED!";

			SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
			SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
		}

		// Draws String
		SpriteBatch::DrawString(stream.str().c_str(), _playerPositionDisplay, Color::Green);
		SpriteBatch::DrawString(score.str().c_str(), _playerScoreDisplay, Color::Red);
	}
	else if (!_pacman->isAlive)
	{
		SpriteBatch::DrawString(gameOver.str().c_str(), _gameOverDisplay, Color::Red);
		Audio::Stop(_pacmanMoveSound);

		if (mouseState->LeftButton == Input::ButtonState::PRESSED)
		{
			Restart();
		}
	}
	else if (_pacman->score == 600 && _pacman->isAlive)
	{
		SpriteBatch::DrawString(playerWin.str().c_str(), _youWinDisplay, Color::Green);
		Audio::Stop(_pacmanMoveSound);

		if (mouseState->LeftButton == Input::ButtonState::PRESSED)
		{
			Restart();
		}
	}

	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	float _pacmanSpeed = _cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;

	// Checks if D key is pressed
	if (keyboardState->IsKeyDown(Input::Keys::D) || keyboardState->IsKeyDown(Input::Keys::RIGHT)) {
		_pacman->position->X += _pacmanSpeed; //Moves Pacman across X axis
		_pacman->direction = 0;
	}
	// Checks if A key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::A) || keyboardState->IsKeyDown(Input::Keys::LEFT)) {
		_pacman->position->X += -_pacmanSpeed; //Moves Pacman across -X axis
		_pacman->direction = 2;
	}
	// Checks if S key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::S) || keyboardState->IsKeyDown(Input::Keys::DOWN)) {
		_pacman->position->Y += _pacmanSpeed; //Moves Pacman across -Y axis
		_pacman->direction = 1;
	}
	// Checks if W key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::W) || keyboardState->IsKeyDown(Input::Keys::UP)) {
		_pacman->position->Y += -_pacmanSpeed; //Moves Pacman across Y axis
		_pacman->direction = -1;
	}

	if (!keyboardState->IsKeyDown(Input::Keys::D) && !keyboardState->IsKeyDown(Input::Keys::A) && !keyboardState->IsKeyDown(Input::Keys::S) && !keyboardState->IsKeyDown(Input::Keys::W) && !keyboardState->IsKeyDown(Input::Keys::UP) && !keyboardState->IsKeyDown(Input::Keys::DOWN) && !keyboardState->IsKeyDown(Input::Keys::LEFT) && !keyboardState->IsKeyDown(Input::Keys::RIGHT))
	{
		if (_pacman->direction == 0)
		{
			_pacman->position->X += _pacmanSpeed; //Moves Pacman across X axis
		}
		else if (_pacman->direction == 2)
		{
			_pacman->position->X += -_pacmanSpeed; //Moves Pacman across -X axis
		}
		else if (_pacman->direction == 1)
		{
			_pacman->position->Y += _pacmanSpeed; //Moves Pacman across -Y axis
		}
		else if (_pacman->direction == -1)
		{
			_pacman->position->Y += -_pacmanSpeed; //Moves Pacman across Y axis
		}
	}

	//apply speed boost
	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT)) 
	{
		_pacman->speedMultiplier = 2.0f;
	}
	else 
	{
		_pacman->speedMultiplier = 1.0f;
	}

	//randomise cherry position
	if (keyboardState->IsKeyDown(Input::Keys::R))
	{
		_cherry->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	}
}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey)
{
	//Pause
	if (state->IsKeyDown(pauseKey) && !_escKeyDown) {
		_paused = !_paused;
		_escKeyDown = true;
	}

	if (state->IsKeyUp(pauseKey)) {
		_escKeyDown = false;
	}
}

void Pacman::CheckViewportCollision()
{
	// Right Border
	if (_pacman->position->X + _pacman->sourceRect->Width > Graphics::GetViewportWidth()) {
		_pacman->position->X = 32 - _pacman->sourceRect->Width;
	}
	// Left Border
	else if (_pacman->position->X + _pacman->sourceRect->Width < 32) {
		_pacman->position->X = Graphics::GetViewportWidth() - _pacman->sourceRect->Width;
	}
	// Top Border
	else if (_pacman->position->Y + _pacman->sourceRect->Width > Graphics::GetViewportHeight()) {
		_pacman->position->Y = 32 - _pacman->sourceRect->Height;
	}
	// Bottom Border
	else if (_pacman->position->Y + _pacman->sourceRect->Width < 32) {
		_pacman->position->Y = Graphics::GetViewportHeight() - _pacman->sourceRect->Height;
	}
}

void Pacman::UpdatePacman(int elapsedTime)
{
	if (_pacman->isAlive)
	{
		if (_pacman->currentFrameTime > _cPacmanFrameTime)
		{
			_pacman->frame++;

			if (_pacman->frame > +1) {
				_pacman->frame = 0;
			}

			_pacman->currentFrameTime = 0;
		}

		//pacman animation
		_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;
		//Update Pacman Direction
		_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction;
		//pacman current frame time
		_pacman->currentFrameTime += elapsedTime;
	}
}

void Pacman::UpdateMunchie(Pickup& munchie, int elapsedTime)
{
		munchie.currentFrameTime += elapsedTime;

		if (munchie.currentFrameTime > _cMunchieFrameTime) {
			munchie.frameCount++;

			if (munchie.frameCount >= 2) {
				munchie.frameCount = 0;
			}

			munchie.currentFrameTime = 0;
	}
}

void Pacman::UpdateGhost(Enemy& ghost, int elapsedTime)
{
	if (abs(_pacman->position->Y - ghost.position->Y) < 200.0f && abs(_pacman->position->X - ghost.position->X) < 200.0f)
	{
		if (_pacman->position->Y < ghost.position->Y)
		{
			ghost.position->Y -= ghost.speed; //Moves down
			ghost.direction = -1;
		}
		else if (_pacman->position->Y > ghost.position->Y)
		{
			ghost.position->Y += ghost.speed; //Moves up
			ghost.direction = 1;
		}

		if (_pacman->position->X > ghost.position->X)
		{
			ghost.position->X += ghost.speed; //Moves right
			ghost.direction = 0;
		}
		else if (_pacman->position->X < ghost.position->X)
		{
			ghost.position->X -= ghost.speed; //Move left
			ghost.direction = 2;
		}
	}
	
	//Update Ghost Direction
	ghost.sourceRect->Y = ghost.sourceRect->Height * ghost.direction;
}

void Pacman::CheckGhostCollisions(Enemy& ghost)
{
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		bottom2 = ghost.position->Y + ghost.sourceRect->Height;
		top2 = ghost.position->Y;
		left2 = ghost.position->X;
		right2 = ghost.position->X + ghost.sourceRect->Width;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			_pacman->isAlive = false;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::CheckMunchieCollisions(Pickup& munchie)
{
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;

	bottom2 = munchie.position->Y + munchie.rect->Height;
	top2 = munchie.position->Y;
	left2 = munchie.position->X;
	right2 = munchie.position->X + munchie.rect->Width;

	if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
	{
		if (!munchie.isCollected)
		{
			Audio::Play(_pop);
			_pacman->score += 10;
		}
		munchie.isCollected = true;
		i = MUNCHIECOUNT;
	}
}

void Pacman::CheckCherryCollisions(Pickup* cherry)
{
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;

	bottom2 = cherry->position->Y + cherry->rect->Height;
	top2 = cherry->position->Y;
	left2 = cherry->position->X;
	right2 = cherry->position->X + cherry->rect->Width;

	if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
	{
		if (!cherry->isCollected)
		{
			Audio::Play(_eatFruit);
			_pacman->score += 100;
		}
		cherry->isCollected = true;
	}
}

void Pacman::LoadChecks()
{
	if (!Audio::IsInitialised())
	{
		std::cout << "Audio engine has failed to initialise!";
	}
	if (!_pop->IsLoaded())
	{
		std::cout << "_pop sound effect has failed to load.";
	}	
	if (!_pacmanMoveSound->IsLoaded())
	{
		std::cout << "_pacmanMoveSound sound effect has failed to load.";
	}
}

void Pacman::Restart()
{
	_pacman->position = new Vector2(512.0f, 384.0f);
	_pacman->isAlive = true;
	_pacman->score = 0;

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchie[i].position = new Vector2((rand() % (Graphics::GetViewportWidth() - 20)), (rand() % (Graphics::GetViewportHeight() - 20)));
		_munchie[i].isCollected = false;
	}

	_cherry->position = new Vector2((rand() % (Graphics::GetViewportWidth() - 20)), (rand() % (Graphics::GetViewportHeight() - 20)));
	_cherry->isCollected = false;

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghost[i].position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	}
}
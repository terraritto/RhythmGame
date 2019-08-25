#include "Game.h"
#include "main_header\Actors\Actor.h"
#include "main_header\Objects\UIScreen.h"
#include "main_header\Objects\Font.h"
#include "main_header\Objects\MainScreen.h"
#include "main_header\Objects\StartScene.h"
#include "main_header\Objects\InputSystem.h"
#include "main_header\Objects\LoadNotesFile.h"
#include "main_header\Objects\define.h"
#include <algorithm>

Game::Game()
	: mGameState(GameState::EGamePlay)
	, mRhythmGameState(RhythmGame::EStartScene)
	, mNextState(RhythmGame::EStartScene)
	, mUpdatingActors(false)
	, mTickCount(0)
{

}

bool Game::Initialize()
{
	// ウインドウモードで起動
	ChangeWindowMode(TRUE);

	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
	//SetGraphMode(540, 960, 32);

	// ＤＸライブラリの初期化
	if (DxLib_Init() < 0) { return false; };

	//音データのメモリに載せる際の手法の選択
	SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMPRESS);
	
	SetDrawScreen(DX_SCREEN_BACK);
	//initialize input system
	mInputSystem = new InputSystem();
	if (!mInputSystem->Initialize())
	{
		return false;
	}
	
	SetMouseDispFlag(FALSE);
	
	mTickCount = GetNowCount();

	LoadData();

	return true;
}

void Game::Shutdown()
{
	UnLoadData();
	DxLib_End();
}

void Game::RunLoop()
{
	while (mGameState != GameState::EQuit && ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	mInputSystem->PrepareForUpdate();

	mInputSystem->Update();
	const InputState& state = mInputSystem->GetState();

	//Input about actor
	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		if (mGameState == GameState::EGamePlay) {
			actor->ProcessInput(state);
		}
		else if (!mUIStack.empty())
		{
			mUIStack.back()->ProcessInput(state);
		}
	}
	mUpdatingActors = false;

	//Input about ui
	if (mGameState == GameState::EGamePlay)
	{

	}

	//Input about StartScene
	if (mRhythmGameState == RhythmGame::EStartScene)
	{
		mStartSceen->ProcessInput(state);
	}
}

void Game::GenerateOutput()
{
	std::vector<Actor*> drawActor;

	for (auto actor : mActors)
	{
		//透過なし部分だけを描画
		if (actor->GetIsDrawModel<bool>()) {
			auto chara = actor->GetModelHandle();
			if (chara)
			{
				VECTOR pos = actor->GetPosition<VECTOR>();
				pos = Math::VectorTransAxis(pos);
				MV1SetPosition(chara, pos);
				MV1DrawModel(chara);
			}
		}
		else
		{
			drawActor.emplace_back(actor);
		}
	}

	//透過あり部分を描画
	for (auto actor : drawActor)
	{
		actor->Draw();
	}

	//UI draw
	for (auto ui : mUIStack)
	{
		ui->Draw();
	}

	//if string display above, you can't see to overlap image.
	//so string is loaded in this position.
	if (mRhythmGameState == RhythmGame::EStartScene)
	{
		mStartSceen->DrawStr();
	}

	//上の奴
	//DrawBox(0, 0, 540, 100, GetColor(255, 0, 0), TRUE);
	//DrawBox(0, 150, 150, 300, GetColor(0, 255, 0), TRUE);
}

void Game::UpdateGame()
{
	//Time
 	float deltaTime = (GetNowCount() - mTickCount) / 1000.0f;
	if (deltaTime > 0.05f) { deltaTime = 0.05f; }
	mTickCount = GetNowCount();

	//StartScene
	if (mRhythmGameState == RhythmGame::EStartScene)
	{
		mStartSceen->Update();
	}

	//GameScene
	if (mRhythmGameState == RhythmGame::EGameScene) {
		mMainScreen->Update(); //最初に時間を取得しないと大変なことになるので、時間を得る
	}

	//check delete
	DeleteManager();

	// update all actors
	if (mGameState == GameState::EGamePlay)
	{
		mUpdatingActors = true;
		for (auto actor : mActors)
		{
			actor->Update(deltaTime);
		}
		mUpdatingActors = false;

		//move any pending actors to mActors
		for (auto pending : mPendingActors)
		{
			pending->ComputeWorldTransform();
			mActors.emplace_back(pending);
		}
		mPendingActors.clear();

		//Add any dead actors to temporary array
		std::vector<Actor*> deadActors;
		for (auto actor : mActors)
		{
			if (actor->GetState<Actor::State>() == Actor::EDead)
			{
				deadActors.emplace_back(actor);
			}
		}

		// Delete dead actors(which removes them from mActors)
		// delete:can remove entity
		for (auto actor : deadActors)
		{
			delete actor;
		}
	}

	//update UI screens
	for (auto ui : mUIStack)
	{
		if (ui->GetState() == UIScreen::EActive)
		{
			ui->Update(deltaTime);
		}
	}

	//Delete any UIScreens that are closed
	auto iter = mUIStack.begin();
	while (iter != mUIStack.end())
	{
		if ((*iter)->GetState() == UIScreen::EClosing)
		{
			delete* iter;
			iter = mUIStack.erase(iter);
		}
		else
		{
			++iter;
		}
	}

}

void Game::LoadData()
{
	switch (mRhythmGameState)
	{
	case RhythmGame::EStartScene:
		mStartSceen = new StartScene(this);
		break;
	case RhythmGame::EGameScene:
		mMainScreen = new MainScreen(this);
		FileRead("soreha.csv", mMainScreen, this);
		mMainScreen->Start();//とりあえずここでスタート
		break;
	default:
		break;
	}

}

void Game::UnLoadData()
{
	while (!mActors.empty())
	{
		delete mActors.back();
	}

	if (mStartSceen)
	{
		delete mStartSceen;
		mStartSceen = nullptr;
	}

	if (mMainScreen)
	{
		delete mMainScreen;
		mMainScreen = nullptr;
	}

	if (mInputSystem)
	{
		delete mInputSystem;
		mInputSystem = nullptr;
	}

	for (auto font : mFonts)
	{
		font.second->Unload();
	}
}

//Actor
void Game::AddActor(Actor* actor)
{
	//if updating actors, add pending actors
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActors.emplace_back(actor);
	}
}

void Game::RemoveActor(Actor* actor)
{
	//is it in pending actors?
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		//swap to end of vector and pop off
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	//is it in actors?
	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
	{
		//swap to end of vector and pop off
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}

void Game::PushUI(UIScreen* screen)
{
	mUIStack.emplace_back(screen);
}

Font* Game::GetFont(const std::string& fontName)
{
	auto iter = mFonts.find(fontName);
	if (iter != mFonts.end())
	{
		return iter->second;
	}
	else
	{
		Font* font = new Font(this);
		if (font->Load(fontName))
		{
			mFonts.emplace(fontName, font);
		}
		else
		{
			font->Unload();
			delete font;
			font = nullptr;
		}
		return font;
	}
}

//deleter
void Game::DeleteManager()
{
	if (mNextState != mRhythmGameState)
	{
		switch (mRhythmGameState) //delete scene
		{
		case RhythmGame::EStartScene:
			delete mStartSceen;
			mStartSceen = nullptr;
			mActors.shrink_to_fit(); //shink and secure memory.
			break;
		case RhythmGame::EGameScene:
			delete mMainScreen;
			mMainScreen = nullptr;
			mActors.shrink_to_fit(); //shink and secure memory.
		default:
			break;
		}

		mRhythmGameState = mNextState;// change state

		LoadData(); //go to next sceen
	}
}

void Game::DeleteStartScreen(const RhythmGame& state)
{
	mNextState = state;
}

void Game::DeleteMainScreen(const RhythmGame& state)
{
	mNextState = state;
}
#include "../../main_header/Objects/LongNotes.h"
#include "../../main_header/Objects/Player.h"
#include "../../Game.h"

LongNotes::LongNotes(Game* game)
	:Actor(game)
{
	SetPosition<VECTOR>(VGet(game->GetMainScreen()->GetPosOffset().first + 100.0f * mLane + 50,  //レーン位置
		game->GetMainScreen()->GetPosOffset().second + (+2500.0f * (mArrivalTime[0] / 6000000.0f)), //y軸位置
		10.0f));

	game->GetMainScreen()->AddLongNotes(this);
}

LongNotes::~LongNotes()
{
	GetGame<Game>()->GetMainScreen()->RemoveLongNotes(this);
}

void LongNotes::UpdateActor(float deltaTime)
{
	mIsUpdate = true; //毎回初期化してやる
	float startValue = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		+ 2500.0f * ((mArrivalTime[0] - GetGame<Game>()->GetMainScreen()->mNowTime) / 6000000.0f);
	float endValue = +2500.0f * ((mArrivalTime[1] - GetGame<Game>()->GetMainScreen()->mNowTime) / 6000000.0f);

	if (GetGame<Game>()->GetMainScreen()->mPlayer->mIsLongInput[static_cast<int>(mColor)])
	{
		LONGLONG plTime = GetGame<Game>()->GetMainScreen()->mPlayer->mInputLongTime[static_cast<int>(mColor)]
			- GetGame<Game>()->GetMainScreen()->mFirstTime;
		if (
			(plTime - mArrivalTime[0]) >= -JUDGE_TIME * 2
			)
		{
			float shrink = (mArrivalTime[1] - GetGame<Game>()->GetMainScreen()->mNowTime) /
				static_cast<float>(mArrivalTime[1] - mArrivalTime[0]); //shrink ∈ [0,1] となる倍率
			if (shrink <= 0)
			{
				SetState(EDead);
				MV1DeleteModel(GetModelHandle());

				//update combo
				GetGame<Game>()->GetMainScreen()->mTotalCombo += 1;
				GetGame<Game>()->GetMainScreen()->mCombo += 1;
			}

			float scale = mScale * shrink; //これが求める倍率

			MV1SetScale(GetModelHandle(), VGet(1.0f, 1.0f, scale));//scaleを反映

			VECTOR pos = GetPosition<VECTOR>();
			pos.y = GetGame<Game>()->GetMainScreen()->GetPosOffset().second + scale * 10;
			SetPosition<VECTOR>(pos);
			MV1SetOpacityRate(GetModelHandle(), 1.0f); //透過度を上げる

			mIsUpdate = false;
		}
	}

	if (startValue < GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		&& mIsUpdate == true)
	{
		float shrink = (mArrivalTime[1] - GetGame<Game>()->GetMainScreen()->mNowTime) /
			static_cast<float>(mArrivalTime[1] - mArrivalTime[0]); //shrink ∈ [0,1] となる倍率
		if (shrink <= 0) 
		{ 
			SetState(EDead); 	
			MV1DeleteModel(GetModelHandle());
		}

		float scale = mScale * shrink; //これが求める倍率
		
		MV1SetScale(GetModelHandle(), VGet(1.0f, 1.0f, scale));//scaleを反映
		
		VECTOR pos = GetPosition<VECTOR>();
		pos.y = GetGame<Game>()->GetMainScreen()->GetPosOffset().second + scale * 10;
		SetPosition<VECTOR>(pos);
		MV1SetOpacityRate(GetModelHandle(), 0.5f); //透過度を下げる

		//update combo
		GetGame<Game>()->GetMainScreen()->mCombo = 0;
	}
	else if(mIsUpdate == true)
	{
		VECTOR pos = GetPosition<VECTOR>();
		pos.x = GetGame<Game>()->GetMainScreen()->GetPosOffset().first + 100.0f * mLane + 50;
		pos.y = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
			+ (endValue);//+2500.0f * ((ArrivalTime - GetGame<Game>()->GetMainScreen()->mNowTime) / 6000000.0f));
		SetPosition<VECTOR>(pos);
	}
}

void LongNotes::SetColor(char color)
{
	switch (color)
	{
	case 'r':
		SetModelHandle<std::string>("object/Notes_red.mv1");
		mColor = EColor::ERed;
		break;
	case 'g':
		SetModelHandle<std::string>("object/Notes_green.mv1");
		mColor = EColor::EGreen;
		break;
	case 'b':
		SetModelHandle<std::string>("object/Notes_blue.mv1");
		mColor = EColor::EBlue;
		break;
	default:
		break;
	}
}

void LongNotes::SetScale()
{
	float start = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		+ (+2500.0f * (mArrivalTime[0] / 6000000.0f));
	float end = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		+ (+2500.0f * (mArrivalTime[1] / 6000000.0f));
	float size = (end - start) / 10.0f;
	MV1SetScale(GetModelHandle(),VGet(1.0f,1.0f,size));
	mScale = size;
}
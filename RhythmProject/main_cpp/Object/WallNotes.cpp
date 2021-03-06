#include "../../main_header/Objects/WallNotes.h"
#include "../../main_header/Objects/ObjectSampler.h"
#include "../../main_header/Objects/Player.h"
#include "../../Game.h"
#include "../../main_header/Objects/define.h"

WallNotes::WallNotes(Game* game)
	:Actor(game)
{
	game->GetMainScreen()->AddWallNotes(this);
}

WallNotes::~WallNotes()
{
	GetGame<Game>()->GetMainScreen()->RemoveWallNotes(this);
	GetGame<Game>()->GetMainScreen()->EndWallEffect(); //end effect
}

void WallNotes::UpdateActor(float deltaTime)
{
	float startValue = GetGame<Game>()->GetMainScreen()->GetPosOffset().second + mScale * 10
		+ 2500.0f * ((mArrivalTime[0] - GetGame<Game>()->GetMainScreen()->mNowTime) / (ONE_TIME * (2.1 - GetGame<Game>()->GetGameSpeed() / 5)));

	float endValue = +2500.0f * ((mArrivalTime[1] - GetGame<Game>()->GetMainScreen()->mNowTime) / (ONE_TIME * (2.1 - GetGame<Game>()->GetGameSpeed() / 5)));


	if (startValue < GetGame<Game>()->GetMainScreen()->GetPosOffset().second + mScale * 10)
	{
		if (endValue < -500.0f) // 0.0が丁度終わりの部分が届いた場所、そこから少しずらす
		{
			SetState(EDead);
		}

		VECTOR pos = GetPosition<VECTOR>();
		pos.x = GetGame<Game>()->GetMainScreen()->GetPosOffset().first + 100.0f * mLane + 50;//ここでoffsetの影響
		pos.y = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
			+ (endValue);
		SetPosition<VECTOR>(pos);
		
		if (endValue < -100.0f) { return; }//これ以降に当たることはないので重い処理は消す
		else //当たり判定
		{
			int objectHandle = GetGame<Game>()->GetMainScreen()->mObjectSampler[1]->GetModelHandle();

			//位置調整
			MV1SetScale(objectHandle, VGet(1.0f, 1.0f, mScale));//scaleを反映
			MV1SetPosition(objectHandle, Math::VectorTransAxis(pos));

			//collisionの更新
			GetGame<Game>()->GetMainScreen()->mObjectSampler[1]->RefreshCollision();

			//player capsule用
			int plHandle = GetGame<Game>()->GetMainScreen()->mPlayer->GetModelHandle();
			VECTOR posDown = Math::VectorTransAxis(GetGame<Game>()->GetMainScreen()->mPlayer->GetPosition<VECTOR>());
			posDown = VAdd(posDown, VGet(0.0f, PLAYER_RAD_OFFSET, 0.0f));
			VECTOR posUp = VAdd(posDown, VGet(0.0f, PLAYER_RAD_OFFSET, 0.0f));
			
			//当たり判定
			MV1_COLL_RESULT_POLY_DIM  result = MV1CollCheck_Capsule(objectHandle, 0, posDown, posUp, PLAYER_RAD_OFFSET);

			if (result.HitNum >= 1) //hit
			{
				MV1SetMaterialDrawBlendMode(plHandle, 0, DX_BLENDMODE_ADD);
				MV1SetMaterialDrawBlendParam(plHandle, 0, 50);
				GetGame<Game>()->GetMainScreen()->StartWallEffect(posDown); //start effect
			}
		}
	}
	else
	{
		VECTOR pos = GetPosition<VECTOR>();
		pos.x = GetGame<Game>()->GetMainScreen()->GetPosOffset().first + 100.0f * mLane + 50;//ここでoffsetの影響
		pos.y = startValue;//GetGame<Game>()->GetMainScreen()->GetPosOffset().second
			//+ (endValue);//+2500.0f * ((ArrivalTime - GetGame<Game>()->GetMainScreen()->mNowTime) / 6000000.0f));
		pos.z = mHeight; //高さの影響
		SetPosition<VECTOR>(pos);
		GetGame<Game>()->GetMainScreen()->EndWallEffect(); //end effect
	}
}

void WallNotes::Draw()
{
	int objectHandle = GetGame<Game>()->GetMainScreen()->mObjectSampler[1]->GetModelHandle();
	MV1SetScale(objectHandle, VGet(1.0f, 1.0f, mScale));//scaleを反映
	VECTOR pos = GetPosition<VECTOR>();
	MV1SetPosition(objectHandle, Math::VectorTransAxis(pos));
	MV1DrawModel(objectHandle);
}

void WallNotes::SetScale()
{
	float start = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		+ (+2500.0f * (mArrivalTime[0] / (ONE_TIME * (2.1 - GetGame<Game>()->GetGameSpeed() / 5))));
	float end = GetGame<Game>()->GetMainScreen()->GetPosOffset().second
		+ (+2500.0f * (mArrivalTime[1] / (ONE_TIME * (2.1 - GetGame<Game>()->GetGameSpeed() / 5))));
	float size = (end - start) / 10.0f;
	mScale = size;
}
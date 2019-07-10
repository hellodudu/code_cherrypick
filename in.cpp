//------------------------------------------------------------------------------
//!\file ultimate.cpp
//!\brief ultimate服务器对象
//!
//!\date 2018-12-24
//! last 2018-12-24
//!\author dudu
//------------------------------------------------------------------------------
#include "stdafx.h"
#include "ultimate.h"

#include "..\ServerDefine\ultimate_define.h"
#include "../ServerDefine/invite_center.h"
#include "..\ServerDefine\msg_world_ultimate.h"
#include "player_mgr.h"
#include "player.h"
#include "guild_mgr.h"
#include "db_session.h"
#include "record_mgr.h"
#include "scene_mgr.h"
#include "mail_mgr.h"
#include "activity_mgr.h"
#include "center_invite.h"
#include "world_message.pb.h"

//---------------------------------------------------------------------------
// 填充protobuf crossplayerinfo信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoCrossPlayerInfo(world_message::CrossPlayerInfo* pProtoInfo, const tagPlayerInfo* pPlayerInfo)
{
	if (!VALID(pProtoInfo) || !VALID(pPlayerInfo))
		return;

	pProtoInfo->set_player_id(pPlayerInfo->n64PlayerID);
	pProtoInfo->set_user_id(string(pPlayerInfo->szUserID));
	pProtoInfo->set_server_id((uint32)pPlayerInfo->dwWorldID);
	pProtoInfo->set_gender((int32)pPlayerInfo->nGender);
	pProtoInfo->set_race((int32)pPlayerInfo->nRace);
	pProtoInfo->set_head_protrait((int32)pPlayerInfo->n16HeadProtrait);
	pProtoInfo->set_head_quality((int32)pPlayerInfo->n32HeadQuality);
	pProtoInfo->set_player_name(string(pPlayerInfo->szPlayerName));
	pProtoInfo->set_guild_id(VALID(pPlayerInfo->pGuildMem) ? pPlayerInfo->pGuildMem->n64GuildID : INVALID);
	pProtoInfo->set_last_logoff_time((uint32)pPlayerInfo->dwLastLogoffTime);
	pProtoInfo->set_online(pPlayerInfo->bOnline == TRUE);
	pProtoInfo->set_level(pPlayerInfo->nLevel);
	pProtoInfo->set_vip_level(pPlayerInfo->nVipLevel);
	pProtoInfo->set_player_score(pPlayerInfo->nPlayerScore);
	pProtoInfo->set_history_score(pPlayerInfo->nHistoryScore);
	pProtoInfo->set_remains_floor(pPlayerInfo->nRemainsFloor);
	pProtoInfo->set_peak_level(pPlayerInfo->nPeakLevel);
	pProtoInfo->set_player_status(VALID(pPlayerInfo->pStatusInfo) ? pPlayerInfo->pStatusInfo->dwPlayerStatus : 0);
	pProtoInfo->set_cur_decorate(pPlayerInfo->nCurDecorate);
	pProtoInfo->set_return_player_id(pPlayerInfo->n64ReturnPlayerID);
	pProtoInfo->set_last_square_action_time(pPlayerInfo->dwLastSquareActionTime);

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
		pProtoInfo->add_hero_type_id((uint32)pPlayerInfo->dwHeroTypeID[n]);

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
		pProtoInfo->add_fashion_id(pPlayerInfo->nFashionID[n]);

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
		pProtoInfo->add_mount_type_id((uint32)pPlayerInfo->dwMountTypeID[n]);

	for (auto n = 0; n < X_Rune_Max_Group; ++n)
		pProtoInfo->add_rune_type_id((uint32)pPlayerInfo->dwRuneTypeID[n]);

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
		pProtoInfo->add_state_flag(pPlayerInfo->nStateFlag[n]);

}

//---------------------------------------------------------------------------
// 填充protobuf crossguildinfo信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoCrossGuildInfo(world_message::CrossGuildInfo* pProtoInfo, const Guild* pGuild)
{
	if (!VALID(pProtoInfo) || !VALID(pGuild))
		return;

	pProtoInfo->set_guild_id(pGuild->GetID());
	pProtoInfo->set_server_id(sServer.GetWorldID());
	pProtoInfo->set_master_id(pGuild->GetMasterID());
	pProtoInfo->set_create_time((uint32)pGuild->GetCreateTime());
	pProtoInfo->set_hall_level(pGuild->GetLevel());
	pProtoInfo->set_score(pGuild->GetGuildScore());
	pProtoInfo->set_castle_score(pGuild->GetGuildCastleScore());
	pProtoInfo->set_member_num(pGuild->GetMemberNum());

	pProtoInfo->set_guild_name(string(pGuild->GetName()));
	pProtoInfo->set_master_name(string(pGuild->GetMasterName()));
}

//---------------------------------------------------------------------------
// 填充protobuf arenarecord信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoArenaRecord(Player* pPlayer, world_message::ArenaRecord* pProtoInfo, const tagGroupRecord* pFirstGroup, const tagGroupRecord* pFollowGroup)
{
	if (!VALID(pPlayer) || !VALID(pProtoInfo) || !VALID(pFirstGroup) || !VALID(pFollowGroup))
		return;

	pProtoInfo->set_player_id(pPlayer->GetID());

	world_message::GroupRecord* pGroupRecord = pProtoInfo->mutable_first_group();
	if (!VALID(pGroupRecord))
		return;

	FillProtoGroupRecord(pGroupRecord, pFirstGroup);

	pGroupRecord = pProtoInfo->mutable_follow_group();
	if (!VALID(pGroupRecord))
		return;

	FillProtoGroupRecord(pGroupRecord, pFollowGroup);
}

//---------------------------------------------------------------------------
// 填充protobuf group record信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoGroupRecord(world_message::GroupRecord* pProtoInfo, const tagGroupRecord* pGroupRecord)
{
	if (!VALID(pProtoInfo) || !VALID(pGroupRecord))
		return;

	pProtoInfo->set_player_id(pGroupRecord->n64PlayerID);
	pProtoInfo->set_level(pGroupRecord->nLevel);
	pProtoInfo->set_player_score(pGroupRecord->nPlayerScore);
	pProtoInfo->set_name(string(pGroupRecord->szName));

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
	{
		world_message::HeroRecord* pHeroRecord = pProtoInfo->add_hero_record();
		if (!VALID(pHeroRecord))
			return;

		FillProtoHeroRecord(pHeroRecord, &(pGroupRecord->stHeroRecord[n]));
	}

	for (auto n = 0; n < EDM_End; ++n)
	{
		pProtoInfo->add_dmg_mod_att(pGroupRecord->nDmgModAtt[n]);
	}

	for (auto n = 0; n < X_Rune_Max_Group; ++n)
	{
		pProtoInfo->add_rune_id(pGroupRecord->dwRuneID[n]);
	}

	for (auto n = 0; n < X_Rune_Max_Group; ++n)
	{
		pProtoInfo->add_rune_level(pGroupRecord->n8RuneLevel[n]);
	}

	pProtoInfo->set_guild_id(pGroupRecord->n64GuildID);
	pProtoInfo->set_world_name(string(pGroupRecord->szWorldName));
	pProtoInfo->set_guild_name(string(pGroupRecord->szGuildName));
	pProtoInfo->set_protrait(pGroupRecord->nProtrait);
	pProtoInfo->set_head_quality(pGroupRecord->n8HeadQuality);
	pProtoInfo->set_vip_level(pGroupRecord->n8VipLevel);
	pProtoInfo->set_flag(pGroupRecord->n8Flag);

	for (auto n = 0; n < X_Max_Summon_Num; ++n)
	{
		for (auto m = 0; m < EHDM_End; ++m)
		{
			pProtoInfo->add_hero_dmg_mod_att(pGroupRecord->nHeroDmgModAtt[n][m]);
		}
	}

	pProtoInfo->set_head_protrait(pGroupRecord->n16HeadProtrait);

	for (auto n = 0; n < EBG_End; ++n)
	{
		world_message::BabyRecord* pBabyRecord = pProtoInfo->add_baby_record();
		if (!VALID(pBabyRecord))
			return;

		FillProtoBabyRecord(pBabyRecord, &(pGroupRecord->stBaby[n]));
	}
}

//---------------------------------------------------------------------------
// 填充protobuf hero record信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoHeroRecord(world_message::HeroRecord* pProtoInfo, const tagHeroRecord* pHeroRecord)
{
	if (!VALID(pProtoInfo) || !VALID(pHeroRecord))
		return;

	pProtoInfo->set_entity_id(pHeroRecord->dwEntityID);
	pProtoInfo->set_state_flag(pHeroRecord->nStateFlag);
	pProtoInfo->set_fashion_id(pHeroRecord->nFashionID);
	pProtoInfo->set_mount_type_id(pHeroRecord->dwMountTypeID);
	pProtoInfo->set_rage_level(pHeroRecord->nRageLevel);
	pProtoInfo->set_level(pHeroRecord->nLevel);
	
	for (auto n = 0; n < EHA_End; ++n)
	{
		pProtoInfo->add_att(pHeroRecord->nAtt[n]);
	}

	for (auto n = 0; n < EHA_End; ++n)
	{
		pProtoInfo->add_base_att(pHeroRecord->nBaseAtt[n]);
	}

	for (auto n = 0; n < EHA_End; ++n)
	{
		pProtoInfo->add_base_att_mod_pct(pHeroRecord->nBaseAttModPct[n]);
	}

	for (auto n = 0; n < EHA_End; ++n)
	{
		pProtoInfo->add_att_mod(pHeroRecord->nAttMod[n]);
	}

	for (auto n = 0; n < EHA_End; ++n)
	{
		pProtoInfo->add_att_mod_pct(pHeroRecord->nAttModPct[n]);
	}

	for (auto n = 0; n < X_Passive_Spell_Num; ++n)
	{
		pProtoInfo->add_passive_spell(pHeroRecord->dwPassiveSpell[n]);
	}

	pProtoInfo->set_fly_up(pHeroRecord->nFlyUp);
	pProtoInfo->set_star_level(pHeroRecord->nStarLevel);
	pProtoInfo->set_quality(pHeroRecord->nQuality);
}

//---------------------------------------------------------------------------
// 填充protobuf baby record信息
//---------------------------------------------------------------------------
VOID Ultimate::FillProtoBabyRecord(world_message::BabyRecord* pProtoInfo, const tagBabyRecord* pBabyRecord)
{
	if (!VALID(pProtoInfo) || !VALID(pBabyRecord))
		return;

	pProtoInfo->set_id(pBabyRecord->n8ID);
	pProtoInfo->set_type(pBabyRecord->n8Type);
	pProtoInfo->set_mood(pBabyRecord->u8Mood);
	pProtoInfo->set_fashion_id(pBabyRecord->u8FashionID);
	pProtoInfo->set_health(pBabyRecord->nHealth);
	pProtoInfo->set_teacher_id(pBabyRecord->dwTeacherID);
	pProtoInfo->set_hire_time(pBabyRecord->dwHireTime);

	for (auto n = 0; n < X_Baby_Skill_Num; ++n)
	{
		pProtoInfo->add_skill_id(pBabyRecord->dwSkillID[n]);
	}

	pProtoInfo->set_name(string(pBabyRecord->szName));
	pProtoInfo->set_train_level(pBabyRecord->n8TrainLevel);
}

//---------------------------------------------------------------------------
// 解码protobuf to ArenaRecord
//---------------------------------------------------------------------------
VOID Ultimate::DecodeProtoToArenaRecord(const world_message::ArenaRecord* pProto, tagArenaRecord* pArenaRecord)
{
	if (!VALID(pProto) || !VALID(pArenaRecord))
		return;

	pArenaRecord->n64PlayerID = pProto->player_id();

	DecodeProtoToGroupRecord(&pProto->first_group(), &pArenaRecord->stFirstRecord);
	DecodeProtoToGroupRecord(&pProto->follow_group(), &pArenaRecord->stFollowRecord);
}

//---------------------------------------------------------------------------
// 解码protobuf to GroupRecord
//---------------------------------------------------------------------------
VOID Ultimate::DecodeProtoToGroupRecord(const world_message::GroupRecord* pProto, tagGroupRecord* pGroupRecord)
{
	if (!VALID(pProto) || !VALID(pGroupRecord))
		return;

	pGroupRecord->n64PlayerID = pProto->player_id();
	pGroupRecord->nLevel = pProto->level();
	pGroupRecord->nPlayerScore = pProto->player_score();
	strncpy_s(pGroupRecord->szName, SHORT_STRING, pProto->name().c_str(), _TRUNCATE);

	for (auto n = 0; n < pProto->hero_record_size(); ++n)
	{
		DecodeProtoToHeroRecord(&pProto->hero_record(n), &(pGroupRecord->stHeroRecord[n]));
	}

	for (auto n = 0; n < pProto->dmg_mod_att_size(); ++n)
	{
		pGroupRecord->nDmgModAtt[n] = pProto->dmg_mod_att(n);
	}

	for (auto n = 0; n < pProto->rune_id_size(); ++n)
	{
		pGroupRecord->dwRuneID[n] = pProto->rune_id(n);
	}

	for (auto n = 0; n < pProto->rune_level_size(); ++n)
	{
		pGroupRecord->n8RuneLevel[n] = pProto->rune_level(n);
	}

	pGroupRecord->n64GuildID = pProto->guild_id();
	strncpy_s(pGroupRecord->szWorldName, SHORT_STRING, pProto->world_name().c_str(), _TRUNCATE);
	strncpy_s(pGroupRecord->szGuildName, SHORT_STRING, pProto->guild_name().c_str(), _TRUNCATE);
	pGroupRecord->nProtrait = pProto->protrait();
	pGroupRecord->n8HeadQuality = pProto->head_quality();
	pGroupRecord->n8VipLevel = pProto->vip_level();
	pGroupRecord->n8Flag = pProto->flag();

	for (auto n = 0; n < pProto->hero_dmg_mod_att_size(); ++n)
	{
		pGroupRecord->nHeroDmgModAtt[n / EHDM_End][n % EHDM_End] = pProto->hero_dmg_mod_att(n);
	}
	
	pGroupRecord->n16HeadProtrait = pProto->head_protrait();

	for (auto n = 0; n < pProto->baby_record_size(); ++n)
	{
		DecodeProtoToBabyRecord(&pProto->baby_record(n), &(pGroupRecord->stBaby[n]));
	}
}

//---------------------------------------------------------------------------
// 解码protobuf to HeroRecord
//---------------------------------------------------------------------------
VOID Ultimate::DecodeProtoToHeroRecord(const world_message::HeroRecord* pProto, tagHeroRecord* pHeroRecord)
{
	if (!VALID(pProto) || !VALID(pHeroRecord))
		return;

	pHeroRecord->dwEntityID = pProto->entity_id();
	pHeroRecord->nStateFlag = pProto->state_flag();
	pHeroRecord->nFashionID = pProto->fashion_id();
	pHeroRecord->dwMountTypeID = pProto->mount_type_id();
	pHeroRecord->nRageLevel = pProto->rage_level();
	pHeroRecord->nLevel = pProto->level();

	for (auto n = 0; n < pProto->att_size(); ++n)
	{
		pHeroRecord->nAtt[n] = pProto->att(n);
	}

	for (auto n = 0; n < pProto->base_att_size(); ++n)
	{
		pHeroRecord->nBaseAtt[n] = pProto->base_att(n);
	}

	for (auto n = 0; n < pProto->base_att_mod_pct_size(); ++n)
	{
		pHeroRecord->nBaseAttModPct[n] = pProto->base_att_mod_pct(n);
	}

	for (auto n = 0; n < pProto->att_mod_size(); ++n)
	{
		pHeroRecord->nAttMod[n] = pProto->att_mod(n);
	}

	for (auto n = 0; n < pProto->att_mod_pct_size(); ++n)
	{
		pHeroRecord->nAttModPct[n] = pProto->att_mod_pct(n);
	}

	for (auto n = 0; n < pProto->passive_spell_size(); ++n)
	{
		pHeroRecord->dwPassiveSpell[n] = pProto->passive_spell(n);
	}

	pHeroRecord->nFlyUp = pProto->fly_up();
	pHeroRecord->nStarLevel = pProto->star_level();
	pHeroRecord->nQuality = pProto->quality();
}

//---------------------------------------------------------------------------
// 解码protobuf to BabyRecord
//---------------------------------------------------------------------------
VOID Ultimate::DecodeProtoToBabyRecord(const world_message::BabyRecord* pProto, tagBabyRecord* pBabyRecord)
{
	if (!VALID(pProto) || !VALID(pBabyRecord))
		return;

	pBabyRecord->n8ID = pProto->id();
	pBabyRecord->n8Type = pProto->type();
	pBabyRecord->u8Mood = pProto->mood();
	pBabyRecord->u8FashionID = pProto->fashion_id();
	pBabyRecord->nHealth = pProto->health();
	pBabyRecord->dwTeacherID = pProto->teacher_id();
	pBabyRecord->dwHireTime = pProto->hire_time();

	for (auto n = 0; n < pProto->skill_id_size(); ++n)
	{
		pBabyRecord->dwSkillID[n] = pProto->skill_id(n);
	}

	strncpy_s(pBabyRecord->szName, SHORT_STRING, pProto->name().c_str(), _TRUNCATE);
}

//---------------------------------------------------------------------------
// 解码protobuf to ArenaTargetInfo
//---------------------------------------------------------------------------
VOID Ultimate::DecodeProtoToArenaTargetInfo(const world_message::ArenaTargetInfo* pProto, tagArenaTargetInfo* pArenaTargetInfo)
{
	if (!VALID(pProto) || !VALID(pArenaTargetInfo))
		return;

	pArenaTargetInfo->n64PlayerID = pProto->player_id();
	strncpy_s(pArenaTargetInfo->szName, SHORT_STRING, pProto->player_name().c_str(), _TRUNCATE);
	strncpy_s(pArenaTargetInfo->szWorldName, SHORT_STRING, pProto->server_name().c_str(), _TRUNCATE);
	pArenaTargetInfo->nLevel = pProto->level();
	pArenaTargetInfo->nPlayerScore = pProto->player_score();
	pArenaTargetInfo->nHeadProtrait = pProto->head_protrait();
	pArenaTargetInfo->nHeadQuality = pProto->head_quality();
	pArenaTargetInfo->nArenaScore = pProto->arena_score();
}

//---------------------------------------------------------------------------
// 消息注册
//---------------------------------------------------------------------------
VOID Ultimate::RegisterUltimateMsg()
{
	RegisterOneProtoMsg("world_message.MUW_WorldLogon", &Ultimate::HandleWorldLogon);
	RegisterOneProtoMsg("world_message.MUW_TestConnect", &Ultimate::HandleTestConnect);
	RegisterOneProtoMsg("world_message.MUW_HeartBeat", &Ultimate::HandleHeartBeat);

	RegisterOneProtoMsg("world_message.MUW_RequestPlayerInfo", &Ultimate::HandleRequestPlayerInfo);
	RegisterOneProtoMsg("world_message.MUW_RequestGuildInfo", &Ultimate::HandleRequestGuildInfo);

	RegisterOneProtoMsg("world_message.MUW_PlayUltimateRecord", &Ultimate::HandlePlayUltimateRecord);
	RegisterOneProtoMsg("world_message.MUW_RequestUltimatePlayer", &Ultimate::HandleRequestUltimatePlayer);
	RegisterOneProtoMsg("world_message.MUW_ViewFormation", &Ultimate::HandleViewFormation);

	RegisterOneProtoMsg("world_message.MUW_ArenaAddRecord", &Ultimate::HandleArenaAddRecord);
	RegisterOneProtoMsg("world_message.MUW_ArenaStartBattle", &Ultimate::HandleArenaStartBattle);
	RegisterOneProtoMsg("world_message.MUW_RequestArenaRank", &Ultimate::HandleRequestArenaRank);
	RegisterOneProtoMsg("world_message.MUW_SyncArenaSeason", &Ultimate::HandleSyncArenaSeason);
	RegisterOneProtoMsg("world_message.MUW_ArenaWeeklyReward", &Ultimate::HandleArenaWeeklyReward);
	RegisterOneProtoMsg("world_message.MUW_ArenaSeasonReward", &Ultimate::HandleArenaSeasonReward);
	RegisterOneProtoMsg("world_message.MUW_ArenaChampion", &Ultimate::HandleArenaChampion);
	RegisterOneProtoMsg("world_message.MUW_ArenaChampionOnline", &Ultimate::HandleArenaChampionOnline);

	RegisterOneProtoMsg("world_message.MUW_CheckInvite", &Ultimate::HandleCheckInvite);
	RegisterOneProtoMsg("world_message.MUW_AddInviteResult", &Ultimate::HandleAddInviteResult);
	RegisterOneProtoMsg("world_message.MUW_InviteRecharge", &Ultimate::HandleInviteRecharge);
}

VOID Ultimate::UnRegisterUltimateMsg()
{
	tagProtoRecvInfo* pRecvInfo = NULL;
	m_mapProtoHandle.ResetIterator();

	while (m_mapProtoHandle.PeekNext(pRecvInfo))
	{
		SAFE_DEL(pRecvInfo);
	}
	m_mapProtoHandle.Clear();
}

VOID Ultimate::RegisterOneProtoMsg(LPCSTR pMsgName, PROTOMSGHANDLE handle)
{
	string strName = pMsgName;
	DWORD dwMsgID = Crc32(strName.c_str());

	// 查找该消息是否已经注册
	if (m_mapProtoHandle.IsExist(dwMsgID))
		return;

	// 生成一个新的消息
	tagProtoRecvInfo* pInfo = new tagProtoRecvInfo;
	pInfo->strName = strName;
	pInfo->handle = handle;

	// 加入到队列中去
	m_mapProtoHandle.Add(dwMsgID, pInfo);
}

//---------------------------------------------------------------
// 时间改变
//---------------------------------------------------------------
VOID Ultimate::HandleTimeChange(const tagEventBase* pObj)
{
	MTransPtr(p, pObj, const tagEventTimeChange);

	if (p->ePeriod != ETP_Day)
		return;

	m_bChampionOnline = false;
}

//---------------------------------------------------------------
// 离线玩家登陆
//---------------------------------------------------------------
VOID Ultimate::HandleLoadOfflinePlayer(const tagEventBase* pObj)
{
	MTransPtrF(pEvent, pObj, tagEventLoadOfflinePlayer);

	Player* pPlayer = sPlayerMgr.GetPlayerByGUID(pEvent->n64PlayerID);
	if (!VALID(pPlayer))
		return;

	if (pEvent->dwReasonFlag & EPLR_UltimateArena)
	{
		GenArenaRecord(pPlayer);
	}
}

//---------------------------------------------------------------
// 生成竞技场镜像
//---------------------------------------------------------------
VOID Ultimate::GenArenaRecord(Player* pPlayer)
{
	tagGroupRecord	stFirstRecord;
	tagGroupRecord	stFollowRecord;
	pPlayer->CalPlayerScore();
	sSceneMgr.BuildGroupRecord(X_Ultimate_Arena_SceneID, pPlayer, &stFirstRecord);
	sSceneMgr.BuildFollowerGroupRecord(X_Ultimate_Arena_SceneID, pPlayer, &stFollowRecord);

	world_message::MWU_ArenaAddRecord stSend;
	FillProtoArenaRecord(pPlayer, stSend.mutable_record(), &stFirstRecord, &stFollowRecord);
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 初始化、更新、销毁
//---------------------------------------------------------------------------
BOOL Ultimate::Init()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	const tagUltimateConfig* pConfig = sServer.GetUltimateCfg();
	if (!VALID(pConfig))
		return FALSE;

	m_dwUltimateID = pConfig->dwID;
	m_dwPort = pConfig->dwPort;
	m_strIP = pConfig->strIP;
	m_strUltimateName = pConfig->strName;
	m_mapChampion.clear();
	m_bChampionOnline = false;
	m_nSeason = 0;

	InterlockedExchange((LPLONG)(&m_bConnected), FALSE);
	InterlockedExchange((LPLONG)(&m_bAuthed), FALSE);
	m_nTick = 0;
	m_dwUltimateTime = UCLOCK->CurrentClock();
	m_dwSyncTick = sServer.GetTick();
	m_bRecvHeartBeat = TRUE;

	// 初始化网络进行连接
	m_NetInterface.Init();
	m_NetInterface.TryConnect(m_strIP.c_str(), m_dwPort);

	m_mapProtoHandle.Clear();
	RegisterUltimateMsg();

	RegisterEventHandle(EE_TimeChange, &Ultimate::HandleTimeChange);
	RegisterEventHandle(EE_LoadOfflinePlayer, &Ultimate::HandleLoadOfflinePlayer);

	return TRUE;
}

VOID Ultimate::Update()
{
	UpdateConnection();
	UpdateMsg();
	UpdateOnfflineLoad();

	UpdateUI();
}

//-----------------------------------------------------------------------------
// 加载离线玩家
//-----------------------------------------------------------------------------
VOID Ultimate::UpdateOnfflineLoad()
{
	INT32 nLoadOneTick = 3;
	if (m_listOffLineLoad.Size() == 0) return;
	INT64 n64PlayerID = m_listOffLineLoad.PopFront();
	while (VALID(n64PlayerID))
	{
		Player* pPlayer = sPlayerMgr.GetPlayerByGUID(n64PlayerID);
		// 玩家在线
		if (VALID(pPlayer))
		{
			GenArenaRecord(pPlayer);
		}
		else
		{
			tagPlayerInfo* pInfo = sPlayerMgr.GetPlayerInfoByGUID(n64PlayerID);
			if (VALID(pInfo))
			{
				if (!sDBSession.CheakPlayerLoad(pInfo->dwAccountID))
				{
					tagRequestPlayerLoad stRequest;
					stRequest.dwAccountID = pInfo->dwAccountID;
					stRequest.n64PlayerID = n64PlayerID;
					stRequest.fRequestTime = Time::AccumSec();
					stRequest.bCreate = FALSE;
					stRequest.dwReasonFlag |= EPLR_UltimateArena;

					sDBSession.RequestForPlayerLoad(stRequest);
				}
				else
				{
					m_listOffLineLoad.PushBack(n64PlayerID);
				}

			}
		}

		if (nLoadOneTick-- <= 0)
		{
			break;
		}
		else
		{
			n64PlayerID = m_listOffLineLoad.PopFront();
		}
	}
}

VOID Ultimate::Destroy()
{
	UnRegisterUltimateMsg();
	google::protobuf::ShutdownProtobufLibrary();
}

VOID Ultimate::UpdateConnection()
{
	// 如果验证完成或者已经连接
	if (m_bAuthed || m_bConnected)
	{
		// 如果网络底层断开，说明要重连
		if (!m_NetInterface.IsConnected())
		{
			InterlockedExchange((LPLONG)&m_bConnected, FALSE);
			InterlockedExchange((LPLONG)&m_bAuthed, FALSE);
			m_NetInterface.TryConnect(m_strIP.c_str(), m_dwPort);

			ErrLog("Disconnect from UltimateServer! Try to connect...!\r\n");
		}
		else
		{
			++m_nTick;

			// 每5秒钟发送心跳包检测连接
			if (m_nTick >= 25)
			{
				if (!m_bRecvHeartBeat)
				{
					// 断线重连
					m_NetInterface.Disconnect();
				}

				SendHeartBeat();
				m_nTick = 0;
			}
		}
	}
	// 如果还没有建立连接
	else
	{
		// 如果底层已经连接上
		if (m_NetInterface.IsConnected())
		{
			InterlockedExchange((LPLONG)&m_bConnected, TRUE);
			m_bRecvHeartBeat = TRUE;

			// 发送第一条消息
			world_message::MWU_WorldLogon world_logon;
			world_logon.set_world_id((uint32)sServer.GetWorldID());
			world_logon.set_world_name(std::string(sServer.GetWorldName()));
			SendProtoMessage(&world_logon, world_logon.SerializeAsString());
		}
		// 底层不再尝试连接了，则再尝试
		else if (!m_NetInterface.IsTryingConnect())
		{
			InterlockedExchange((LPLONG)&m_bConnected, FALSE);
			InterlockedExchange((LPLONG)&m_bAuthed, FALSE);

			m_NetInterface.TryConnect(m_strIP.c_str(), m_dwPort);
		}
		// 底层还在连接，则再等等
		else
		{

		}
	}
}


VOID Ultimate::UpdateMsg()
{
	DWORD dwSize = 0;
	LPBYTE pMsg = NULL;

	while (TRUE)
	{
		// 接收消息
		pMsg = m_NetInterface.Recv(dwSize);
		if (!VALID(pMsg)) break;

		RecvMessage(pMsg, dwSize);

		// 返回
		m_NetInterface.FreeRecved(pMsg);
	}
}

//---------------------------------------------------------------------------
// console
//---------------------------------------------------------------------------
VOID Ultimate::UpdateUI()
{

}

//---------------------------------------------------------------------------
// 接收消息
//---------------------------------------------------------------------------
VOID Ultimate::RecvMessage(LPVOID pMsg, DWORD dwSize)
{
	if (dwSize < sizeof(tagNetCmd))
	{ 
		return;
	}

	DWORD dwID = *(DWORD*)pMsg;

	// proto message
	if (dwID == Crc32("MUW_DirectProtoMsg"))
	{
		google::protobuf::Message* pMessage = RecvProtoMessage((LPBYTE)pMsg + sizeof(tagNetCmd), dwSize);
		if (VALID(pMessage))
		{
			DWORD dwMsgID = Crc32(pMessage->GetTypeName().c_str());
			const tagProtoRecvInfo* pProtoInfo = m_mapProtoHandle.Peek(dwMsgID);
			if (VALID(pProtoInfo))
			{
				SEH_PROTECT_START	// 异常保护开始，注意此宏带花括号
				//------------------------------------------------------------------------------

				(this->*(pProtoInfo->handle))(pMessage);

				//------------------------------------------------------------------------------
				SEH_PROTECT_END		// 异常保护开始，注意此宏带花括号
			}

			SAFE_DEL(pMessage);
		}
	}

	// transfer message
	else if (dwID == Crc32("MWU_TransferMsg"))
	{
		RecvTransferMessage((LPBYTE)pMsg, dwSize);
	}

	else
	{ }
}

//---------------------------------------------------------------------------
// 接收proto message
//---------------------------------------------------------------------------
google::protobuf::Message* Ultimate::RecvProtoMessage(LPVOID pMsg, DWORD dwSize)
{
	// 2 bytes message name length + message name + proto data
	uint16 uNameLen = *(uint16*)pMsg;

	char* pMsgName = new char[uNameLen + 1];
	fxCore::fxStrncpy(pMsgName, (LPCTSTR)pMsg + sizeof(uint16), uNameLen + 1);

	DWORD dwSizeProtoData = dwSize - sizeof(uint16) - uNameLen;
	char* pProtoData = new char[dwSizeProtoData + 1];
	memcpy(pProtoData, (LPCTSTR)pMsg + sizeof(uint16) + uNameLen, dwSizeProtoData);
	pProtoData[dwSizeProtoData] = 0;

	const Descriptor* descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(string(pMsgName));
	if (!VALID(descriptor))
		return NULL;

	const google::protobuf::Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
	if (!VALID(prototype))
		return NULL;

	google::protobuf::Message* message = prototype->New();
	if (!VALID(message))
		return NULL;

	message->ParseFromArray(pProtoData, dwSizeProtoData);

	SAFE_DEL_ARRAY(pProtoData);
	SAFE_DEL_ARRAY(pMsgName);
	
	return message;
}

//---------------------------------------------------------------------------
// 接收转发消息
//---------------------------------------------------------------------------
DWORD Ultimate::RecvTransferMessage(LPVOID pMsg, DWORD dwSize)
{
	tagMWU_TransferMsg* pBaseMsg = (tagMWU_TransferMsg*)pMsg;

	if (sServer.GetWorldID() != pBaseMsg->dwWorldID)
	{
		ErrLog("Recv other world<id:%u> msg!\r\n", pBaseMsg->dwWorldID);
		return INVALID;
	}

	Player* pPlayer = sPlayerMgr.GetPlayerByGUID(pBaseMsg->n64PlayerID);
	if (!VALID(pPlayer))
		return INVALID;

	tagNetCmd *pCmd = (tagNetCmd*)pBaseMsg->byBuffer;
	DecodeProtoMsg(msg, pCmd);
	pPlayer->SendMessage(msg);

	return 0;
}

//---------------------------------------------------------------------------
// 发送心跳信息
//---------------------------------------------------------------------------
VOID Ultimate::SendHeartBeat()
{
	m_bRecvHeartBeat = FALSE;

	if (!m_bConnected || !m_bAuthed)
		return;

	world_message::MWU_HeartBeat stSend;
	SendProtoMessage(&stSend, stSend.SerializeAsString());

	/*
	CHAR szWorldTime[X_DATETIME_LEN + 1];
	DateTime2String(szWorldTime,X_DATETIME_LEN + 1,UCLOCK->CurrentClock());
	ErrLog("Send HeartBeat:%s at Tick:%d\r\n", szWorldTime, sServer.GetTick());
	*/
}

DWORD Ultimate::GetUltimateTime() const
{
	INT nTickDiff = sServer.GetTick() - m_dwSyncTick;
	INT nSecDiff = nTickDiff / TICK_PER_SECOND;

	DWORD dwRet = m_dwUltimateTime;

	if (nSecDiff > 0)
		dwRet = IncTime(m_dwUltimateTime, nTickDiff / TICK_PER_SECOND);

	/*
	CHAR szWorldTime[X_DATETIME_LEN + 1];
	CHAR szBattleTime[X_DATETIME_LEN + 1];
	DateTime2String(szWorldTime,X_DATETIME_LEN + 1,UCLOCK->CurrentClock());
	DateTime2String(szBattleTime,X_DATETIME_LEN + 1,m_dwBattleTime);

	ErrLog("Get Battle Time:%s|%s at Tick:%d|%d\r\n", szWorldTime, szBattleTime, sServer.GetTick(), nSecDiff);
	*/

	return dwRet;
}

VOID Ultimate::SendTransferMessage(DWORD dwWorldID, INT64 n64PlayerID, fxMessage& msg)
{
	// 如果是本服玩家，则直接发送
	if (!VALID(dwWorldID) || dwWorldID == sServer.GetWorldID())
	{
		Player* pPlayer = sPlayerMgr.GetPlayerByGUID(n64PlayerID);
		if (VALID(pPlayer))
		{
			pPlayer->SendMessage(msg);
		}

		return;
	}

	// 如果掉线或被踢掉，则不发送
	if (!m_bConnected)
		return;

	INT32 nSize = msg.GetSize() + sizeof(tagMWU_TransferMsg) + sizeof(tagNetCmd);
	CREATE_VAR_MSG(pSend, nSize, MWU_TransferMsg);
	pSend->dwWorldID = dwWorldID;
	pSend->n64PlayerID = n64PlayerID;
	tagNetCmd* pNetCmd = (tagNetCmd*)pSend->byBuffer;
	pNetCmd->dwID = msg.GetID();

	UINT32 uSize;
	if (msg.Encode((LPBYTE)pNetCmd + sizeof(tagNetCmd), msg.GetSize(), uSize))
	{
		ASSERT(uSize <= msg.GetSize());
		pNetCmd->dwSize = (DWORD)uSize + sizeof(tagNetCmd);

		// 调用底层发送
		SendMessage(pSend, pSend->dwSize);
 	}
}

//---------------------------------------------------------------------------
// 登陆反馈
//---------------------------------------------------------------------------
DWORD Ultimate::HandleWorldLogon(const google::protobuf::Message* pMsg)
{
	InterlockedExchange((LPLONG)(&m_bAuthed), TRUE);

	SendHeartBeat();

	// 通知BattlerServer连接成功
	std::set<DWORD>& setServerID = sServer.GetServerIDSet();
	world_message::MWU_WorldConnected stSend;
	for (std::set<DWORD>::iterator it = setServerID.begin(); it != setServerID.end(); ++it)
	{
		UINT32 u32WorldID = *it;
		stSend.add_world_id(u32WorldID);
	}
	SendProtoMessage(&stSend, stSend.SerializeAsString());

	MsgLog(_T("UltimateServer:%s is connected!\r\n"), GetUltimateName());

	return 0;
}

DWORD Ultimate::HandleTestConnect(const google::protobuf::Message* pMsg)
{
	world_message::MWU_TestConnect stSend;
	SendProtoMessage(&stSend, stSend.SerializeAsString());

	return 0;
}

DWORD Ultimate::HandleHeartBeat(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_HeartBeat);
	if (!VALID(pRecv))
		return 0;

	uint32 uUltimateTime = pRecv->battle_time();
	if (!VALID(uUltimateTime))
		return 0;

	tagDateTime stTime = GetCurDateTime((DWORD)uUltimateTime);
	m_dwUltimateTime = stTime;
	m_dwSyncTick = sServer.GetTick();
	m_bRecvHeartBeat = TRUE;

	/*
	CHAR szWorldTime[X_DATETIME_LEN + 1];
	CHAR szBattleTime[X_DATETIME_LEN + 1];
	DateTime2String(szWorldTime,X_DATETIME_LEN + 1,UCLOCK->CurrentClock());
	DateTime2String(szBattleTime,X_DATETIME_LEN + 1,m_dwBattleTime);

	ErrLog("Recv HeartBeat:%s|%s at Tick:%d\r\n", szWorldTime, szBattleTime, m_dwSyncTick);
	*/

	return 0;
}

DWORD Ultimate::HandleRequestPlayerInfo(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_RequestPlayerInfo);
	if (!VALID(pRecv))
		return 0;

	TList<const tagPlayerInfo*> listPlayerInfo;

	for (int32 n = pRecv->min_level(); n <= sPlayerMgr.GetMaxPlayerLevel(); n++)
	{
		SimpleVector<INT64> vecPlayerID = sPlayerMgr.GetPlayerLevelRef(n);

		if (vecPlayerID.Empty())
			continue;

		while (!vecPlayerID.Empty())
		{
			const tagPlayerInfo* pInfo = sPlayerMgr.GetPlayerInfoByGUID(vecPlayerID.Back());
			vecPlayerID.PopBack();

			if (!VALID(pInfo))
				continue;

			listPlayerInfo.PushBack(pInfo);
		}
	}

	INT32 nInfoNum = 0;
	world_message::MWU_RequestPlayerInfo* pSend = new world_message::MWU_RequestPlayerInfo;
	while (!listPlayerInfo.Empty())
	{
		const tagPlayerInfo* pInfo = listPlayerInfo.PopFront();
		if (!VALID(pInfo))
			continue;

		// 一次最多发送5000条
		if (nInfoNum++ >= 5000)
		{
			SendProtoMessage(pSend, pSend->SerializeAsString());
			SAFE_DEL(pSend);

			pSend = new world_message::MWU_RequestPlayerInfo;
			nInfoNum = 0;
		}
		
		world_message::CrossPlayerInfo* pSendPlayerInfo = pSend->add_info();
		if (VALID(pSendPlayerInfo))
		{
			Ultimate::FillProtoCrossPlayerInfo(pSendPlayerInfo, pInfo);
		}
	}
	
	SendProtoMessage(pSend, pSend->SerializeAsString());
	SAFE_DEL(pSend);

	return 0;
}

DWORD Ultimate::HandleRequestGuildInfo(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_RequestGuildInfo);
	if (!VALID(pRecv))
		return 0;

	TList<Guild*> listGuild;
	sGuildMgr.GetGuildList(listGuild);

	world_message::MWU_RequestGuildInfo* pSend = new world_message::MWU_RequestGuildInfo;
	INT32 nGuildNum = 0;
	while (!listGuild.Empty())
	{
		Guild* pGuild = listGuild.PopFront();
		if (!VALID(pGuild))
			continue;

		if (nGuildNum++ >= 5000)
		{
			SendProtoMessage(pSend, pSend->SerializeAsString());
			SAFE_DEL(pSend);

			world_message::MWU_RequestGuildInfo* pSend = new world_message::MWU_RequestGuildInfo;
		}

		world_message::CrossGuildInfo* pSendGuildInfo = pSend->add_info();
		if (VALID(pSendGuildInfo))
		{
			Ultimate::FillProtoCrossGuildInfo(pSendGuildInfo, pGuild);
		}
	}

	SendProtoMessage(pSend, pSend->SerializeAsString());
	SAFE_DEL(pSend);

	return 0;
}

//---------------------------------------------------------------------------
// 播放录像
//---------------------------------------------------------------------------
DWORD Ultimate::HandlePlayUltimateRecord(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_PlayUltimateRecord);
	if (!VALID(pRecv))
		return 0;

	PlayUltimateRecord(pRecv->src_player_id(), pRecv->src_server_id(), pRecv->record_id(), pRecv->dst_server_id());
	return 0;
}

//---------------------------------------------------------------------------
// 请求玩家信息
//---------------------------------------------------------------------------
DWORD Ultimate::HandleRequestUltimatePlayer(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_RequestUltimatePlayer);
	if (!VALID(pRecv))
		return 0;

	RequestUltimatePlayer(pRecv->src_player_id(), pRecv->src_server_id(), pRecv->dst_player_id(), pRecv->dst_server_id());
	return 0;
}

//---------------------------------------------------------------------------
// 请求玩家简易阵容
//---------------------------------------------------------------------------
DWORD Ultimate::HandleViewFormation(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ViewFormation);
	if (!VALID(pRecv))
		return 0;

	ViewFormation(pRecv->src_player_id(), pRecv->src_server_id(), pRecv->dst_player_id(), pRecv->dst_server_id());
	return 0;
}

VOID Ultimate::PlayUltimateRecord(INT64 n64SrcPlayerID, DWORD dwSrcServerID, INT64 n64RecordID, DWORD dwDstServerID)
{
	if (!VALID(n64SrcPlayerID) || !VALID(dwSrcServerID) || !VALID(n64RecordID) || !VALID(dwDstServerID))
		return;

	// 本服请求
	if (dwDstServerID == sServer.GetWorldID())
	{
		DWORD dwErrorCode = sRecordMgr.RequestRecord(dwSrcServerID, n64SrcPlayerID, n64RecordID);
		if (dwErrorCode != E_Success)
		{
			// 同步客户端录像过期
			CreateProtoMsg(msg, MS_PlayUltimateRecord, );
			msg << (UINT32)INVALID;
			SendTransferMessage(dwSrcServerID, n64SrcPlayerID, msg);
		}
	}

	// 跨服请求
	else
	{
		world_message::MWU_PlayUltimateRecord stSend;
		stSend.set_src_player_id(n64SrcPlayerID);
		stSend.set_src_server_id((uint32)dwSrcServerID);
		stSend.set_record_id(n64RecordID);
		stSend.set_dst_server_id((uint32)dwDstServerID);
		SendProtoMessage(&stSend, stSend.SerializeAsString());
	}
}

VOID Ultimate::RequestUltimatePlayer(INT64 n64SrcPlayerID, DWORD dwSrcServerID, INT64 n64DstPlayerID, DWORD dwDstServerID)
{
	if (!VALID(n64SrcPlayerID) || !VALID(dwSrcServerID) || !VALID(n64DstPlayerID))
		return;

	// 本服请求
	if (dwDstServerID == sServer.GetWorldID())
	{
		Player* pRemotePlayer = sPlayerMgr.GetPlayerByGUID(n64DstPlayerID);
		if (VALID(pRemotePlayer))
		{
			CreateProtoMsg(msgSend, MS_RequestRemotePlayer,);
			pRemotePlayer->BuildPlayerAllStuff(msgSend);
			SendTransferMessage(dwSrcServerID, n64SrcPlayerID, msgSend);
		}
	}

	// 跨服请求
	else
	{
		world_message::MWU_RequestUltimatePlayer stSend;
		stSend.set_src_player_id(n64SrcPlayerID);
		stSend.set_src_server_id((uint32)dwSrcServerID);
		stSend.set_dst_player_id(n64DstPlayerID);
		stSend.set_dst_server_id((uint32)dwDstServerID);
		SendProtoMessage(&stSend, stSend.SerializeAsString());
	}
}

VOID Ultimate::ViewFormation(INT64 n64SrcPlayerID, DWORD dwSrcServerID, INT64 n64DstPlayerID, DWORD dwDstServerID)
{
	if (!VALID(n64SrcPlayerID) || !VALID(dwSrcServerID) || !VALID(n64DstPlayerID))
		return;

	// 本服请求
	if (dwDstServerID == sServer.GetWorldID())
	{
		const tagPlayerInfo* pRemoteInfo = sPlayerMgr.GetPlayerInfoByGUID(n64DstPlayerID);
		if (VALID(pRemoteInfo))
		{
			CreateProtoMsg(msgSend, MS_ViewFormation, );
			sPlayerMgr.CreateFormationProto(msgSend, pRemoteInfo);
			SendTransferMessage(dwSrcServerID, n64SrcPlayerID, msgSend);
		}
	}

	// 跨服请求
	else
	{
		world_message::MWU_ViewFormation stSend;
		stSend.set_src_player_id(n64SrcPlayerID);
		stSend.set_src_server_id((uint32)dwSrcServerID);
		stSend.set_dst_player_id(n64DstPlayerID);
		stSend.set_dst_server_id((uint32)dwDstServerID);
		SendProtoMessage(&stSend, stSend.SerializeAsString());
	}
}

//---------------------------------------------------------------------------
// 竞技场开始匹配
//---------------------------------------------------------------------------
VOID Ultimate::ArenaMatching(Player* pPlayer)
{
	if (!VALID(pPlayer))
		return;

	if (!sAvyMgr.IsAvyStart(X_Ultimate_Avy_TypeID))
		return;

	world_message::MWU_ArenaMatching stSend;
	stSend.set_player_id(pPlayer->GetID());
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 竞技场战斗结果
//---------------------------------------------------------------------------
VOID Ultimate::OnArenaBattleResult(Player* pAttack, INT64 n64TargetID, bool bAttackWin)
{
	if (!VALID(pAttack))
		return;

	// 通知battle战斗结果
	world_message::MWU_ArenaBattleResult stSend;
	stSend.set_attack_id(pAttack->GetID());
	stSend.set_target_id(n64TargetID);
	stSend.set_attack_win(bAttackWin);
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 请求竞技场排名
//---------------------------------------------------------------------------
VOID Ultimate::RequestArenaRank(Player* pPlayer, INT32 nPage)
{
	if (!VALID(pPlayer))
		return;

	if (!sAvyMgr.IsAvyStart(X_Ultimate_Avy_TypeID))
		return;

	if (!MIsBetween(nPage, 0, 10))
		return;

	if (pPlayer->GetHeroContainer().GetMaster()->GetGilgulStep() == 0)
		return;

	// 请求排名
	world_message::MWU_RequestArenaRank stSend;
	stSend.set_player_id(pPlayer->GetID());
	stSend.set_page(nPage);
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 请求竞技场冠军
//---------------------------------------------------------------------------
VOID Ultimate::RequestArenaChampion(Player* pPlayer)
{
	if (!VALID(pPlayer))
		return;

	CreateProtoMsg(msg, MS_RequestUltimateArenaChampion, );
	msg << (INT32)m_mapChampion.size();

	for (auto it = m_mapChampion.begin(); it != m_mapChampion.end(); it++)
	{
		CreateProtoMsg(data, ArenaChampion, );
		data << it->second.nRank;
		data << it->second.n64PlayerID;
		data << it->second.nScore;
		data << string(it->second.szPlayerName);
		data << string(it->second.szServerName);
		data << (UINT32)it->second.dwMasterID;
		data << (INT32)it->second.nFashionID;

		msg << data;
	}

	pPlayer->SendMessage(msg);
}

//---------------------------------------------------------------------------
// 膜拜或领奖
//---------------------------------------------------------------------------
VOID Ultimate::ChampionWorship(Player* pPlayer)
{
	if (!VALID(pPlayer))
		return;

	if (m_mapChampion.size() == 0)
		return;

	if (IsInSameDay(pPlayer->GetUltimateController().GetWorshipTime(), UCLOCK->CurrentClock()))
		return;

	// 冠军领奖
	if (m_mapChampion.find(pPlayer->GetID()) != m_mapChampion.end())
	{
		pPlayer->GetAttController().ModAttValueWithLog(EPA_Thew, 30, ELCID_UltimateWorship);
		pPlayer->GetAttController().ModAttValueWithLog(EPA_Stamina, 20, ELCID_UltimateWorship);

		tagLootData stLoot;
		stLoot.eType = ELT_Currency;
		stLoot.dwTypeMisc = EMT_SeniorExploit;
		stLoot.nNum = 30;
		pPlayer->GainLoot(stLoot, ELCID_UltimateWorship);
	}

	// 膜拜冠军
	else
	{
		pPlayer->GetAttController().ModAttValueWithLog(EPA_Thew, 30, ELCID_UltimateWorship);
		pPlayer->GetAttController().ModAttValueWithLog(EPA_Stamina, 20, ELCID_UltimateWorship);

		tagLootData stLoot;
		stLoot.eType = ELT_Currency;
		stLoot.dwTypeMisc = EMT_SeniorExploit;
		stLoot.nNum = 30;
		pPlayer->GainLoot(stLoot, ELCID_UltimateWorship);
	}

	pPlayer->GetUltimateController().SetWorshipTime(UCLOCK->CurrentClock());
}

//---------------------------------------------------------------------------
// 冠军上线全服广播
//---------------------------------------------------------------------------
VOID Ultimate::ChampionOnline(Player* pPlayer)
{
	if (!VALID(pPlayer))
		return;

	auto it = m_mapChampion.find(pPlayer->GetID());
	if (it == m_mapChampion.end())
		return;

	if (it->second.nRank != 1)
		return;

	if (m_bChampionOnline)
		return;

	world_message::MWU_ArenaChampionOnline stSend;
	stSend.set_player_id(it->second.n64PlayerID);
	stSend.set_player_name(string(it->second.szPlayerName));
	stSend.set_server_name(string(it->second.szServerName));
	SendProtoMessage(&stSend, stSend.SerializeAsString());

	m_bChampionOnline = true;
}

//---------------------------------------------------------------------------
// 同步玩家信息
//---------------------------------------------------------------------------
VOID Ultimate::SyncPlayerInfo(const tagPlayerInfo* pInfo)
{
	if (!VALID(pInfo))
		return;

	world_message::MWU_ReplacePlayerInfo stSend;
	FillProtoCrossPlayerInfo(stSend.mutable_info(), pInfo);
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 同步帮会信息
//---------------------------------------------------------------------------
VOID Ultimate::SyncGuildInfo(Guild* pGuild)
{
	if (!VALID(pGuild))
		return;

	world_message::MWU_ReplaceGuildInfo stSend;
	FillProtoCrossGuildInfo(stSend.mutable_info(), pGuild);
	SendProtoMessage(&stSend, stSend.SerializeAsString());
}

//---------------------------------------------------------------------------
// 添加竞技场镜像
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaAddRecord(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaAddRecord);
	if (!VALID(pRecv))
		return 0;

	if (!VALID(pRecv->player_id()))
		return 0;

	const tagPlayerInfo* pInfo = sPlayerMgr.GetPlayerInfoByGUID(pRecv->player_id());
	if (!VALID(pInfo))
		return 0;

	Player* pPlayer = sPlayerMgr.GetPlayerByGUID(pRecv->player_id());
	if (!VALID(pPlayer))
	{
		tagRequestPlayerLoad stRequest;
		stRequest.dwAccountID = pInfo->dwAccountID;
		stRequest.n64PlayerID = pInfo->n64PlayerID;
		stRequest.fRequestTime = Time::AccumSec();
		stRequest.bCreate = FALSE;
		stRequest.dwReasonFlag |= EPLR_UltimateArena;
		sDBSession.RequestForPlayerLoad(stRequest);
		return 0;
	}

	GenArenaRecord(pPlayer);
	return 0;
}

//---------------------------------------------------------------------------
// 开始竞技场战斗
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaStartBattle(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaStartBattle);
	if (!VALID(pRecv))
		return 0;

	if (!VALID(pRecv->attack_id()))
		return 0;

	Player* pPlayer = sPlayerMgr.GetPlayerByGUID(pRecv->attack_id());
	if (!VALID(pPlayer))
		return 0;

	INT32 nTimes = pPlayer->GetUltimateController().GetArenaTimes();
	if (nTimes <= 0)
		return 0;

	tagArenaRecord stRecord;
	Ultimate::DecodeProtoToArenaRecord(&pRecv->target_record(), &stRecord);
	if (!stRecord.Valid())
		return 0;

	pPlayer->GetUltimateController().SetArenaTimes(nTimes - 1, FALSE);

	// 和机器人战斗
	if (pRecv->bot())
	{
		const tagUltimateArenaBotEntry* pBotEntry = sResMgr.RankPeekUltimateArenaBot();
		if (!VALID(pBotEntry))
			return 0;

		tagEnterSceneInfo stInfo;
		stInfo.dwTypeID = X_Ultimate_Arena_SceneID;
		stInfo.n64PlayerID = pPlayer->GetID();
		stInfo.n64Attack = pPlayer->GetID();
		stInfo.n64Defence = INVALID;
		stInfo.stMisc.ultimate_arena.n64AttackID = pPlayer->GetID();
		stInfo.stMisc.ultimate_arena.nRoundIndex[ESC_Attack] = 0;
		stInfo.stMisc.ultimate_arena.nRoundIndex[ESC_Defence] = 0;
		stInfo.stMisc.ultimate_arena.dwEntityGroupID = pBotEntry->dwEntityGroupID;
		stInfo.stMisc.ultimate_arena.dwPvp = 0;

		sSceneMgr.RequestEnterScene(stInfo);
	}

	// 和玩家战斗
	else
	{
		tagGroupRecord	stFirstRecord;
		tagGroupRecord	stFollowRecord;
		pPlayer->CalPlayerScore();
		sSceneMgr.BuildGroupRecord(X_Ultimate_Arena_SceneID, pPlayer, &stFirstRecord);
		sSceneMgr.BuildFollowerGroupRecord(X_Ultimate_Arena_SceneID, pPlayer, &stFollowRecord);

		tagCrossSceneData stData;
		stData.dwSceneTypeID = X_Ultimate_Arena_SceneID;
		stData.stMisc.ultimate_arena.n64AttackID = pPlayer->GetID();
		stData.stMisc.ultimate_arena.dwEntityGroupID = INVALID;
		stData.stMisc.ultimate_arena.dwPvp = 1;
		stData.stMisc.ultimate_arena.nRoundIndex[ESC_Attack] = 0;
		stData.stTeam[ESC_Attack].stGroupRecord[0] = stFirstRecord;
		stData.stTeam[ESC_Attack].stGroupRecord[1] = stFollowRecord;

		stData.stMisc.ultimate_arena.nRoundIndex[ESC_Defence] = 0;
		stData.stTeam[ESC_Defence].stGroupRecord[0] = stRecord.stFirstRecord;
		stData.stTeam[ESC_Defence].stGroupRecord[1] = stRecord.stFollowRecord;

		for (int i = 0; i < MAX_TEAM_LOOP; i++)
		{
			stData.dwSeed[i] = Rand();
		}

		sSceneMgr.RequestCrossTeamEnterScene(&stData);
	}

	return 0;
}

//---------------------------------------------------------------------------
// 请求竞技场排行
//---------------------------------------------------------------------------
DWORD Ultimate::HandleRequestArenaRank(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_RequestArenaRank);
	if (!VALID(pRecv))
		return 0;

	INT32 nPage = pRecv->page();
	Player* pPlayer = sPlayerMgr.GetPlayerByGUID(pRecv->player_id());
	if (!VALID(pPlayer))
		return 0;

	CreateProtoMsg(msg, MS_RequestUltimateArenaRank, INVALID);
	msg << nPage;
	msg << pRecv->score();
	msg << pRecv->rank() + 1;
	msg << (UINT32)((DWORD)GetCurDateTime((DWORD)pRecv->season_end_time()));

	msg << pRecv->infos_size();
	for (auto n = 0; n < pRecv->infos_size(); ++n)
	{
		tagArenaTargetInfo stInfo;
		Ultimate::DecodeProtoToArenaTargetInfo(&pRecv->infos(n), &stInfo);

		CreateProtoMsg(data, UltimateArenaTargetInfo, INVALID);
		data << stInfo.n64PlayerID;
		data << string(stInfo.szName);
		data << string(stInfo.szWorldName);
		data << stInfo.nLevel;
		data << stInfo.nPlayerScore;
		data << stInfo.nHeadProtrait;
		data << stInfo.nHeadQuality;
		data << stInfo.nArenaScore;

		msg << data;
	}

	pPlayer->SendMessage(msg);

	return 0;
}

//---------------------------------------------------------------------------
// 同步竞技场赛季信息
//---------------------------------------------------------------------------
DWORD Ultimate::HandleSyncArenaSeason(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_SyncArenaSeason);
	if (!VALID(pRecv))
		return 0;

	int32 nLastSeason = m_nSeason;
	m_nSeason = pRecv->season();
	m_uArenaSeasonEndTime = pRecv->end_time();

	// new season
	if (nLastSeason > 0 && m_nSeason > nLastSeason)
	{
		auto it = sPlayerMgr.Begin();
		Player* pPlayer = NULL;
		while (sPlayerMgr.PeekNext(it, pPlayer))
		{
			if (!VALID(pPlayer))
				continue;

			pPlayer->GetUltimateController().NewSeason(m_nSeason);
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// 每周段位奖励
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaWeeklyReward(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaWeeklyReward);
	if (!VALID(pRecv))
		return 0;

	for (auto n = 0; n < pRecv->data_size(); ++n)
	{
		INT64 n64PlayerID = pRecv->data(n).player_id();
		INT32 nScore = pRecv->data(n).score();

		tagPlayerInfo* pInfo = sPlayerMgr.GetPlayerInfoByGUID(n64PlayerID);
		if (!VALID(pInfo))
			continue;

		INT32 nSection = UltimateController::GetSectionByScore(nScore);
		const tagUltimateArenaWeeklyRewardEntry* pEntry = sResMgr.GetUltimateArenaWeeklyRewardEntry(nSection);
		if (!VALID(pEntry))
			continue;

		sMailMgr.SendUltimateArenaWeeklyReward(pInfo->n64PlayerID, nSection, pEntry->listLoot);
	}
	
	return 0;
}

//---------------------------------------------------------------------------
// 每月赛季排名奖励
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaSeasonReward(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaSeasonReward);
	if (!VALID(pRecv))
		return 0;

	tagPlayerInfo* pInfo = sPlayerMgr.GetPlayerInfoByGUID(pRecv->player_id());
	if (!VALID(pInfo))
		return 0;

	const tagUltimateArenaSeasonRewardEntry* pEntry = sResMgr.GetUltimateArenaSeasonRewardEntry(pRecv->rank());
	if (!VALID(pEntry))
		return 0;

	sMailMgr.SendUltimateArenaSeasonReward(pInfo->n64PlayerID, pRecv->rank(), pEntry->listLoot);
	return 0;
}

//---------------------------------------------------------------------------
// 同步竞技场冠军
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaChampion(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaChampion);
	if (!VALID(pRecv))
		return 0;

	m_mapChampion.clear();
	for (auto n = 0; n < pRecv->data_size(); ++n)
	{
		tagUltimateArenaChampion stChampion;
		const world_message::ArenaChampion& stProto = pRecv->data(n);

		stChampion.nRank = stProto.rank();
		stChampion.n64PlayerID = stProto.player_id();
		stChampion.nScore = stProto.score();
		strncpy_s(stChampion.szPlayerName, SHORT_STRING, stProto.player_name().c_str(), _TRUNCATE);
		strncpy_s(stChampion.szServerName, SHORT_STRING, stProto.server_name().c_str(), _TRUNCATE);
		stChampion.dwMasterID = stProto.master_id();
		stChampion.nFashionID = stProto.fashion_id();
		m_mapChampion.insert(make_pair(stChampion.n64PlayerID, stChampion));
	}

	return 0;
}

//---------------------------------------------------------------------------
// 竞技场冠军上线，全服广播
//---------------------------------------------------------------------------
DWORD Ultimate::HandleArenaChampionOnline(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_ArenaChampionOnline);
	if (!VALID(pRecv))
		return 0;

	CreateProtoMsg(msg, MS_UltimateChampionOnline, INVALID);
	msg << pRecv->player_name();
	msg << pRecv->server_name();
	sPlayerMgr.SendWorldMessage(msg);

	return 0;
}

//---------------------------------------------------------------------------
// 查询邀请是否有效
//---------------------------------------------------------------------------
DWORD Ultimate::HandleCheckInvite(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_CheckInvite);
	if (!VALID(pRecv))
		return 0;

	INT32 nErrorCode = sCenterInvite.CheckInviteResult(pRecv->inviter_id());

	world_message::MWU_CheckInviteResult stSend;
	stSend.set_inviter_id(pRecv->inviter_id());
	stSend.set_newbie_id(pRecv->newbie_id());
	stSend.set_error_code(nErrorCode);
	SendProtoMessage(&stSend, stSend.SerializeAsString());

	return 0;
}

//---------------------------------------------------------------------------
// 邀请结果
//---------------------------------------------------------------------------
DWORD Ultimate::HandleAddInviteResult(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_AddInviteResult);
	if (!VALID(pRecv))
		return 0;

	sCenterInvite.AddInviteResult(pRecv->newbie_id(), pRecv->inviter_id(), pRecv->error_code());
	return 0;
}

//---------------------------------------------------------------------------
// 新玩家充值
//---------------------------------------------------------------------------
DWORD Ultimate::HandleInviteRecharge(const google::protobuf::Message* pMsg)
{
	MTransPtr(pRecv, pMsg, const world_message::MUW_InviteRecharge);
	if (!VALID(pRecv))
		return 0;

	sCenterInvite.HandleNewbieRecharge(pRecv->newbie_id(), pRecv->newbie_name(), pRecv->inviter_id(), pRecv->diamond_gift());
	return 0;
}

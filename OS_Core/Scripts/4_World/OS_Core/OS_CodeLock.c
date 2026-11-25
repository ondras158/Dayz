// Rozšíření vanilla kódového zámku (DigitalCodeLock - brány plotů i dveře):
// - majitel = kdo nastavil kód; majitel a jeho skupina odemknou/zamknou bez kódu,
// - admin může zámek odemknout nebo smazat kód (admin nástroje),
// - ostatní dál zadávají kód jako ve vanille.

modded class DigitalCodeLock
{
	protected static const int OS_STORE_MAGIC = 784512779;
	static ref array<DigitalCodeLock> s_OS_All = new array<DigitalCodeLock>;   // jen server

	protected string m_OS_OwnerId;    // jen server
	protected int m_OS_OwnerHash;     // synchronizováno (hash Steam ID)
	protected int m_OS_GroupId;       // synchronizováno (skupina majitele)

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableInt("m_OS_OwnerHash");
		RegisterNetSyncVariableInt("m_OS_GroupId");
		if (g_Game.IsServer())
			s_OS_All.Insert(this);
	}

	void ~DigitalCodeLock()
	{
		if (s_OS_All)
			s_OS_All.RemoveItem(this);
	}

	string OS_GetOwnerId()
	{
		return m_OS_OwnerId;
	}

	bool OS_HasOwner()
	{
		return m_OS_OwnerHash != 0;
	}

	void OS_SetOwner(string steamId)
	{
		m_OS_OwnerId = steamId;
		m_OS_OwnerHash = 0;
		if (steamId != "")
			m_OS_OwnerHash = Transport.OS_HashId(steamId);
		OS_RefreshGroup();
	}

	//! skupina majitele (změní se, když majitel vstoupí do skupiny nebo z ní odejde)
	void OS_RefreshGroup()
	{
		int groupId = 0;
		if (m_OS_OwnerId != "" && OS_Server.Get() && OS_Server.Get().m_Groups)
		{
			OS_Group group = OS_Server.Get().m_Groups.GetGroupOf(m_OS_OwnerId);
			if (group)
				groupId = group.Id;
		}
		if (groupId != m_OS_GroupId)
		{
			m_OS_GroupId = groupId;
			SetSynchDirty();
		}
	}

	//! server: skutečná skupina majitele (ne uložená kopie)
	bool OS_CanAccessServer(string steamId)
	{
		if (m_OS_OwnerId == "" || steamId == "")
			return false;
		if (m_OS_OwnerId == steamId)
			return true;
		return OS_Server.Get() && OS_Server.Get().m_Groups && OS_Server.Get().m_Groups.SameGroup(steamId, m_OS_OwnerId);
	}

	bool OS_CanAccess(string steamId, int groupId)
	{
		if (!OS_HasOwner() || steamId == "")
			return false;
		if (Transport.OS_HashId(steamId) == m_OS_OwnerHash)
			return true;
		return groupId > 0 && groupId == m_OS_GroupId;
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		ctx.Write(OS_STORE_MAGIC);
		ctx.Write(m_OS_OwnerId);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		// starší uložení bez majitele - nic dalšího nečíst
		int magic;
		if (ctx.Read(magic) && magic == OS_STORE_MAGIC)
		{
			string ownerId;
			if (ctx.Read(ownerId))
				OS_SetOwner(ownerId);
		}
		return true;
	}

	//! nejbližší zámek k místu (server)
	static DigitalCodeLock OS_FindNearest(vector pos, float radius)
	{
		DigitalCodeLock best;
		float bestDist = radius;
		foreach (DigitalCodeLock codeLock : s_OS_All)
		{
			if (!codeLock)
				continue;
			float dist = vector.Distance(pos, codeLock.GetWorldPosition());
			if (dist <= bestDist)
			{
				bestDist = dist;
				best = codeLock;
			}
		}
		return best;
	}

	//! obnovit skupiny majitelů u všech zámků (server, občas)
	static void OS_RefreshAllGroups()
	{
		foreach (DigitalCodeLock codeLock : s_OS_All)
		{
			if (codeLock)
				codeLock.OS_RefreshGroup();
		}
	}
}

modded class CodeLockComponent
{
	//! první nastavení kódu = majitel zámku
	override void CheckCodeRequest(string pin, PlayerIdentity identity)
	{
		bool hadPin = m_HasPIN;
		super.CheckCodeRequest(pin, identity);

		if (!hadPin && m_HasPIN && identity && g_Game.IsServer())
		{
			DigitalCodeLock codeLock = DigitalCodeLock.Cast(m_Parent);
			if (codeLock)
				codeLock.OS_SetOwner(identity.GetPlainId());
		}
	}

	override void ResetPIN(bool sync = true)
	{
		super.ResetPIN(sync);
		DigitalCodeLock codeLock = DigitalCodeLock.Cast(m_Parent);
		if (codeLock && g_Game.IsServer())
			codeLock.OS_SetOwner("");
	}
}

// ---------------------------------------------------------------- akce na zámku

//! majitel / skupina: odemknout bez kódu
class ActionOSCodeLockUnlock : ActionInteractBase
{
	void ActionOSCodeLockUnlock()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "Odemknout (skupina)";
	}

	override string GetText()
	{
		return OS_T("Odemknout (skupina)");
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursorInherited({DigitalCodeLock}, UAMaxDistances.DEFAULT);
	}

	bool WantsLocked()
	{
		return false;
	}

	static bool HasAccess(PlayerBase player, DigitalCodeLock codeLock)
	{
		if (!player || !codeLock || !player.GetIdentity())
			return false;
		string id = player.GetIdentity().GetPlainId();
		if (g_Game.IsServer())
			return codeLock.OS_CanAccessServer(id);
		return codeLock.OS_CanAccess(id, OS_ClientSettings.GroupId);
	}

	//! zamknout jen zavřenou bránu
	static bool GateClosed(DigitalCodeLock codeLock)
	{
		Fence fence = Fence.Cast(codeLock.GetHierarchyParent());
		return !fence || !fence.IsOpened();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		DigitalCodeLock codeLock = DigitalCodeLock.Cast(target.GetObject());
		if (!codeLock || !codeLock.GetHierarchyParent() || !codeLock.HasPIN())
			return false;
		if (codeLock.IsLocked() == WantsLocked())
			return false;
		if (WantsLocked() && !GateClosed(codeLock))
			return false;
		return HasAccess(player, codeLock);
	}

	override void OnExecuteServer(ActionData action_data)
	{
		DigitalCodeLock codeLock = DigitalCodeLock.Cast(action_data.m_Target.GetObject());
		if (!codeLock || !HasAccess(action_data.m_Player, codeLock))
			return;
		if (WantsLocked() && !GateClosed(codeLock))
			return;

		codeLock.SetLockState(WantsLocked());
		if (WantsLocked())
			OS_Util.Notify(action_data.m_Player, OS_T("Zamek"), OS_T("Zamceno"), 3);
		else
			OS_Util.Notify(action_data.m_Player, OS_T("Zamek"), OS_T("Odemceno"), 3);
	}
}

//! majitel / skupina: zamknout bez kódu
class ActionOSCodeLockLock : ActionOSCodeLockUnlock
{
	void ActionOSCodeLockLock()
	{
		m_Text = "Zamknout (skupina)";
	}

	override string GetText()
	{
		return OS_T("Zamknout (skupina)");
	}

	override bool WantsLocked()
	{
		return true;
	}
}

// ---------------------------------------------------------------- akce na vlajce: menu base

class ActionOSBaseMenu : ActionInteractBase
{
	void ActionOSBaseMenu()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "Base";
	}

	override string GetText()
	{
		return OS_T("Base");
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		TerritoryFlag flag = TerritoryFlag.Cast(target.GetObject());
		if (!flag)
			return false;
		return IsInReach(player, target, UAMaxDistances.DEFAULT);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(null, OS_RPC_BASE_OPEN, true, null);
	}
}

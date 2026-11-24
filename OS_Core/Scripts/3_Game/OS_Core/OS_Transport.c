// Vlastnictví vozidel (auta, lodě, helikoptéry) podle Steam ID + sdílení se skupinou. Bez klíčů.
// Je na Transport = společném základu všech vozidel (CarScript, BoatScript, HelicopterScript...).
// Zamčené vozidlo: nejde nastoupit, otevřít dveře, sahat do nákladu ani sundávat díly.
// Zamknout/odemknout může majitel nebo člen skupiny, které je vozidlo přiřazené (klávesa F).

modded class Transport
{
	protected string m_OS_OwnerId;   // Steam64 ID majitele (jen server)
	protected int m_OS_OwnerHash;    // otisk ID majitele - synchronizuje se klientům (pro nabídku akcí)
	protected int m_OS_GroupId;      // skupina, se kterou je auto sdílené (0 = žádná)
	protected bool m_OS_Locked;

	protected static const int OS_STORE_MAGIC = 784512778;

	void Transport()
	{
		RegisterNetSyncVariableInt("m_OS_OwnerHash");
		RegisterNetSyncVariableInt("m_OS_GroupId");
		RegisterNetSyncVariableBool("m_OS_Locked");
	}

	static int OS_HashId(string steamId)
	{
		if (steamId == "")
			return 0;
		int h = steamId.Hash();
		if (h == 0)
			h = 1;
		return h;
	}

	bool OS_HasOwner()
	{
		return m_OS_OwnerHash != 0;
	}

	string OS_GetOwnerId()
	{
		return m_OS_OwnerId;
	}

	int OS_GetGroupId()
	{
		return m_OS_GroupId;
	}

	bool OS_IsLocked()
	{
		return m_OS_Locked;
	}

	//! majitel nebo člen skupiny auta (funguje na serveru i klientovi)
	bool OS_CanAccess(string steamId, int playerGroupId)
	{
		if (!OS_HasOwner())
			return false;
		if (OS_HashId(steamId) == m_OS_OwnerHash)
			return true;
		return m_OS_GroupId != 0 && m_OS_GroupId == playerGroupId;
	}

	//! server: nový majitel (koupě, přepsání v servisu). Sdílení se skupinou se ruší.
	void OS_SetOwner(string ownerId)
	{
		m_OS_OwnerId = ownerId;
		m_OS_OwnerHash = OS_HashId(ownerId);
		m_OS_GroupId = 0;
		if (ownerId == "")
			m_OS_Locked = false;
		SetSynchDirty();
	}

	//! server: sdílet se skupinou (0 = nesdílet)
	void OS_SetGroup(int groupId)
	{
		m_OS_GroupId = groupId;
		SetSynchDirty();
	}

	//! server
	void OS_SetLocked(bool locked)
	{
		if (!OS_HasOwner())
			locked = false;
		m_OS_Locked = locked;
		SetSynchDirty();
	}

	// ------------------------------------------------------------ omezení zamčeného auta

	override bool CanGetIn()
	{
		if (m_OS_Locked)
			return false;
		return super.CanGetIn();
	}

	override bool CanDisplayCargo()
	{
		if (m_OS_Locked)
			return false;
		return super.CanDisplayCargo();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (m_OS_Locked)
			return false;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		if (m_OS_Locked)
			return false;
		return super.CanReleaseCargo(cargo);
	}

	override bool CanReleaseAttachment(EntityAI attachment)
	{
		if (m_OS_Locked)
			return false;
		return super.CanReleaseAttachment(attachment);
	}

	// ------------------------------------------------------------ ukládání

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);

		ctx.Write(OS_STORE_MAGIC);
		ctx.Write(m_OS_OwnerId);
		ctx.Write(m_OS_GroupId);
		ctx.Write(m_OS_Locked);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		// auta uložená před instalací módu tato data nemají
		int magic;
		if (!ctx.Read(magic) || magic != OS_STORE_MAGIC)
			return true;

		string ownerId;
		int groupId;
		bool locked;
		if (ctx.Read(ownerId) && ctx.Read(groupId) && ctx.Read(locked))
		{
			m_OS_OwnerId = ownerId;
			m_OS_OwnerHash = OS_HashId(ownerId);
			m_OS_GroupId = groupId;
			m_OS_Locked = locked;
		}
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

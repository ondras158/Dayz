// Vlastnictví aut a zámky. Kód zámku a majitel se ukládají s autem (přežijí restart).
// Zamčené auto: nejde nastoupit, otevřít dveře, sahat do kufru ani sundávat díly.

modded class CarScript
{
	protected int m_OS_KeyCode;      // 0 = auto bez zámku / bez majitele
	protected bool m_OS_Locked;
	protected string m_OS_OwnerId;   // Steam64 ID majitele (jen server)

	protected static const int OS_STORE_MAGIC = 784512777;

	void CarScript()
	{
		RegisterNetSyncVariableInt("m_OS_KeyCode");
		RegisterNetSyncVariableBool("m_OS_Locked");
	}

	int OS_GetKeyCode()
	{
		return m_OS_KeyCode;
	}

	bool OS_IsLocked()
	{
		return m_OS_Locked;
	}

	string OS_GetOwnerId()
	{
		return m_OS_OwnerId;
	}

	bool OS_HasOwner()
	{
		return m_OS_KeyCode != 0;
	}

	//! server: nastaví nový zámek a majitele
	void OS_SetOwnership(int keyCode, string ownerId)
	{
		m_OS_KeyCode = keyCode;
		m_OS_OwnerId = ownerId;
		if (keyCode == 0)
			m_OS_Locked = false;
		SetSynchDirty();
	}

	//! server
	void OS_SetLocked(bool locked)
	{
		if (m_OS_KeyCode == 0)
			locked = false;
		m_OS_Locked = locked;
		SetSynchDirty();
	}

	static int OS_GenerateKeyCode()
	{
		return Math.RandomIntInclusive(100000, 999999999);
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
		ctx.Write(m_OS_KeyCode);
		ctx.Write(m_OS_Locked);
		ctx.Write(m_OS_OwnerId);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		// auta uložená před instalací módu tato data nemají
		int magic;
		if (!ctx.Read(magic) || magic != OS_STORE_MAGIC)
			return true;

		if (!ctx.Read(m_OS_KeyCode) || !ctx.Read(m_OS_Locked) || !ctx.Read(m_OS_OwnerId))
		{
			m_OS_KeyCode = 0;
			m_OS_Locked = false;
			m_OS_OwnerId = "";
		}
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

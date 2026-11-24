// Vlastnictví aut a zámky. Kód zámku a majitel se ukládají s autem (přežijí restart).
// Zamčené auto: nejde nastoupit, otevřít dveře, sahat do kufru ani sundávat díly.

modded class CarScript
{
	protected int m_BC_KeyCode;      // 0 = auto bez zámku / bez majitele
	protected bool m_BC_Locked;
	protected string m_BC_OwnerId;   // Steam64 ID majitele (jen server)

	protected static const int BC_STORE_MAGIC = 784512777;

	void CarScript()
	{
		RegisterNetSyncVariableInt("m_BC_KeyCode");
		RegisterNetSyncVariableBool("m_BC_Locked");
	}

	int BC_GetKeyCode()
	{
		return m_BC_KeyCode;
	}

	bool BC_IsLocked()
	{
		return m_BC_Locked;
	}

	string BC_GetOwnerId()
	{
		return m_BC_OwnerId;
	}

	bool BC_HasOwner()
	{
		return m_BC_KeyCode != 0;
	}

	//! server: nastaví nový zámek a majitele
	void BC_SetOwnership(int keyCode, string ownerId)
	{
		m_BC_KeyCode = keyCode;
		m_BC_OwnerId = ownerId;
		if (keyCode == 0)
			m_BC_Locked = false;
		SetSynchDirty();
	}

	//! server
	void BC_SetLocked(bool locked)
	{
		if (m_BC_KeyCode == 0)
			locked = false;
		m_BC_Locked = locked;
		SetSynchDirty();
	}

	static int BC_GenerateKeyCode()
	{
		return Math.RandomIntInclusive(100000, 999999999);
	}

	// ------------------------------------------------------------ omezení zamčeného auta

	override bool CanGetIn()
	{
		if (m_BC_Locked)
			return false;
		return super.CanGetIn();
	}

	override bool CanDisplayCargo()
	{
		if (m_BC_Locked)
			return false;
		return super.CanDisplayCargo();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (m_BC_Locked)
			return false;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		if (m_BC_Locked)
			return false;
		return super.CanReleaseCargo(cargo);
	}

	override bool CanReleaseAttachment(EntityAI attachment)
	{
		if (m_BC_Locked)
			return false;
		return super.CanReleaseAttachment(attachment);
	}

	// ------------------------------------------------------------ ukládání

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);

		ctx.Write(BC_STORE_MAGIC);
		ctx.Write(m_BC_KeyCode);
		ctx.Write(m_BC_Locked);
		ctx.Write(m_BC_OwnerId);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		// auta uložená před instalací módu tato data nemají
		int magic;
		if (!ctx.Read(magic) || magic != BC_STORE_MAGIC)
			return true;

		if (!ctx.Read(m_BC_KeyCode) || !ctx.Read(m_BC_Locked) || !ctx.Read(m_BC_OwnerId))
		{
			m_BC_KeyCode = 0;
			m_BC_Locked = false;
			m_BC_OwnerId = "";
		}
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

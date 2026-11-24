// Klíč od auta. Kód klíče musí odpovídat kódu zámku auta.

class OS_CarKey : Inventory_Base
{
	protected int m_OS_KeyCode;

	void OS_CarKey()
	{
		RegisterNetSyncVariableInt("m_OS_KeyCode");
	}

	int OS_GetKeyCode()
	{
		return m_OS_KeyCode;
	}

	//! server
	void OS_SetKeyCode(int code)
	{
		m_OS_KeyCode = code;
		SetSynchDirty();
	}

	bool OS_Fits(CarScript car)
	{
		return car && m_OS_KeyCode != 0 && car.OS_GetKeyCode() == m_OS_KeyCode;
	}

	override string GetDisplayName()
	{
		string name = super.GetDisplayName();
		if (m_OS_KeyCode != 0)
		{
			int shortCode = m_OS_KeyCode % 10000;
			name = name + " #" + shortCode.ToString();
		}
		return name;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionOSLockCar);
		AddAction(ActionOSUnlockCar);
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_OS_KeyCode);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		if (!ctx.Read(m_OS_KeyCode))
			m_OS_KeyCode = 0;
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

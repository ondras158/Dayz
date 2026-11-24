// Klíč od auta. Kód klíče musí odpovídat kódu zámku auta.

class BC_CarKey : Inventory_Base
{
	protected int m_BC_KeyCode;

	void BC_CarKey()
	{
		RegisterNetSyncVariableInt("m_BC_KeyCode");
	}

	int BC_GetKeyCode()
	{
		return m_BC_KeyCode;
	}

	//! server
	void BC_SetKeyCode(int code)
	{
		m_BC_KeyCode = code;
		SetSynchDirty();
	}

	bool BC_Fits(CarScript car)
	{
		return car && m_BC_KeyCode != 0 && car.BC_GetKeyCode() == m_BC_KeyCode;
	}

	override string GetDisplayName()
	{
		string name = super.GetDisplayName();
		if (m_BC_KeyCode != 0)
		{
			int shortCode = m_BC_KeyCode % 10000;
			name = name + " #" + shortCode.ToString();
		}
		return name;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionBCLockCar);
		AddAction(ActionBCUnlockCar);
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_BC_KeyCode);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		if (!ctx.Read(m_BC_KeyCode))
			m_BC_KeyCode = 0;
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

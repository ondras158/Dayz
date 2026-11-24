class OS_AdminMenu : OS_MenuBase
{
	protected ref array<string> m_Names = new array<string>;
	protected ref array<string> m_Ids = new array<string>;
	protected bool m_God;

	protected TextListboxWidget m_PlayerList;
	protected TextWidget m_Target;
	protected ButtonWidget m_BtnHeal;
	protected ButtonWidget m_BtnKill;
	protected ButtonWidget m_BtnTpTo;
	protected ButtonWidget m_BtnTpHere;
	protected ButtonWidget m_BtnGod;
	protected ButtonWidget m_BtnSpawn;
	protected ButtonWidget m_BtnMoney;
	protected ButtonWidget m_BtnTpCoords;
	protected ButtonWidget m_BtnUnlockCar;
	protected ButtonWidget m_BtnRefresh;
	protected ButtonWidget m_BtnClose;
	protected EditBoxWidget m_EditItem;
	protected EditBoxWidget m_EditArg;

	override string LayoutName()
	{
		return "admin_menu.layout";
	}

	void SetData(array<string> names, array<string> ids, bool god)
	{
		m_Names.Clear();
		m_Names.Copy(names);
		m_Ids.Clear();
		m_Ids.Copy(ids);
		m_God = god;

		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_PlayerList = FindList("PlayerList");
		m_Target = FindText("Target");
		m_BtnHeal = FindButton("BtnHeal");
		m_BtnKill = FindButton("BtnKill");
		m_BtnTpTo = FindButton("BtnTpTo");
		m_BtnTpHere = FindButton("BtnTpHere");
		m_BtnGod = FindButton("BtnGod");
		m_BtnSpawn = FindButton("BtnSpawn");
		m_BtnMoney = FindButton("BtnMoney");
		m_BtnTpCoords = FindButton("BtnTpCoords");
		m_BtnUnlockCar = FindButton("BtnUnlockCar");
		m_BtnRefresh = FindButton("BtnRefresh");
		m_BtnClose = FindButton("BtnClose");
		m_EditItem = FindEdit("EditItem");
		m_EditArg = FindEdit("EditArg");

		Refresh();
	}

	protected void Refresh()
	{
		if (!m_PlayerList)
			return;

		string previous = SelectedId();

		m_PlayerList.ClearItems();
		for (int i = 0; i < m_Names.Count(); i++)
		{
			m_PlayerList.AddItem(m_Names[i], null, 0);
			if (i < m_Ids.Count() && m_Ids[i] == previous)
				m_PlayerList.SelectRow(i);
		}

		if (m_BtnGod)
		{
			if (m_God)
				m_BtnGod.SetText("Godmode: ZAPNUTO");
			else
				m_BtnGod.SetText("Godmode: VYPNUTO");
		}
	}

	protected string SelectedId()
	{
		if (!m_PlayerList)
			return "";

		int row = m_PlayerList.GetSelectedRow();
		if (row >= 0 && row < m_Ids.Count())
			return m_Ids[row];
		return "";
	}

	protected string SelectedName()
	{
		int row = -1;
		if (m_PlayerList)
			row = m_PlayerList.GetSelectedRow();
		if (row >= 0 && row < m_Names.Count())
			return m_Names[row];
		return "";
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_Target)
		{
			string name = SelectedName();
			if (name == "")
				name = "ja (nikdo neni vybran)";
			m_Target.SetText("Cil: " + name);
		}
	}

	protected void SendAction(int action, string targetId, string arg)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(targetId);
		rpc.Write(arg);
		rpc.Send(null, OS_RPC_ADMIN_ACTION, true, null);
	}

	//! akce, které potřebují vybraného hráče
	protected bool RequireTarget()
	{
		if (SelectedId() != "")
			return true;
		SetStatus("Nejdriv vyber hrace v seznamu.");
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnHeal)
		{
			SendAction(OS_AdminAction.HEAL, SelectedId(), "");
			return true;
		}
		if (w == m_BtnKill)
		{
			if (RequireTarget())
				SendAction(OS_AdminAction.KILL, SelectedId(), "");
			return true;
		}
		if (w == m_BtnTpTo)
		{
			if (RequireTarget())
				SendAction(OS_AdminAction.TELEPORT_TO, SelectedId(), "");
			return true;
		}
		if (w == m_BtnTpHere)
		{
			if (RequireTarget())
				SendAction(OS_AdminAction.TELEPORT_HERE, SelectedId(), "");
			return true;
		}
		if (w == m_BtnGod)
		{
			SendAction(OS_AdminAction.TOGGLE_GOD, "", "");
			return true;
		}
		if (w == m_BtnSpawn && m_EditItem)
		{
			SendAction(OS_AdminAction.SPAWN_ITEM, SelectedId(), m_EditItem.GetText());
			return true;
		}
		if (w == m_BtnMoney && m_EditArg)
		{
			SendAction(OS_AdminAction.GIVE_MONEY, SelectedId(), m_EditArg.GetText());
			return true;
		}
		if (w == m_BtnTpCoords && m_EditArg)
		{
			SendAction(OS_AdminAction.TELEPORT_COORDS, "", m_EditArg.GetText());
			return true;
		}
		if (w == m_BtnUnlockCar)
		{
			SendAction(OS_AdminAction.UNLOCK_CAR, "", "");
			return true;
		}
		if (w == m_BtnRefresh)
		{
			OS_Client.SendToServer(OS_RPC_ADMIN_OPEN);
			return true;
		}
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}

		return super.OnClick(w, x, y, button);
	}
}

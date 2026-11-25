// Menu base u vlajky - pořadí čtení musí odpovídat OS_ServerBases.SendData

class OS_BaseData
{
	string Currency;
	bool HasBase;
	bool CanManage;
	bool IsOwner;
	string OwnerName;
	int Level;
	int MaxLevel;
	float Radius;
	float NextRadius;
	int NextPrice;
	int FoundPrice;
	bool FarmRequiresBase;

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Currency) || !ctx.Read(HasBase) || !ctx.Read(CanManage) || !ctx.Read(IsOwner) || !ctx.Read(OwnerName))
			return false;
		if (!ctx.Read(Level) || !ctx.Read(MaxLevel) || !ctx.Read(Radius) || !ctx.Read(NextRadius) || !ctx.Read(NextPrice))
			return false;
		if (!ctx.Read(FoundPrice) || !ctx.Read(FarmRequiresBase))
			return false;
		return true;
	}
}

class OS_BaseMenu : OS_MenuBase
{
	protected ref OS_BaseData m_Data;

	override string LayoutName()
	{
		return "base_menu.layout";
	}

	void SetData(OS_BaseData data)
	{
		m_Data = data;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		Refresh();
	}

	protected void SetVisible(string widgetName, bool visible)
	{
		Widget w = layoutRoot.FindAnyWidget(widgetName);
		if (w)
			w.Show(visible);
	}

	protected void SetLabel(string widgetName, string text)
	{
		TextWidget w = FindText(widgetName);
		if (w)
			w.SetText(text);
	}

	protected void Refresh()
	{
		if (!m_Data)
			return;

		string cur = " " + m_Data.Currency;
		int radius = m_Data.Radius;

		SetVisible("BtnFound", !m_Data.HasBase);
		SetVisible("BtnUpgrade", m_Data.HasBase && m_Data.CanManage);
		SetVisible("BtnRemove", m_Data.HasBase && m_Data.IsOwner);

		if (!m_Data.HasBase)
		{
			SetLabel("Info1", OS_T("U teto vlajky jeste neni base."));
			SetLabel("Info2", OS_F("Zalozenim ziskas oblast s polomerem %1 m kolem vlajky.", radius.ToString()));
			if (m_Data.FarmRequiresBase)
				SetLabel("Info3", OS_T("Farmu muzes postavit jen ve sve base."));
			else
				SetLabel("Info3", "");
			ButtonWidget found = FindButton("BtnFound");
			if (found)
			{
				if (m_Data.FoundPrice > 0)
					found.SetText(OS_F("Zalozit base (%1%2)", m_Data.FoundPrice.ToString(), cur));
				else
					found.SetText(OS_T("Zalozit base"));
			}
			return;
		}

		SetLabel("Info1", OS_F("Majitel: %1", m_Data.OwnerName));
		SetLabel("Info2", OS_F("Uroven %1 / %2   polomer %3 m", m_Data.Level.ToString(), m_Data.MaxLevel.ToString(), radius.ToString()));
		if (!m_Data.CanManage)
			SetLabel("Info3", OS_T("Tahle base patri jinemu hraci."));
		else
			SetLabel("Info3", OS_T("Spravovat ji muze majitel a jeho skupina."));

		ButtonWidget upgrade = FindButton("BtnUpgrade");
		if (upgrade)
		{
			if (m_Data.Level >= m_Data.MaxLevel)
			{
				upgrade.SetText(OS_T("Nejvetsi polomer"));
			}
			else
			{
				int nextRadius = m_Data.NextRadius;
				upgrade.SetText(OS_F("Zvetsit na %1 m (%2%3)", nextRadius.ToString(), m_Data.NextPrice.ToString(), cur));
			}
		}
	}

	protected void Send(int action)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Send(null, OS_RPC_BASE_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnFound")
			Send(OS_BaseAction.FOUND);
		else if (name == "BtnUpgrade")
			Send(OS_BaseAction.UPGRADE);
		else if (name == "BtnRemove")
			Send(OS_BaseAction.REMOVE);
		else if (name == "BtnClose")
			Close();
		else
			return super.OnClick(w, x, y, button);
		return true;
	}
}

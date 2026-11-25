// Menu dovedností (klávesa O) - pořadí čtení odpovídá OS_ServerSkills.SendData

class OS_SkillsMenu : OS_MenuBase
{
	protected int m_Level;
	protected int m_MaxLevel;
	protected int m_Xp;
	protected int m_Needed;
	protected int m_Points;
	protected int m_ResetPrice;
	protected string m_Currency;
	protected ref array<string> m_Ids = new array<string>;
	protected ref array<string> m_Names = new array<string>;
	protected ref array<string> m_Descriptions = new array<string>;
	protected ref array<int> m_Ranks = new array<int>;
	protected ref array<int> m_MaxRanks = new array<int>;
	protected ref array<string> m_Effects = new array<string>;
	protected ref array<string> m_NextEffects = new array<string>;
	protected TextListboxWidget m_List;
	protected int m_LastRow = -2;

	override string LayoutName()
	{
		return "skills_menu.layout";
	}

	bool ReadData(ParamsReadContext ctx)
	{
		if (!ctx.Read(m_Level) || !ctx.Read(m_MaxLevel) || !ctx.Read(m_Xp) || !ctx.Read(m_Needed) || !ctx.Read(m_Points) || !ctx.Read(m_ResetPrice) || !ctx.Read(m_Currency))
			return false;
		if (!ctx.Read(m_Ids) || !ctx.Read(m_Names) || !ctx.Read(m_Descriptions) || !ctx.Read(m_Ranks) || !ctx.Read(m_MaxRanks) || !ctx.Read(m_Effects) || !ctx.Read(m_NextEffects))
			return false;
		if (layoutRoot)
			Refresh();
		return true;
	}

	override void OnCreated()
	{
		m_List = FindList("SkillList");
		Refresh();
	}

	protected void SetLabel(string widgetName, string text)
	{
		TextWidget w = FindText(widgetName);
		if (w)
			w.SetText(text);
	}

	protected void Refresh()
	{
		if (!m_List)
			return;

		if (m_Needed > 0)
			SetLabel("LevelText", OS_F("Uroven %1 / %2   zkusenosti %3 / %4", m_Level.ToString(), m_MaxLevel.ToString(), m_Xp.ToString(), m_Needed.ToString()));
		else
			SetLabel("LevelText", OS_F("Uroven %1 / %2 (maximum)", m_Level.ToString(), m_MaxLevel.ToString()));
		SetLabel("PointsText", OS_F("Body k rozdeleni: %1", m_Points.ToString()));

		ButtonWidget reset = FindButton("BtnReset");
		if (reset)
		{
			reset.Show(m_ResetPrice >= 0);
			if (m_ResetPrice > 0)
				reset.SetText(OS_F("Prerozdelit body (%1 %2)", m_ResetPrice.ToString(), m_Currency));
			else
				reset.SetText(OS_T("Prerozdelit body"));
		}

		int previous = m_List.GetSelectedRow();
		m_List.ClearItems();
		for (int i = 0; i < m_Names.Count(); i++)
		{
			string label = OS_F("%1   %2 / %3   (%4)", m_Names[i], m_Ranks[i].ToString(), m_MaxRanks[i].ToString(), m_Effects[i]);
			m_List.AddItem(label, null, 0);
		}
		if (previous >= 0 && previous < m_Names.Count())
			m_List.SelectRow(previous);
		m_LastRow = -2;
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (!m_List)
			return;
		int row = m_List.GetSelectedRow();
		if (row == m_LastRow)
			return;
		m_LastRow = row;

		if (row < 0 || row >= m_Names.Count())
		{
			SetLabel("Detail1", OS_T("Vyber dovednost v seznamu."));
			SetLabel("Detail2", "");
			return;
		}
		SetLabel("Detail1", m_Descriptions[row]);
		if (m_Ranks[row] >= m_MaxRanks[row])
			SetLabel("Detail2", OS_F("Ted: %1   (maximum)", m_Effects[row]));
		else
			SetLabel("Detail2", OS_F("Ted: %1   ->   dalsi uroven: %2", m_Effects[row], m_NextEffects[row]));
	}

	protected void Send(string skillId)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(skillId);
		rpc.Send(null, OS_RPC_SKILLS_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnUpgrade")
		{
			int row = -1;
			if (m_List)
				row = m_List.GetSelectedRow();
			if (row < 0 || row >= m_Ids.Count())
			{
				SetStatus(OS_T("Vyber dovednost v seznamu."));
				return true;
			}
			Send(m_Ids[row]);
			return true;
		}
		if (name == "BtnReset")
		{
			Send("#reset");
			return true;
		}
		if (name == "BtnClose")
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}
}

// Menu vaření u ohně - pořadí čtení odpovídá OS_ServerCooking.SendData

class OS_CookMenu : OS_MenuBase
{
	protected ref array<string> m_Names = new array<string>;
	protected ref array<string> m_Details = new array<string>;
	protected ref array<int> m_Ready = new array<int>;
	protected TextListboxWidget m_List;
	protected int m_LastRow = -2;

	override string LayoutName()
	{
		return "cook_menu.layout";
	}

	bool ReadData(ParamsReadContext ctx)
	{
		array<string> names = new array<string>;
		array<string> details = new array<string>;
		array<int> ready = new array<int>;
		if (!ctx.Read(names) || !ctx.Read(details) || !ctx.Read(ready))
			return false;
		m_Names = names;
		m_Details = details;
		m_Ready = ready;
		if (layoutRoot)
			Refresh();
		return true;
	}

	override void OnCreated()
	{
		m_List = FindList("RecipeList");
		Refresh();
	}

	protected void Refresh()
	{
		if (!m_List)
			return;
		int previous = m_List.GetSelectedRow();
		m_List.ClearItems();
		for (int i = 0; i < m_Names.Count(); i++)
		{
			string label = m_Names[i];
			if (i < m_Ready.Count() && m_Ready[i] == 1)
				label = OS_F("%1   [lze uvarit]", label);
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

		// řádky detailu: suroviny | nádobí a čas | bonus
		array<string> lines = new array<string>;
		if (row >= 0 && row < m_Details.Count())
		{
			string text = m_Details[row];
			text.Replace("   |   ", "|");
			text.Split("|", lines);
		}
		else
		{
			lines.Insert(OS_T("Vyber recept vlevo."));
		}
		for (int i = 1; i <= 3; i++)
		{
			TextWidget line = FindText("Detail" + i.ToString());
			if (!line)
				continue;
			if (i <= lines.Count())
				line.SetText(lines[i - 1]);
			else
				line.SetText("");
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnCook")
		{
			int row = -1;
			if (m_List)
				row = m_List.GetSelectedRow();
			if (row < 0)
			{
				SetStatus(OS_T("Vyber recept vlevo."));
				return true;
			}
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(row);
			rpc.Send(null, OS_RPC_COOK_ACTION, true, null);
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

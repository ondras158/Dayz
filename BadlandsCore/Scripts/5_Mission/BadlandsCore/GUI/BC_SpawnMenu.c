class BC_SpawnMenu : BC_MenuBase
{
	protected ref array<string> m_Zones = new array<string>;
	protected float m_TimeLeft = 60;
	protected bool m_Sent;

	protected TextListboxWidget m_List;
	protected TextWidget m_Countdown;
	protected ButtonWidget m_BtnSpawn;
	protected ButtonWidget m_BtnStay;

	void SetData(array<string> zones, int timeout)
	{
		m_Zones.Copy(zones);
		m_TimeLeft = timeout;
	}

	override string LayoutName()
	{
		return "spawn_menu.layout";
	}

	override void OnCreated()
	{
		m_List = FindList("ZoneList");
		m_Countdown = FindText("Countdown");
		m_BtnSpawn = FindButton("BtnSpawn");
		m_BtnStay = FindButton("BtnStay");

		if (m_List)
		{
			foreach (string zone : m_Zones)
			{
				m_List.AddItem(zone, null, 0);
			}
			if (m_Zones.Count() > 0)
				m_List.SelectRow(0);
		}
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		m_TimeLeft -= timeslice;
		if (m_Countdown)
		{
			int seconds = Math.Ceil(m_TimeLeft);
			if (seconds < 0)
				seconds = 0;
			m_Countdown.SetText("Zbyva " + seconds.ToString() + " s - pak zustanes na aktualnim miste");
		}

		if (m_TimeLeft <= 0)
			Choose(-1);
	}

	override void OnBack()
	{
		Choose(-1);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnSpawn)
		{
			if (m_List && m_List.GetSelectedRow() >= 0)
				Choose(m_List.GetSelectedRow());
			return true;
		}

		if (w == m_BtnStay)
		{
			Choose(-1);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	//! -1 = zůstat na vanilla spawnu
	protected void Choose(int index)
	{
		if (m_Sent)
			return;
		m_Sent = true;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(index);
		rpc.Send(null, BC_RPC_SPAWN_CHOICE, true, null);

		Close();
	}
}

// Menu osobní garáže (klávesa K): postavit garáž, nastavit místo pro vozidlo, zrušit.

class OS_PersonalGarageMenu : OS_MenuBase
{
	protected bool m_Enabled;
	protected int m_Price;
	protected string m_Currency;
	protected bool m_HasGarage;
	protected vector m_GaragePos;
	protected float m_Radius;

	protected TextWidget m_Info;
	protected TextWidget m_Info2;
	protected ButtonWidget m_BtnBuild;
	protected ButtonWidget m_BtnSetSpawn;
	protected ButtonWidget m_BtnRemove;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "pgarage_menu.layout";
	}

	void SetData(bool enabled, int price, string currency, bool hasGarage, vector pos, float radius)
	{
		m_Enabled = enabled;
		m_Price = price;
		m_Currency = currency;
		m_HasGarage = hasGarage;
		m_GaragePos = pos;
		m_Radius = radius;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_Info = FindText("Info");
		m_Info2 = FindText("Info2");
		m_BtnBuild = FindButton("BtnBuild");
		m_BtnSetSpawn = FindButton("BtnSetSpawn");
		m_BtnRemove = FindButton("BtnRemove");
		m_BtnClose = FindButton("BtnClose");
		Refresh();
	}

	protected float DistanceToGarage()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !m_HasGarage)
			return 99999;
		vector p = player.GetPosition();
		float dx = p[0] - m_GaragePos[0];
		float dz = p[2] - m_GaragePos[2];
		return Math.Sqrt(dx * dx + dz * dz);
	}

	protected void Refresh()
	{
		bool near = DistanceToGarage() <= m_Radius;

		if (!m_Enabled)
		{
			SetLabel(m_Info, "Osobni garaze jsou na serveru vypnute.");
			SetLabel(m_Info2, "");
		}
		else if (m_HasGarage)
		{
			int dist = Math.Round(DistanceToGarage());
			int gx = Math.Round(m_GaragePos[0]);
			int gz = Math.Round(m_GaragePos[2]);
			SetLabel(m_Info, "Tvoje garaz: " + gx.ToString() + " / " + gz.ToString() + "   (" + dist.ToString() + " m)");
			if (near)
				SetLabel(m_Info2, "Jsi u garaze - klavesa B = ulozit / vyzvednout vozidlo.");
			else
				SetLabel(m_Info2, "Garaz je i na mape. Dojdi k ni a pouzij klavesu B.");
		}
		else
		{
			SetLabel(m_Info, "Nemas osobni garaz. Postav ji tam, kde stojis.");
			SetLabel(m_Info2, "Cena: " + m_Price.ToString() + " " + m_Currency);
		}

		ShowW(m_BtnBuild, m_Enabled && !m_HasGarage);
		ShowW(m_BtnSetSpawn, m_Enabled && m_HasGarage && near);
		ShowW(m_BtnRemove, m_HasGarage);
		if (m_BtnBuild)
			m_BtnBuild.SetText("Postavit garaz zde (" + m_Price.ToString() + " " + m_Currency + ")");
	}

	protected void SetLabel(TextWidget w, string text)
	{
		if (w)
			w.SetText(text);
	}

	protected void ShowW(Widget w, bool visible)
	{
		if (w)
			w.Show(visible);
	}

	protected void SendAction(int action)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Send(null, OS_RPC_PGARAGE_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnBuild)
		{
			SendAction(OS_PersonalGarageAction.BUILD);
			return true;
		}
		if (w == m_BtnRemove)
		{
			SendAction(OS_PersonalGarageAction.REMOVE);
			return true;
		}
		if (w == m_BtnSetSpawn)
		{
			Close();
			OS_Client.Get().StartPlacer(OS_SpawnTarget.PERSONAL_GARAGE, "car", false, false, vector.Zero, 0);
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

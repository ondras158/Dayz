// HUD v rohu obrazovky: stav hráče, zóna a PvP, spoluhráči, souřadnice / směr / čas.
// Stav a zónu posílá server (OS_ServerHud), výdrž, poloha, směr a čas jsou z klienta.

class OS_Hud
{
	protected Widget m_Root;
	protected Widget m_Bg;
	protected Widget m_StatusBox;
	protected Widget m_GroupBox;
	protected Widget m_InfoBox;
	protected bool m_Hidden;          // skryto klávesou
	protected float m_SlowTimer = 10;

	protected static const float WIDTH = 300;
	protected static const float BAR_WIDTH = 140;

	void ToggleHidden()
	{
		m_Hidden = !m_Hidden;
	}

	protected void Create()
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "hud.layout");
		if (!m_Root)
			return;
		m_Bg = m_Root.FindAnyWidget("Bg");
		m_StatusBox = m_Root.FindAnyWidget("StatusBox");
		m_GroupBox = m_Root.FindAnyWidget("GroupBox");
		m_InfoBox = m_Root.FindAnyWidget("InfoBox");

		SetLabel("L_Health", OS_T("Zdravi"));
		SetLabel("L_Blood", OS_T("Krev"));
		SetLabel("L_Food", OS_T("Jidlo"));
		SetLabel("L_Water", OS_T("Voda"));
		SetLabel("L_Stamina", OS_T("Vydrz"));
		SetLabel("L_Heat", OS_T("Teplo"));
		SetLabel("GroupTitle", OS_T("Skupina"));
	}

	void Destroy()
	{
		if (m_Root)
			m_Root.Unlink();
		m_Root = null;
	}

	protected TextWidget Text(string name)
	{
		return TextWidget.Cast(m_Root.FindAnyWidget(name));
	}

	protected void SetLabel(string name, string text, int color = 0)
	{
		TextWidget w = Text(name);
		if (!w)
			return;
		w.SetText(text);
		if (color != 0)
			w.SetColor(color);
		w.SetTextOutline(1);
	}

	//! pruh 0..1, barva podle hodnoty (nebo pevná)
	protected void SetBar(string key, float value, string text, int color = 0)
	{
		Widget fill = m_Root.FindAnyWidget("BF_" + key);
		if (fill)
		{
			float width = BAR_WIDTH * Math.Clamp(value, 0, 1);
			fill.SetSize(Math.Max(1, width), 10);
			if (color == 0)
			{
				if (value > 0.6)
					color = ARGB(240, 80, 200, 90);
				else if (value > 0.3)
					color = ARGB(240, 230, 190, 50);
				else
					color = ARGB(240, 230, 60, 50);
			}
			fill.SetColor(color);
		}
		SetLabel("V_" + key, text);
	}

	protected string Percent(float value)
	{
		int pct = Math.Round(Math.Clamp(value, 0, 1) * 100);
		return pct.ToString() + " %";
	}

	protected string Direction(float yaw)
	{
		// 0 = sever, po směru hodinových ručiček
		float angle = yaw;
		while (angle < 0)
			angle += 360;
		while (angle >= 360)
			angle -= 360;
		int index = Math.Round(angle / 45);
		if (index >= 8)
			index = 0;
		int deg = Math.Round(angle);
		return DirectionName(index) + " " + deg.ToString() + "°";
	}

	protected string DirectionName(int index)
	{
		if (index == 0)
			return OS_T("S");
		if (index == 1)
			return OS_T("SV");
		if (index == 2)
			return OS_T("V");
		if (index == 3)
			return OS_T("JV");
		if (index == 4)
			return OS_T("J");
		if (index == 5)
			return OS_T("JZ");
		if (index == 6)
			return OS_T("Z");
		return OS_T("SZ");
	}

	void Update(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		bool visible = OS_ClientHud.Enabled && OS_ClientHud.Received && !m_Hidden && player && player.IsAlive() && !g_Game.GetUIManager().GetMenu();
		if (!visible)
		{
			if (m_Root)
				m_Root.Show(false);
			return;
		}
		if (!m_Root)
		{
			Create();
			if (!m_Root)
				return;
		}
		m_Root.Show(true);

		// rozložení bloků pod sebe
		float y = 0;
		if (m_StatusBox)
		{
			m_StatusBox.Show(OS_ClientHud.ShowStatus);
			if (OS_ClientHud.ShowStatus)
			{
				m_StatusBox.SetPos(0, y);
				y += 196;
			}
		}
		int groupCount = OS_ClientGroup.Count();
		bool showGroup = OS_ClientHud.ShowGroup && groupCount > 0;
		if (m_GroupBox)
		{
			m_GroupBox.Show(showGroup);
			if (showGroup)
			{
				m_GroupBox.SetPos(0, y);
				int lines = Math.Min(groupCount, 6);
				y += 26 + lines * 19;
			}
		}
		if (m_InfoBox)
		{
			m_InfoBox.Show(OS_ClientHud.ShowInfo);
			if (OS_ClientHud.ShowInfo)
			{
				m_InfoBox.SetPos(0, y);
				y += 46;
			}
		}
		if (m_Bg)
			m_Bg.SetSize(WIDTH, y + 4);

		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		if (OS_ClientHud.Left)
			m_Root.SetPos(20, 120);
		else
			m_Root.SetPos(screenW - WIDTH - 20, 120);

		if (OS_ClientHud.ShowStatus)
			UpdateStatus(player);
		if (showGroup)
			UpdateGroup(player, groupCount);
		if (OS_ClientHud.ShowInfo)
			UpdateInfo(player);
	}

	protected void UpdateStatus(PlayerBase player)
	{
		// zóna
		int zoneColor = ARGB(255, 200, 200, 200);
		string zoneText = OS_T("PvE - hraci ti neublizi");
		if (OS_ClientHud.ZoneType == 1)
		{
			zoneColor = ARGB(255, 90, 220, 110);
			zoneText = OS_F("BEZPECNA ZONA - %1", OS_ClientHud.ZoneName);
		}
		else if (OS_ClientHud.ZoneType == 2)
		{
			zoneColor = ARGB(255, 240, 70, 60);
			zoneText = OS_F("PvP ZONA - %1", OS_ClientHud.ZoneName);
		}
		else if (OS_ClientHud.ZoneType == 3)
		{
			zoneColor = ARGB(255, 245, 160, 50);
			zoneText = OS_F("PvP jeste %1 s", OS_ClientHud.PvPSeconds.ToString());
		}
		else if (OS_ClientHud.ZoneType == 4)
		{
			zoneColor = ARGB(255, 240, 70, 60);
			zoneText = OS_T("PvP - pozor na hrace");
		}
		SetLabel("Zone", zoneText, zoneColor);
		SetLabel("BaseText", OS_ClientHud.BaseText, ARGB(255, 120, 170, 255));

		SetBar("Health", OS_ClientHud.Health, Percent(OS_ClientHud.Health));
		SetBar("Blood", OS_ClientHud.Blood, Percent(OS_ClientHud.Blood), ARGB(240, 200, 40, 40));
		SetBar("Food", OS_ClientHud.Food, Percent(OS_ClientHud.Food));
		SetBar("Water", OS_ClientHud.Water, Percent(OS_ClientHud.Water), ARGB(240, 70, 150, 240));

		float stamina = 1;
		StaminaHandler handler = player.GetStaminaHandler();
		if (handler && handler.GetStaminaCap() > 0)
			stamina = handler.GetStamina() / handler.GetStaminaCap();
		SetBar("Stamina", stamina, Percent(stamina), ARGB(240, 230, 220, 120));

		// teplo: -1 zima .. 0 v pohodě .. 1 horko
		float heat = OS_ClientHud.Heat;
		string heatText = OS_T("OK");
		int heatColor = ARGB(240, 80, 200, 90);
		if (heat < -0.45)
		{
			heatText = OS_T("Zima");
			heatColor = ARGB(240, 70, 120, 240);
		}
		else if (heat < -0.15)
		{
			heatText = OS_T("Chladno");
			heatColor = ARGB(240, 120, 180, 240);
		}
		else if (heat > 0.45)
		{
			heatText = OS_T("Horko");
			heatColor = ARGB(240, 230, 60, 50);
		}
		else if (heat > 0.15)
		{
			heatText = OS_T("Teplo");
			heatColor = ARGB(240, 230, 160, 50);
		}
		SetBar("Heat", (heat + 1) * 0.5, heatText, heatColor);
	}

	protected void UpdateGroup(PlayerBase player, int count)
	{
		vector myPos = player.GetPosition();
		for (int i = 0; i < 6; i++)
		{
			string line = "";
			if (i < count)
			{
				int meters = Math.Round(vector.Distance(myPos, OS_ClientGroup.GetPosition(i)));
				string distance = meters.ToString() + " m";
				if (meters >= 1000)
				{
					float km = Math.Round(meters / 100.0) / 10.0;
					distance = km.ToString() + " km";
				}
				line = OS_ClientGroup.Names[i] + "   " + distance;
			}
			SetLabel("G" + i.ToString(), line, ARGB(255, 120, 200, 255));
		}
	}

	protected void UpdateInfo(PlayerBase player)
	{
		vector pos = player.GetPosition();
		// směr pohledu kamery (ne těla)
		vector camDir = g_Game.GetCurrentCameraDirection();
		vector ori = camDir.VectorToAngles();

		int year;
		int month;
		int day;
		int hour;
		int minute;
		g_Game.GetWorld().GetDate(year, month, day, hour, minute);
		string minuteText = minute.ToString();
		if (minute < 10)
			minuteText = "0" + minuteText;

		string line1 = Direction(ori[0]) + "   " + hour.ToString() + ":" + minuteText;
		if (OS_ClientHud.ShowCoordinates)
		{
			int px = Math.Round(pos[0]);
			int pz = Math.Round(pos[2]);
			line1 = "X " + px.ToString() + "  Z " + pz.ToString() + "   " + line1;
		}
		SetLabel("Info1", line1);
		SetLabel("Info2", OS_F("Hracu online: %1", OS_ClientHud.PlayersOnline.ToString()));
	}
}

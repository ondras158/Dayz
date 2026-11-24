// Jména spoluhráčů nad hlavou (vykresluje se každý snímek).

class OS_NameTags
{
	protected ref array<Widget> m_Tags = new array<Widget>;
	protected ref array<TextWidget> m_Texts = new array<TextWidget>;

	protected static const float TAG_WIDTH = 300;
	protected static const float TAG_HEIGHT = 40;

	protected void EnsureTags(int count)
	{
		while (m_Tags.Count() < count)
		{
			Widget root = g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "nametag.layout");
			if (!root)
				return;
			m_Tags.Insert(root);
			m_Texts.Insert(TextWidget.Cast(root.FindAnyWidget("Text")));
			root.Show(false);
		}
	}

	protected void HideFrom(int index)
	{
		for (int i = index; i < m_Tags.Count(); i++)
		{
			m_Tags[i].Show(false);
		}
	}

	void Update()
	{
		PlayerBase me = PlayerBase.Cast(g_Game.GetPlayer());
		int count = OS_ClientGroup.Count();

		// skrýt: vypnuto, nikdo, mrtvý, otevřené menu (inventář, mapa...)
		if (!OS_ClientGroup.ShowNameTags || count == 0 || !me || !me.IsAlive() || g_Game.GetUIManager().GetMenu())
		{
			HideFrom(0);
			return;
		}

		EnsureTags(count);

		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		vector myPos = me.GetPosition();

		int shown = 0;
		for (int i = 0; i < count && shown < m_Tags.Count(); i++)
		{
			vector worldPos;
			PlayerBase entity = OS_ClientGroup.GetEntity(i);
			if (entity)
			{
				int head = entity.GetBoneIndex("Head");
				if (head >= 0)
					worldPos = entity.GetBonePositionWS(head) + Vector(0, 0.35, 0);
				else
					worldPos = entity.GetPosition() + Vector(0, 2.0, 0);
			}
			else
			{
				worldPos = OS_ClientGroup.Positions[i] + Vector(0, 2.0, 0);
			}

			float dist = vector.Distance(myPos, worldPos);
			if (dist > OS_ClientGroup.NameTagMaxDistance)
				continue;

			vector screen = g_Game.GetScreenPosRelative(worldPos);
			if (screen[2] <= 0 || screen[0] < 0 || screen[0] > 1 || screen[1] < 0 || screen[1] > 1)
				continue;

			string label = OS_ClientGroup.Names[i];
			if (OS_ClientGroup.NameTagShowDistance)
			{
				int meters = Math.Round(dist);
				label = label + "  " + meters.ToString() + " m";
			}

			Widget tag = m_Tags[shown];
			TextWidget text = m_Texts[shown];
			if (text)
				text.SetText(label);

			// blízko = výrazné, daleko = průhlednější
			float alpha = Math.Clamp(1.2 - dist / OS_ClientGroup.NameTagMaxDistance, 0.45, 1.0);
			tag.SetAlpha(alpha);
			tag.SetPos(screen[0] * screenW - TAG_WIDTH / 2, screen[1] * screenH - TAG_HEIGHT);
			tag.Show(true);
			shown++;
		}

		HideFrom(shown);
	}

	void Destroy()
	{
		foreach (Widget w : m_Tags)
		{
			if (w)
				w.Unlink();
		}
		m_Tags.Clear();
		m_Texts.Clear();
	}
}

// značky spoluhráčů na mapě
modded class MapMenu
{
	protected float m_OS_MarkerTimer;
	protected bool m_OS_HasMarkers;

	override Widget Init()
	{
		Widget root = super.Init();
		m_OS_MarkerTimer = 10;   // vykreslit hned v prvním Update
		return root;
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (!m_MapWidgetInstance)
			return;

		m_OS_MarkerTimer += timeslice;
		if (m_OS_MarkerTimer < 1.0)
			return;
		m_OS_MarkerTimer = 0;

		int count = OS_ClientGroup.Count();
		if (!OS_ClientGroup.ShowMapMarkers)
			count = 0;
		bool garage = OS_ClientSettings.HasPersonalGarage;
		if (count == 0 && !garage && !m_OS_HasMarkers)
			return;

		// obnovit značky: vlastní značky z mapy + spoluhráči
		m_MapWidgetInstance.ClearUserMarks();
		if (m_Map)
			LoadMapMarkers();

		int color = ARGB(255, 60, 200, 255);
		for (int i = 0; i < count; i++)
		{
			m_MapWidgetInstance.AddUserMark(OS_ClientGroup.GetPosition(i), OS_ClientGroup.Names[i], color, "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa");
		}
		if (garage)
		{
			vector gp = OS_ClientSettings.PersonalGaragePos;
			gp[1] = g_Game.SurfaceY(gp[0], gp[2]);
			m_MapWidgetInstance.AddUserMark(gp, "Moje garaz", ARGB(255, 255, 190, 40), "\\DZ\\gear\\navigation\\data\\map_station_ca.paa");
		}
		m_OS_HasMarkers = count > 0 || garage;
	}
}

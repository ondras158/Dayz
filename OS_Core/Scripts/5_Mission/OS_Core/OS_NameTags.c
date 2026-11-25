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

// značky spoluhráčů na mapě, PvP a bezpečné zóny, otevření mapy klávesou bez mapy v inventáři
modded class MapMenu
{
	static bool s_OS_OpenedByKey;     // nastaví MissionGameplay hned po otevření klávesou
	static float s_OS_OpenedAt;

	protected float m_OS_MarkerTimer;
	protected bool m_OS_HasMarkers;
	protected bool m_OS_ByKey;
	protected CanvasWidget m_OS_ZoneCanvas;

	override Widget Init()
	{
		Widget root = super.Init();
		m_OS_MarkerTimer = 10;   // vykreslit hned v prvním Update
		m_OS_ByKey = false;
		if (m_MapWidgetInstance)
			m_OS_ZoneCanvas = CanvasWidget.Cast(g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "mapzones.layout", m_MapWidgetInstance));
		return root;
	}

	override void OnHide()
	{
		super.OnHide();

		// otevřeno naší klávesou: vrátit ovládání (vanilla to dělá jen s ignoreMapOwnership)
		if (m_OS_ByKey)
		{
			m_OS_ByKey = false;
			if (!CfgGameplayHandler.GetMapIgnoreMapOwnership() && g_Game && g_Game.GetMission())
			{
				g_Game.GetMission().RemoveActiveInputExcludes({"map"});
				g_Game.GetMission().RemoveActiveInputRestriction(EInputRestrictors.MAP);
			}
		}
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (s_OS_OpenedByKey)
		{
			s_OS_OpenedByKey = false;
			m_OS_ByKey = true;
		}

		if (!m_MapWidgetInstance)
			return;

		// stejnou klávesou mapu zavřít
		if (m_OS_ByKey && g_Game.GetTickTime() - s_OS_OpenedAt > 0.3)
		{
			UAInput mapInput = GetUApi().GetInputByName("UAOSMap");
			if (mapInput && mapInput.LocalPress())
			{
				PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
				if (player)
					player.SetLastMapInfo(m_MapWidgetInstance.GetScale(), m_MapWidgetInstance.GetMapPos());
				CloseMapMenu();
				return;
			}
		}

		OS_DrawZones();

		m_OS_MarkerTimer += timeslice;
		if (m_OS_MarkerTimer < 1.0)
			return;
		m_OS_MarkerTimer = 0;

		int count = OS_ClientGroup.Count();
		if (!OS_ClientGroup.ShowMapMarkers)
			count = 0;
		bool garage = OS_ClientSettings.HasPersonalGarage;
		bool farm = OS_ClientSettings.HasFarm;
		int zones = OS_ClientZones.Count();
		if (count == 0 && !garage && !farm && zones == 0 && !m_OS_HasMarkers)
			return;

		// obnovit značky: vlastní značky z mapy + spoluhráči + zóny
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
			m_MapWidgetInstance.AddUserMark(gp, OS_T("Moje garaz"), ARGB(255, 255, 190, 40), "\\DZ\\gear\\navigation\\data\\map_station_ca.paa");
		}
		if (farm)
		{
			vector fp = OS_ClientSettings.FarmPos;
			fp[1] = g_Game.SurfaceY(fp[0], fp[2]);
			m_MapWidgetInstance.AddUserMark(fp, OS_T("Moje farma"), ARGB(255, 120, 220, 80), "\\DZ\\gear\\navigation\\data\\map_vineyard_ca.paa");
		}
		for (int p = 0; p < OS_ClientZones.PvPNames.Count(); p++)
		{
			vector pvpPos = Vector(OS_ClientZones.PvPX[p], 0, OS_ClientZones.PvPZ[p]);
			pvpPos[1] = g_Game.SurfaceY(pvpPos[0], pvpPos[2]);
			m_MapWidgetInstance.AddUserMark(pvpPos, "PvP: " + OS_ClientZones.PvPNames[p], OS_ClientZones.PvPColor, "\\DZ\\gear\\navigation\\data\\map_police_ca.paa");
		}
		for (int s = 0; s < OS_ClientZones.SafeNames.Count(); s++)
		{
			vector safePos = Vector(OS_ClientZones.SafeX[s], 0, OS_ClientZones.SafeZ[s]);
			safePos[1] = g_Game.SurfaceY(safePos[0], safePos[2]);
			m_MapWidgetInstance.AddUserMark(safePos, OS_ClientZones.SafeNames[s], OS_ClientZones.SafeColor, "\\DZ\\gear\\navigation\\data\\map_tourism_ca.paa");
		}
		for (int bm = 0; bm < OS_ClientZones.BaseNames.Count(); bm++)
		{
			vector basePos = Vector(OS_ClientZones.BaseX[bm], 0, OS_ClientZones.BaseZ[bm]);
			basePos[1] = g_Game.SurfaceY(basePos[0], basePos[2]);
			m_MapWidgetInstance.AddUserMark(basePos, OS_ClientZones.BaseNames[bm], OS_ClientZones.BaseColor, "\\DZ\\gear\\navigation\\data\\map_camp_ca.paa");
		}
		for (int dm = 0; dm < OS_ClientZones.DropNames.Count(); dm++)
		{
			vector dropPos = Vector(OS_ClientZones.DropX[dm], 0, OS_ClientZones.DropZ[dm]);
			dropPos[1] = g_Game.SurfaceY(dropPos[0], dropPos[2]);
			m_MapWidgetInstance.AddUserMark(dropPos, OS_ClientZones.DropNames[dm], ARGB(255, 255, 120, 30), "\\DZ\\gear\\navigation\\data\\map_tsign_ca.paa");
		}
		m_OS_HasMarkers = count > 0 || garage || farm || zones > 0;
	}

	//! kruhy zón - každý snímek, protože se mapa posouvá a zoomuje
	protected void OS_DrawZones()
	{
		if (!m_OS_ZoneCanvas)
			return;
		m_OS_ZoneCanvas.Clear();
		if (OS_ClientZones.Count() == 0)
			return;

		float mapW;
		float mapH;
		m_MapWidgetInstance.GetScreenSize(mapW, mapH);
		float mapX;
		float mapY;
		m_MapWidgetInstance.GetScreenPos(mapX, mapY);

		// MapToScreen vrací souřadnice buď vůči mapě, nebo vůči obrazovce - zjistit podle středu mapy
		vector center = m_MapWidgetInstance.MapToScreen(m_MapWidgetInstance.GetMapPos());
		float offX = 0;
		float offY = 0;
		if (Math.AbsFloat(center[0] - (mapX + mapW * 0.5)) < Math.AbsFloat(center[0] - mapW * 0.5) && mapX > 1)
		{
			offX = -mapX;
			offY = -mapY;
		}

		for (int i = 0; i < OS_ClientZones.PvPNames.Count(); i++)
		{
			OS_DrawCircle(OS_ClientZones.PvPX[i], OS_ClientZones.PvPZ[i], OS_ClientZones.PvPR[i], OS_ClientZones.PvPColor, offX, offY, mapW, mapH);
		}
		for (int j = 0; j < OS_ClientZones.SafeNames.Count(); j++)
		{
			OS_DrawCircle(OS_ClientZones.SafeX[j], OS_ClientZones.SafeZ[j], OS_ClientZones.SafeR[j], OS_ClientZones.SafeColor, offX, offY, mapW, mapH);
		}
		for (int b = 0; b < OS_ClientZones.BaseNames.Count(); b++)
		{
			OS_DrawCircle(OS_ClientZones.BaseX[b], OS_ClientZones.BaseZ[b], OS_ClientZones.BaseR[b], OS_ClientZones.BaseColor, offX, offY, mapW, mapH);
		}
	}

	protected void OS_DrawCircle(float x, float z, float radius, int color, float offX, float offY, float maxX, float maxY)
	{
		vector c = m_MapWidgetInstance.MapToScreen(Vector(x, 0, z));
		vector e = m_MapWidgetInstance.MapToScreen(Vector(x + radius, 0, z));
		float r = e[0] - c[0];
		if (r < 2)
			return;

		float cx = c[0] + offX;
		float cy = c[1] + offY;
		int segments = 64;
		float prevX = cx + r;
		float prevY = cy;
		for (int k = 1; k <= segments; k++)
		{
			float angle = Math.PI2 * k / segments;
			float px = cx + Math.Cos(angle) * r;
			float py = cy + Math.Sin(angle) * r;
			if (OS_InRect(prevX, prevY, maxX, maxY) && OS_InRect(px, py, maxX, maxY))
			{
				m_OS_ZoneCanvas.DrawLine(prevX, prevY, px, py, 3, color);
			}
			prevX = px;
			prevY = py;
		}
	}

	protected bool OS_InRect(float x, float y, float maxX, float maxY)
	{
		return x >= 0 && y >= 0 && x <= maxX && y <= maxY;
	}
}

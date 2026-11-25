// Admin nástroje (podobně jako Community Online Tools):
// teleport dvojklikem do mapy, spawner předmětů a vozidel, čas a počasí,
// ESP hráčů, neviditelnost, volná kamera, auta, vysvlečení, kick a ban.
// Všechno se ověřuje na serveru (Admins.json) - klient jen posílá požadavky.

//! stav admina na klientovi
class OS_ClientAdmin
{
	static bool IsAdmin;
	static string SelectedId;      // vybraný hráč v admin menu ("" = admin sám)
	static string SelectedName;
	static bool Esp;

	static void SendAction(int action, string targetId, string arg)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(targetId);
		rpc.Write(arg);
		rpc.Send(null, OS_RPC_ADMIN_ACTION, true, null);
	}

	static void OnFlag(ParamsReadContext ctx)
	{
		bool isAdmin;
		if (ctx.Read(isAdmin))
			IsAdmin = isAdmin;
		if (!IsAdmin)
			Esp = false;
	}

	static string TargetLabel()
	{
		if (SelectedId == "" || SelectedName == "")
			return OS_T("ja (nikdo neni vybran)");
		return SelectedName;
	}
}

// ---------------------------------------------------------------- ESP

//! jména a vzdálenost všech hráčů v dosahu (i přes zdi)
class OS_AdminESP
{
	protected ref array<Widget> m_Tags = new array<Widget>;
	protected ref array<TextWidget> m_Texts = new array<TextWidget>;
	protected static const float TAG_WIDTH = 300;
	protected static const float TAG_HEIGHT = 40;
	protected static const float MAX_DISTANCE = 1500;

	protected void HideFrom(int index)
	{
		for (int i = index; i < m_Tags.Count(); i++)
		{
			m_Tags[i].Show(false);
		}
	}

	protected bool EnsureTag(int index)
	{
		while (m_Tags.Count() <= index)
		{
			Widget root = g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "nametag.layout");
			if (!root)
				return false;
			TextWidget text = TextWidget.Cast(root.FindAnyWidget("Text"));
			if (text)
				text.SetColor(ARGB(255, 255, 90, 70));
			m_Tags.Insert(root);
			m_Texts.Insert(text);
			root.Show(false);
		}
		return true;
	}

	void Update()
	{
		PlayerBase me = PlayerBase.Cast(g_Game.GetPlayer());
		if (!OS_ClientAdmin.Esp || !OS_ClientAdmin.IsAdmin || !me || g_Game.GetUIManager().GetMenu())
		{
			HideFrom(0);
			return;
		}

		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		vector myPos = me.GetPosition();
		Camera cam = Camera.GetCurrentCamera();
		if (cam)
			myPos = cam.GetPosition();

		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);

		int shown = 0;
		foreach (Man man : players)
		{
			PlayerBase other = PlayerBase.Cast(man);
			if (!other || other == me)
				continue;

			vector worldPos = other.GetPosition() + Vector(0, 2.1, 0);
			float dist = vector.Distance(myPos, worldPos);
			if (dist > MAX_DISTANCE)
				continue;

			vector screen = g_Game.GetScreenPosRelative(worldPos);
			if (screen[2] <= 0 || screen[0] < 0 || screen[0] > 1 || screen[1] < 0 || screen[1] > 1)
				continue;

			if (!EnsureTag(shown))
				break;

			string name = "?";
			if (other.GetIdentity())
				name = other.GetIdentity().GetName();
			int meters = Math.Round(dist);
			string label = name + "  " + meters.ToString() + " m";
			if (!other.IsAlive())
				label = OS_F("%1 [mrtvy]", label);

			if (m_Texts[shown])
				m_Texts[shown].SetText(label);
			m_Tags[shown].SetPos(screen[0] * screenW - TAG_WIDTH / 2, screen[1] * screenH - TAG_HEIGHT);
			m_Tags[shown].Show(true);
			shown++;
		}
		HideFrom(shown);
	}
}

// ---------------------------------------------------------------- volná kamera

//! WASD = pohyb, myš = rozhlížení, Q/E = dolů/nahoru, Shift = rychle; klávesa End = zapnout/vypnout
//! (Shift + End při vypnutí = teleport na místo kamery)
class OS_AdminFreecam
{
	protected Camera m_Camera;
	protected float m_Yaw;
	protected float m_Pitch;

	bool IsActive()
	{
		return m_Camera != null;
	}

	void Toggle()
	{
		if (IsActive())
			Stop(InputValue("UATurbo") > 0 || InputValue("UAWalkRunTemp") > 0);
		else
			Start();
	}

	void Start()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !OS_ClientAdmin.IsAdmin)
			return;

		vector pos = player.GetPosition() + Vector(0, 2, 0);
		m_Camera = Camera.Cast(g_Game.CreateObject("staticcamera", pos, true));
		if (!m_Camera)
			return;

		vector ori = player.GetOrientation();
		m_Yaw = ori[0];
		m_Pitch = 0;
		m_Camera.SetOrientation(Vector(m_Yaw, m_Pitch, 0));
		m_Camera.SetActive(true);
		if (player.GetInputController())
			player.GetInputController().SetDisabled(true);
	}

	void Stop(bool teleport)
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (m_Camera)
		{
			if (teleport)
			{
				vector camPos = m_Camera.GetPosition();
				int camX = Math.Round(camPos[0]);
				int camZ = Math.Round(camPos[2]);
				OS_ClientAdmin.SendAction(OS_AdminAction.TELEPORT_MAP, "", camX.ToString() + " " + camZ.ToString());
			}
			m_Camera.SetActive(false);
			g_Game.ObjectDelete(m_Camera);
			m_Camera = null;
		}
		if (player && player.GetInputController())
			player.GetInputController().SetDisabled(false);
	}

	protected float InputValue(string name)
	{
		UAInput input = GetUApi().GetInputByName(name);
		if (!input)
			return 0;
		return input.LocalValue();
	}

	void Update(float timeslice)
	{
		if (!m_Camera)
			return;

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsAlive() || !OS_ClientAdmin.IsAdmin)
		{
			Stop(false);
			return;
		}
		if (g_Game.GetUIManager().GetMenu())
			return;

		float sensitivity = 2.5;
		m_Yaw = m_Yaw + (InputValue("UAAimRight") - InputValue("UAAimLeft")) * sensitivity;
		m_Pitch = m_Pitch + (InputValue("UAAimUp") - InputValue("UAAimDown")) * sensitivity;
		m_Pitch = Math.Clamp(m_Pitch, -89, 89);

		float speed = 12;
		if (InputValue("UATurbo") > 0 || InputValue("UAWalkRunTemp") > 0)
			speed = 60;

		vector forward = Vector(m_Yaw, m_Pitch, 0).AnglesToVector();
		vector right = Vector(m_Yaw + 90, 0, 0).AnglesToVector();
		float moveF = InputValue("UAMoveForward") - InputValue("UAMoveBack");
		float moveR = InputValue("UAMoveRight") - InputValue("UAMoveLeft");
		float moveU = InputValue("UALeanRight") - InputValue("UALeanLeft");

		vector pos = m_Camera.GetPosition();
		pos = pos + forward * (moveF * speed * timeslice) + right * (moveR * speed * timeslice) + Vector(0, moveU * speed * timeslice, 0);
		float ground = g_Game.SurfaceY(pos[0], pos[2]) + 0.3;
		if (pos[1] < ground)
			pos[1] = ground;

		m_Camera.SetPosition(pos);
		m_Camera.SetOrientation(Vector(m_Yaw, m_Pitch, 0));
	}
}

// ---------------------------------------------------------------- teleport z mapy

modded class MapHandler
{
	//! admin: dvojklik do mapy = teleport vybraného hráče (nebo sebe)
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		bool handled = super.OnDoubleClick(w, x, y, button);
		if (!OS_ClientAdmin.IsAdmin)
			return handled;

		MapWidget map = MapWidget.Cast(w);
		if (!map)
			return handled;

		vector pos = map.ScreenToMap(Vector(x, y, 0));
		int px = Math.Round(pos[0]);
		int pz = Math.Round(pos[2]);
		OS_ClientAdmin.SendAction(OS_AdminAction.TELEPORT_MAP, OS_ClientAdmin.SelectedId, px.ToString() + " " + pz.ToString());
		return true;
	}
}

// ---------------------------------------------------------------- menu nástrojů

class OS_AdminToolsMenu : OS_MenuBase
{
	protected EditBoxWidget m_EditReason;
	protected EditBoxWidget m_EditTime;
	protected TextWidget m_Target;

	override string LayoutName()
	{
		return "admin_tools.layout";
	}

	override void OnCreated()
	{
		m_EditReason = FindEdit("EditReason");
		m_EditTime = FindEdit("EditTime");
		m_Target = FindText("Target");
		RefreshButtons();
	}

	protected void RefreshButtons()
	{
		ButtonWidget esp = FindButton("BtnEsp");
		if (esp)
		{
			if (OS_ClientAdmin.Esp)
				esp.SetText(OS_T("ESP: ZAPNUTO"));
			else
				esp.SetText(OS_T("ESP: VYPNUTO"));
		}
		ButtonWidget freecam = FindButton("BtnFreecam");
		if (freecam && OS_Client.Get())
		{
			if (OS_Client.Get().GetFreecam().IsActive())
				freecam.SetText(OS_T("Freecam: ZAPNUTO"));
			else
				freecam.SetText(OS_T("Freecam: VYPNUTO"));
		}
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (m_Target)
			m_Target.SetText(OS_F("Cil: %1", OS_ClientAdmin.TargetLabel()));
	}

	protected string EditText(EditBoxWidget edit)
	{
		if (!edit)
			return "";
		return edit.GetText();
	}

	protected void Send(int action, string arg = "")
	{
		OS_ClientAdmin.SendAction(action, OS_ClientAdmin.SelectedId, arg);
	}

	protected bool NeedTarget()
	{
		if (OS_ClientAdmin.SelectedId != "")
			return true;
		SetStatus(OS_T("Nejdriv vyber hrace v admin menu (Home)."));
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();

		if (name == "BtnStrip")
		{
			if (NeedTarget())
				Send(OS_AdminAction.STRIP);
		}
		else if (name == "BtnInvis")
		{
			OS_ClientAdmin.SendAction(OS_AdminAction.TOGGLE_INVIS, "", "");
		}
		else if (name == "BtnEsp")
		{
			OS_ClientAdmin.Esp = !OS_ClientAdmin.Esp;
			RefreshButtons();
		}
		else if (name == "BtnFreecam")
		{
			Close();
			if (OS_Client.Get())
				OS_Client.Get().GetFreecam().Toggle();
		}
		else if (name == "BtnKick")
		{
			if (NeedTarget())
				Send(OS_AdminAction.KICK, EditText(m_EditReason));
		}
		else if (name == "BtnBan")
		{
			if (NeedTarget())
				Send(OS_AdminAction.BAN, EditText(m_EditReason));
		}
		else if (name == "BtnAirdropHere")
		{
			Send(OS_AdminAction.AIRDROP, "here");
		}
		else if (name == "BtnAirdropRandom")
		{
			Send(OS_AdminAction.AIRDROP, "random");
		}
		else if (name == "BtnGiveXp")
		{
			Send(OS_AdminAction.GIVE_XP, EditText(m_EditReason));
		}
		else if (name == "BtnLockUnlock")
		{
			Send(OS_AdminAction.CODELOCK_UNLOCK);
		}
		else if (name == "BtnLockReset")
		{
			Send(OS_AdminAction.CODELOCK_RESET);
		}
		else if (name == "BtnBaseRemove")
		{
			Send(OS_AdminAction.BASE_REMOVE);
		}
		else if (name == "BtnAddAdmin")
		{
			Send(OS_AdminAction.ADD_ADMIN, EditText(m_EditReason));
		}
		else if (name == "BtnRemoveAdmin")
		{
			Send(OS_AdminAction.REMOVE_ADMIN, EditText(m_EditReason));
		}
		else if (name == "BtnUnban")
		{
			OS_ClientAdmin.SendAction(OS_AdminAction.UNBAN, "", EditText(m_EditReason));
		}
		else if (name == "BtnCarRepair")
		{
			Send(OS_AdminAction.VEHICLE_REPAIR);
		}
		else if (name == "BtnCarDestroy")
		{
			Send(OS_AdminAction.VEHICLE_DESTROY);
		}
		else if (name == "BtnCarDelete")
		{
			Send(OS_AdminAction.VEHICLE_DELETE);
		}
		else if (name == "BtnSpawner")
		{
			if (OS_Client.Get())
				OS_Client.Get().OpenAdminSpawner();
		}
		else if (name == "BtnMorning")
		{
			Send(OS_AdminAction.SET_TIME, "6:00");
		}
		else if (name == "BtnNoon")
		{
			Send(OS_AdminAction.SET_TIME, "12:00");
		}
		else if (name == "BtnEvening")
		{
			Send(OS_AdminAction.SET_TIME, "18:00");
		}
		else if (name == "BtnNight")
		{
			Send(OS_AdminAction.SET_TIME, "0:00");
		}
		else if (name == "BtnSetTime")
		{
			Send(OS_AdminAction.SET_TIME, EditText(m_EditTime));
		}
		else if (name == "BtnClear")
		{
			Send(OS_AdminAction.SET_WEATHER, "clear");
		}
		else if (name == "BtnCloudy")
		{
			Send(OS_AdminAction.SET_WEATHER, "cloudy");
		}
		else if (name == "BtnRain")
		{
			Send(OS_AdminAction.SET_WEATHER, "rain");
		}
		else if (name == "BtnStorm")
		{
			Send(OS_AdminAction.SET_WEATHER, "storm");
		}
		else if (name == "BtnFog")
		{
			Send(OS_AdminAction.SET_WEATHER, "fog");
		}
		else if (name == "BtnClose")
		{
			Close();
		}
		else
		{
			return super.OnClick(w, x, y, button);
		}
		return true;
	}
}

// ---------------------------------------------------------------- spawner

class OS_AdminSpawnMenu : OS_MenuBase
{
	protected static const int MAX_RESULTS = 400;

	protected EditBoxWidget m_EditSearch;
	protected EditBoxWidget m_EditCount;
	protected TextListboxWidget m_List;
	protected TextWidget m_Target;
	protected ref array<string> m_Classes = new array<string>;

	override string LayoutName()
	{
		return "admin_spawn.layout";
	}

	override void OnCreated()
	{
		m_EditSearch = FindEdit("EditSearch");
		m_EditCount = FindEdit("EditCount");
		m_List = FindList("ResultList");
		m_Target = FindText("Target");
		if (m_EditCount)
			m_EditCount.SetText("1");
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (m_Target)
			m_Target.SetText(OS_F("Cil: %1", OS_ClientAdmin.TargetLabel()));
	}

	//! hledá v CfgVehicles, CfgWeapons a CfgMagazines (vanilla i módy) podle názvu nebo classname
	protected void Search()
	{
		if (!m_List || !m_EditSearch)
			return;

		string query = m_EditSearch.GetText();
		query.TrimInPlace();
		query.ToLower();
		if (query.Length() < 2)
		{
			SetStatus(OS_T("Zadej aspon 2 znaky."));
			return;
		}

		m_List.ClearItems();
		m_Classes.Clear();

		array<string> roots = {"CfgVehicles", "CfgWeapons", "CfgMagazines"};
		bool full = false;
		foreach (string root : roots)
		{
			int count = g_Game.ConfigGetChildrenCount(root);
			for (int i = 0; i < count; i++)
			{
				string cls;
				g_Game.ConfigGetChildName(root, i, cls);
				string path = root + " " + cls;
				if (g_Game.ConfigGetInt(path + " scope") != 2)
					continue;

				string display;
				g_Game.ConfigGetText(path + " displayName", display);
				string clsLower = cls;
				clsLower.ToLower();
				string displayLower = display;
				displayLower.ToLower();
				if (!clsLower.Contains(query) && !displayLower.Contains(query))
					continue;

				string label = cls;
				if (display != "" && display.IndexOf("$") != 0)
					label = display + "   (" + cls + ")";
				if (root == "CfgVehicles" && g_Game.IsKindOf(cls, "Transport"))
					label = label + "   [" + OS_T("vozidlo") + "]";

				m_List.AddItem(label, null, 0);
				m_Classes.Insert(cls);
				if (m_Classes.Count() >= MAX_RESULTS)
				{
					full = true;
					break;
				}
			}
			if (full)
				break;
		}

		int found = m_Classes.Count();
		int maxShown = MAX_RESULTS;
		if (full)
			SetStatus(OS_F("Nalezeno: %1 (zobrazeno prvnich %2, upresni hledani)", found.ToString(), maxShown.ToString()));
		else
			SetStatus(OS_F("Nalezeno: %1", found.ToString()));
	}

	protected string SelectedClass()
	{
		if (!m_List)
			return "";
		int row = m_List.GetSelectedRow();
		if (row >= 0 && row < m_Classes.Count())
			return m_Classes[row];
		return "";
	}

	protected string CountText()
	{
		if (!m_EditCount)
			return "1";
		string text = m_EditCount.GetText();
		text.TrimInPlace();
		if (text == "")
			return "1";
		return text;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnSearch")
		{
			Search();
			return true;
		}
		if (name == "BtnClose")
		{
			Close();
			return true;
		}

		string cls = SelectedClass();
		if (name == "BtnToInv" || name == "BtnToGround" || name == "BtnVehicle")
		{
			if (cls == "")
			{
				SetStatus(OS_T("Vyber predmet v seznamu."));
				return true;
			}
			if (name == "BtnToInv")
				OS_ClientAdmin.SendAction(OS_AdminAction.SPAWN_ITEM, OS_ClientAdmin.SelectedId, cls + " " + CountText());
			else if (name == "BtnToGround")
				OS_ClientAdmin.SendAction(OS_AdminAction.SPAWN_GROUND, OS_ClientAdmin.SelectedId, cls + " " + CountText());
			else
				OS_ClientAdmin.SendAction(OS_AdminAction.SPAWN_VEHICLE, OS_ClientAdmin.SelectedId, cls);
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	override bool OnKeyPress(Widget w, int x, int y, int key)
	{
		if (key == KeyCode.KC_RETURN && w == m_EditSearch)
		{
			Search();
			return true;
		}
		return super.OnKeyPress(w, x, y, key);
	}
}

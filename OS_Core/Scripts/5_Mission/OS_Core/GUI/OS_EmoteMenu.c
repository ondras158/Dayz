// Emoce a spánek (klávesa L) + animace u akcí módu.

//! přehrání vanilla emoce (stejně jako menu gest); počká, až se zavře menu
class OS_ClientAnim
{
	protected static int s_Pending;
	protected static float s_PendingUntil;

	static void Play(int emoteId)
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsAlive() || !player.GetEmoteManager())
			return;
		if (player.GetCommand_Vehicle())
			return;
		if (g_Game.GetUIManager().GetMenu())
		{
			Queue(emoteId);
			return;
		}
		player.GetEmoteManager().CreateEmoteCBFromMenu(emoteId);
	}

	//! přehraje se po zavření menu (do 10 s)
	static void Queue(int emoteId)
	{
		s_Pending = emoteId;
		s_PendingUntil = g_Game.GetTickTime() + 10;
	}

	static void Update()
	{
		if (s_Pending == 0)
			return;
		if (g_Game.GetTickTime() > s_PendingUntil)
		{
			s_Pending = 0;
			return;
		}
		if (g_Game.GetUIManager().GetMenu())
			return;
		int id = s_Pending;
		s_Pending = 0;
		Play(id);
	}

	static bool IsPlaying()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		return player && player.GetEmoteManager() && player.GetEmoteManager().IsEmotePlaying();
	}
}

//! spánek na klientovi: ztmavená obrazovka, probuzení pohybem
class OS_ClientSleepView
{
	protected Widget m_Root;

	void OnServerState(ParamsReadContext ctx)
	{
		bool sleeping;
		if (!ctx.Read(sleeping))
			return;
		if (sleeping && !OS_ClientSleep.Sleeping)
		{
			OS_ClientSleep.Sleeping = true;
			OS_ClientSleep.SleepStarted = g_Game.GetTickTime();
			OS_ClientAnim.Play(EmoteConstants.ID_EMOTE_LYINGDOWN);
		}
		else if (!sleeping)
		{
			OS_ClientSleep.Sleeping = false;
		}
	}

	void Update()
	{
		bool show = OS_ClientSleep.Sleeping;
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (show && (!player || !player.IsAlive()))
		{
			OS_ClientSleep.Sleeping = false;
			show = false;
		}

		// postava vstala (pohyb) -> probudit
		if (show && g_Game.GetTickTime() - OS_ClientSleep.SleepStarted > 3 && !OS_ClientAnim.IsPlaying())
		{
			OS_ClientSleep.Sleeping = false;
			show = false;
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(false);
			rpc.Send(null, OS_RPC_SLEEP, true, null);
		}

		if (!show)
		{
			if (m_Root)
				m_Root.Show(false);
			return;
		}
		if (!m_Root)
		{
			m_Root = g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "sleep.layout");
			if (!m_Root)
				return;
		}
		TextWidget text = TextWidget.Cast(m_Root.FindAnyWidget("SleepText"));
		if (text)
		{
			string line = OS_T("Spis... (pohni se pro probuzeni)");
			if (OS_ClientHud.Fatigue >= 0)
			{
				int pct = Math.Round(OS_ClientHud.Fatigue * 100);
				line = OS_F("Spis... unava %1 %   (pohni se pro probuzeni)", pct.ToString());
			}
			text.SetText(line);
		}
		m_Root.Show(true);
	}
}

class OS_EmoteEntry
{
	int Id;            // 0 = spánek
	string Name;
	string Announce;   // text do chatu okolí ("" = nic)

	void OS_EmoteEntry(int id, string name, string announce)
	{
		Id = id;
		Name = name;
		Announce = announce;
	}
}

class OS_EmoteMenu : OS_MenuBase
{
	protected ref array<ref OS_EmoteEntry> m_Entries = new array<ref OS_EmoteEntry>;
	protected TextListboxWidget m_List;

	override string LayoutName()
	{
		return "emote_menu.layout";
	}

	protected void Add(int id, string name, string announce)
	{
		m_Entries.Insert(new OS_EmoteEntry(id, name, announce));
	}

	override void OnCreated()
	{
		Add(0, OS_T("Spat"), OS_T("jde spat"));
		Add(EmoteConstants.ID_EMOTE_SITA, OS_T("Sednout"), "");
		Add(EmoteConstants.ID_EMOTE_SITB, OS_T("Sednout (do turecka)"), "");
		Add(EmoteConstants.ID_EMOTE_LYINGDOWN, OS_T("Lehnout"), "");
		Add(EmoteConstants.ID_EMOTE_CAMPFIRE, OS_T("Sednout k ohni"), "");
		Add(EmoteConstants.ID_EMOTE_GREETING, OS_T("Mavat"), OS_T("mava"));
		Add(EmoteConstants.ID_EMOTE_SALUTE, OS_T("Salutovat"), OS_T("salutuje"));
		Add(EmoteConstants.ID_EMOTE_CLAP, OS_T("Tleskat"), OS_T("tleska"));
		Add(EmoteConstants.ID_EMOTE_THUMB, OS_T("Palec nahoru"), OS_T("ukazuje palec nahoru"));
		Add(EmoteConstants.ID_EMOTE_THUMBDOWN, OS_T("Palec dolu"), OS_T("ukazuje palec dolu"));
		Add(EmoteConstants.ID_EMOTE_POINT, OS_T("Ukazat"), "");
		Add(EmoteConstants.ID_EMOTE_COME, OS_T("Pojd sem"), OS_T("vola: pojd sem"));
		Add(EmoteConstants.ID_EMOTE_MOVE, OS_T("Jdeme"), OS_T("vola: jdeme"));
		Add(EmoteConstants.ID_EMOTE_NOD, OS_T("Kyvnout (ano)"), OS_T("prikyvuje"));
		Add(EmoteConstants.ID_EMOTE_SHAKE, OS_T("Zavrtet hlavou (ne)"), OS_T("vrti hlavou"));
		Add(EmoteConstants.ID_EMOTE_SHRUG, OS_T("Pokrcit rameny"), OS_T("krci rameny"));
		Add(EmoteConstants.ID_EMOTE_SURRENDER, OS_T("Vzdat se"), OS_T("se vzdava"));
		Add(EmoteConstants.ID_EMOTE_SILENT, OS_T("Ticho"), "");
		Add(EmoteConstants.ID_EMOTE_HEART, OS_T("Srdce"), "");
		Add(EmoteConstants.ID_EMOTE_FACEPALM, OS_T("Facepalm"), "");
		Add(EmoteConstants.ID_EMOTE_TAUNT, OS_T("Posmesek"), "");
		Add(EmoteConstants.ID_EMOTE_DANCE, OS_T("Tancit"), OS_T("tanci"));

		m_List = FindList("EmoteList");
		if (m_List)
		{
			foreach (OS_EmoteEntry entry : m_Entries)
			{
				m_List.AddItem(entry.Name, null, 0);
			}
			m_List.SelectRow(0);
		}
		if (OS_ClientHud.Fatigue >= 0)
		{
			int pct = Math.Round(OS_ClientHud.Fatigue * 100);
			SetStatus(OS_F("Unava: %1 %", pct.ToString()));
		}
	}

	protected void PlaySelected()
	{
		if (!m_List)
			return;
		int row = m_List.GetSelectedRow();
		if (row < 0 || row >= m_Entries.Count())
			return;
		OS_EmoteEntry entry = m_Entries[row];
		Close();

		if (entry.Id == 0)
		{
			ScriptRPC sleepRpc = new ScriptRPC();
			sleepRpc.Write(true);
			sleepRpc.Send(null, OS_RPC_SLEEP, true, null);
			return;
		}
		OS_ClientAnim.Queue(entry.Id);
		if (entry.Announce != "")
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(entry.Announce);
			rpc.Send(null, OS_RPC_EMOTE_ANNOUNCE, true, null);
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnPlay")
		{
			PlaySelected();
			return true;
		}
		if (name == "BtnClose")
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_List)
		{
			PlaySelected();
			return true;
		}
		return super.OnDoubleClick(w, x, y, button);
	}
}

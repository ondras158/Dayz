// Nastavení místa spawnu vozidla ve hře: obdélník ve velikosti vozidla před hráčem.
// Hráč se postaví a otočí ve směru jízdy, Y = uložit, N = zrušit (klávesy jdou změnit v nastavení).

class OS_SpawnPlacer
{
	protected bool m_Active;
	protected int m_Target;          // OS_SpawnTarget
	protected float m_Width;
	protected float m_Length;
	protected bool m_OnWater;
	protected bool m_HasCurrent;
	protected vector m_CurrentPos;   // dosavadní místo (žlutě)
	protected float m_CurrentYaw;

	protected Widget m_Root;
	protected CanvasWidget m_Canvas;
	protected TextWidget m_Hint;

	bool IsActive()
	{
		return m_Active;
	}

	//! velikost obdélníku podle druhu vozidla
	static void SizeFor(string category, out float width, out float length)
	{
		width = 2.6;
		length = 5.5;
		if (category == "bike")
		{
			width = 1.2;
			length = 2.4;
		}
		else if (category == "boat")
		{
			width = 3.5;
			length = 8.0;
		}
		else if (category == "heli")
		{
			width = 12.0;
			length = 14.0;
		}
	}

	void Start(int target, string category, bool onWater, bool hasCurrent, vector currentPos, float currentYaw)
	{
		m_Target = target;
		SizeFor(category, m_Width, m_Length);
		m_OnWater = onWater;
		m_HasCurrent = hasCurrent;
		m_CurrentPos = currentPos;
		m_CurrentYaw = currentYaw;

		if (!m_Root)
		{
			m_Root = g_Game.GetWorkspace().CreateWidgets(OS_LAYOUT_DIR + "placer.layout");
			if (m_Root)
			{
				m_Canvas = CanvasWidget.Cast(m_Root.FindAnyWidget("Canvas"));
				m_Hint = TextWidget.Cast(m_Root.FindAnyWidget("Hint"));
			}
		}
		if (m_Root)
			m_Root.Show(true);
		m_Active = true;
	}

	void Stop()
	{
		m_Active = false;
		if (m_Canvas)
			m_Canvas.Clear();
		if (m_Root)
			m_Root.Show(false);
	}

	protected float GroundY(float x, float z)
	{
		float y = g_Game.SurfaceY(x, z);
		if (m_OnWater)
			y = Math.Max(y, g_Game.SurfaceGetSeaLevel());
		return y + 0.15;
	}

	//! rohy obdélníku (střed, natočení ve stupních)
	protected void Corners(vector center, float yaw, out vector c0, out vector c1, out vector c2, out vector c3, out vector nose)
	{
		float rad = yaw * Math.DEG2RAD;
		vector fwd = Vector(Math.Sin(rad), 0, Math.Cos(rad));
		vector right = Vector(fwd[2], 0, -fwd[0]);
		float hw = m_Width / 2;
		float hl = m_Length / 2;

		c0 = center + fwd * hl - right * hw;
		c1 = center + fwd * hl + right * hw;
		c2 = center - fwd * hl + right * hw;
		c3 = center - fwd * hl - right * hw;
		nose = center + fwd * (hl + 1.5);

		c0[1] = GroundY(c0[0], c0[2]);
		c1[1] = GroundY(c1[0], c1[2]);
		c2[1] = GroundY(c2[0], c2[2]);
		c3[1] = GroundY(c3[0], c3[2]);
		nose[1] = GroundY(nose[0], nose[2]);
	}

	protected void Line(vector a, vector b, int color, float width)
	{
		vector sa = g_Game.GetScreenPosRelative(a);
		vector sb = g_Game.GetScreenPosRelative(b);
		if (sa[2] <= 0 || sb[2] <= 0)
			return;

		int sw;
		int sh;
		GetScreenSize(sw, sh);
		m_Canvas.DrawLine(sa[0] * sw, sa[1] * sh, sb[0] * sw, sb[1] * sh, width, color);
	}

	protected void DrawBox(vector center, float yaw, int color, float width)
	{
		vector c0;
		vector c1;
		vector c2;
		vector c3;
		vector nose;
		Corners(center, yaw, c0, c1, c2, c3, nose);

		Line(c0, c1, color, width);
		Line(c1, c2, color, width);
		Line(c2, c3, color, width);
		Line(c3, c0, color, width);
		// šipka = směr jízdy
		Line(c0, nose, color, width);
		Line(c1, nose, color, width);
		// úhlopříčky, aby byl obdélník vidět i zblízka
		Line(c0, c2, color, 1);
		Line(c1, c3, color, 1);
	}

	void Update()
	{
		if (!m_Active)
			return;

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsAlive())
		{
			Stop();
			return;
		}

		// při otevřeném menu nekreslit a nereagovat na klávesy
		bool menuOpen = g_Game.GetUIManager().GetMenu() != null;
		if (m_Root)
			m_Root.Show(!menuOpen);
		if (menuOpen || !m_Canvas)
			return;

		// nové místo: před hráčem, ve směru, kam se dívá
		vector dir = player.GetDirection();
		dir[1] = 0;
		dir.Normalize();
		vector angles = dir.VectorToAngles();
		float yaw = angles[0];
		vector center = player.GetPosition() + dir * (m_Length / 2 + 1.5);

		m_Canvas.Clear();
		if (m_HasCurrent)
			DrawBox(m_CurrentPos, m_CurrentYaw, ARGB(200, 255, 200, 40), 2);   // dosavadní místo
		DrawBox(center, yaw, ARGB(230, 60, 255, 90), 3);                      // nové místo

		if (m_Hint)
			m_Hint.SetText("MISTO PRO VOZIDLO (zelene) - postav se a otoc ve smeru jizdy.   Y = ulozit   N = zrusit   (zlute = dosavadni misto)");

		UAInput confirm = GetUApi().GetInputByName("UAOSPlaceConfirm");
		UAInput cancel = GetUApi().GetInputByName("UAOSPlaceCancel");

		if (confirm && confirm.LocalPress())
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(m_Target);
			rpc.Write(center[0]);
			rpc.Write(center[2]);
			rpc.Write(yaw);
			rpc.Send(null, OS_RPC_VEHICLE_SET_SPAWN, true, null);
			Stop();
		}
		else if (cancel && cancel.LocalPress())
		{
			Stop();
		}
	}

	void Destroy()
	{
		if (m_Root)
			m_Root.Unlink();
		m_Root = null;
	}
}

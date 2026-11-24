// Uvítací zpráva, pravidelná oznámení a kill feed.

class BC_ServerMessages
{
	ref BC_MessagesConfig m_Cfg;
	protected float m_BroadcastTimer;
	protected int m_BroadcastIndex;

	void BC_ServerMessages()
	{
		BC_MessagesConfig defaults = new BC_MessagesConfig();
		defaults.FillDefaults();
		m_Cfg = BC_ConfigFile<BC_MessagesConfig>.Load("Messages.json", defaults, new BC_MessagesConfig());
	}

	void OnConnect(PlayerBase player)
	{
		if (m_Cfg.WelcomeTitle != "" || m_Cfg.WelcomeText != "")
			BC_Util.Notify(player, m_Cfg.WelcomeTitle, m_Cfg.WelcomeText, m_Cfg.WelcomeSeconds);
	}

	void OnTick(float deltaTime)
	{
		if (m_Cfg.BroadcastIntervalMinutes <= 0 || !m_Cfg.Broadcasts || m_Cfg.Broadcasts.Count() == 0)
			return;

		m_BroadcastTimer += deltaTime;
		if (m_BroadcastTimer < m_Cfg.BroadcastIntervalMinutes * 60)
			return;

		m_BroadcastTimer = 0;
		if (m_BroadcastIndex >= m_Cfg.Broadcasts.Count())
			m_BroadcastIndex = 0;

		BC_Util.ChatAll(m_Cfg.Broadcasts[m_BroadcastIndex]);
		m_BroadcastIndex++;
	}

	void OnPlayerKilled(PlayerBase victim, PlayerBase killerPlayer, Object killer)
	{
		if (!m_Cfg.KillFeed)
			return;

		string victimName = BC_Util.PlayerName(victim);
		string msg;

		if (killerPlayer && killerPlayer != victim)
			msg = victimName + " byl zabit hracem " + BC_Util.PlayerName(killerPlayer);
		else if (ZombieBase.Cast(killer))
			msg = victimName + " byl zabit zombiky";
		else
			msg = victimName + " zemrel";

		BC_Util.ChatAll(msg);
	}
}

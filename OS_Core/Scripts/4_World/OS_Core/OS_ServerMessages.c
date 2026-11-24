// Uvítací zpráva, pravidelná oznámení a kill feed.

class OS_ServerMessages
{
	ref OS_MessagesConfig m_Cfg;
	protected float m_BroadcastTimer;
	protected int m_BroadcastIndex;

	void OS_ServerMessages()
	{
		OS_MessagesConfig defaults = new OS_MessagesConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_MessagesConfig>.Load("Messages.json", defaults, new OS_MessagesConfig());
	}

	void OnConnect(PlayerBase player)
	{
		if (m_Cfg.WelcomeTitle != "" || m_Cfg.WelcomeText != "")
			OS_Util.Notify(player, m_Cfg.WelcomeTitle, m_Cfg.WelcomeText, m_Cfg.WelcomeSeconds);
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

		OS_Util.ChatAll(m_Cfg.Broadcasts[m_BroadcastIndex]);
		m_BroadcastIndex++;
	}

	void OnPlayerKilled(PlayerBase victim, PlayerBase killerPlayer, Object killer)
	{
		if (!m_Cfg.KillFeed)
			return;

		string victimName = OS_Util.PlayerName(victim);
		string msg;

		if (killerPlayer && killerPlayer != victim)
			msg = victimName + " byl zabit hracem " + OS_Util.PlayerName(killerPlayer);
		else if (ZombieBase.Cast(killer))
			msg = victimName + " byl zabit zombiky";
		else
			msg = victimName + " zemrel";

		OS_Util.ChatAll(msg);
	}
}

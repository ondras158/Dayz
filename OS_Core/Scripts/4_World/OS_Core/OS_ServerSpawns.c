// Výběr místa spawnu po vytvoření nové postavy.

class OS_PendingSpawn
{
	PlayerBase Player;
	float Created;
	float LastSent = -100;
	bool Acked;
	float AckTime;
	bool HasBed;      // první položka menu = spací místo
}

class OS_ServerSpawns
{
	protected static const float RESEND_SECONDS = 3;
	protected static const float MAX_WAIT_FOR_CLIENT = 180; // jak dlouho čekat, než se klient načte

	ref OS_SpawnConfig m_Cfg;
	protected ref array<ref OS_PendingSpawn> m_Pending = new array<ref OS_PendingSpawn>;

	void OS_ServerSpawns()
	{
		OS_SpawnConfig defaults = new OS_SpawnConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_SpawnConfig>.Load("SpawnZones.json", defaults, new OS_SpawnConfig());
	}

	protected float Now()
	{
		return g_Game.GetTickTime();
	}

	void OnNewCharacter(PlayerBase player)
	{
		if (!player || !m_Cfg.Enabled)
			return;
		bool bed = BedAvailable(player, true);
		if (!bed && (!m_Cfg.Zones || m_Cfg.Zones.Count() == 0))
			return;

		OS_PendingSpawn pending = new OS_PendingSpawn();
		pending.Player = player;
		pending.Created = Now();
		m_Pending.Insert(pending);

		if (m_Cfg.ProtectWhileChoosing)
		{
			player.m_OS_SpawnProtect = true;
			player.OS_RefreshDamage();
		}
	}

	void OnTick()
	{
		float now = Now();
		for (int i = m_Pending.Count() - 1; i >= 0; i--)
		{
			OS_PendingSpawn pending = m_Pending[i];

			bool expired = false;
			if (!pending.Player || !pending.Player.IsAlive())
				expired = true;
			else if (!pending.Acked && now - pending.Created > MAX_WAIT_FOR_CLIENT)
				expired = true;
			else if (pending.Acked && now - pending.AckTime > m_Cfg.MenuTimeoutSeconds + 5)
				expired = true;

			if (expired)
			{
				Finish(i);
				continue;
			}

			if (!pending.Acked && now - pending.LastSent >= RESEND_SECONDS)
			{
				pending.LastSent = now;
				SendMenu(pending.Player);
			}
		}
	}

	protected void SendMenu(PlayerBase player)
	{
		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		array<string> names = new array<string>;
		OS_PendingSpawn pending;
		int pendingIndex = FindIndex(identity);
		if (pendingIndex >= 0)
			pending = m_Pending[pendingIndex];
		if (pending)
		{
			pending.HasBed = BedAvailable(player);
			if (pending.HasBed)
				names.Insert(OS_T("> Moje spaci misto (stan / pristresek)"));
		}
		foreach (OS_SpawnZone zone : m_Cfg.Zones)
		{
			names.Insert(OS_T(zone.Name));
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(names);
		rpc.Write(m_Cfg.MenuTimeoutSeconds);
		rpc.Send(null, OS_RPC_SPAWN_MENU, true, identity);
	}

	protected int FindIndex(PlayerIdentity sender)
	{
		if (!sender)
			return -1;

		for (int i = 0; i < m_Pending.Count(); i++)
		{
			PlayerBase player = m_Pending[i].Player;
			if (player && player.GetIdentity() && player.GetIdentity().GetId() == sender.GetId())
				return i;
		}
		return -1;
	}

	protected void Finish(int index)
	{
		OS_PendingSpawn pending = m_Pending[index];
		if (pending.Player)
		{
			pending.Player.m_OS_SpawnProtect = false;
			pending.Player.OS_RefreshDamage();
		}
		m_Pending.Remove(index);
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType == OS_RPC_SPAWN_ACK)
		{
			int ackIndex = FindIndex(sender);
			if (ackIndex >= 0 && !m_Pending[ackIndex].Acked)
			{
				m_Pending[ackIndex].Acked = true;
				m_Pending[ackIndex].AckTime = Now();
			}
			return true;
		}

		if (rpcType == OS_RPC_SPAWN_CHOICE)
		{
			int zoneIndex;
			if (!ctx.Read(zoneIndex))
				return true;

			int index = FindIndex(sender);
			if (index < 0)
				return true;

			PlayerBase player = m_Pending[index].Player;
			if (m_Pending[index].HasBed)
			{
				if (zoneIndex == 0)
				{
					if (!OS_Server.Get() || !OS_Server.Get().m_Death.SpawnAtBed(player))
						OS_Util.Notify(player, OS_T("Spawn"), OS_T("Spaci misto uz neexistuje - zustavas na miste."));
					Finish(index);
					return true;
				}
				zoneIndex--;
			}
			if (zoneIndex >= 0 && zoneIndex < m_Cfg.Zones.Count())
				SpawnInZone(player, m_Cfg.Zones[zoneIndex]);

			Finish(index);
			return true;
		}

		return false;
	}

	protected bool BedAvailable(PlayerBase player, bool notify = false)
	{
		if (!player || !OS_Server.Get() || !OS_Server.Get().m_Death)
			return false;
		string id = OS_Util.SteamId(player);
		vector pos;
		if (!OS_Server.Get().m_Death.GetBed(id, pos))
			return false;
		int cooldown = OS_Server.Get().m_Death.BedCooldown(id);
		if (cooldown > 0)
		{
			if (!notify)
				return false;
			int minutes = Math.Ceil(cooldown / 60.0);
			OS_Util.Notify(player, OS_T("Spawn"), OS_F("Spaci misto bude zase k dispozici za %1 min.", minutes.ToString()));
			return false;
		}
		return true;
	}

	protected void SpawnInZone(PlayerBase player, OS_SpawnZone zone)
	{
		if (!player || !zone || !zone.Points || zone.Points.Count() == 0)
			return;

		vector point = zone.Points.GetRandomElement();
		float x = point[0] + Math.RandomFloatInclusive(-3, 3);
		float z = point[2] + Math.RandomFloatInclusive(-3, 3);

		OS_Util.Teleport(player, OS_Util.GroundPos(x, z));
		OS_Util.Notify(player, OS_T("Spawn"), OS_F("Objevil ses v oblasti: %1", OS_T(zone.Name)));
	}
}

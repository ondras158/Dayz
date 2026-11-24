// Startovní výbava nových postav.

class OS_ServerLoadouts
{
	ref OS_LoadoutConfig m_Cfg;

	void OS_ServerLoadouts()
	{
		OS_LoadoutConfig defaults = new OS_LoadoutConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_LoadoutConfig>.Load("Loadouts.json", defaults, new OS_LoadoutConfig());
	}

	void Equip(PlayerBase player)
	{
		if (!player || !m_Cfg.Enabled || !m_Cfg.Loadouts || m_Cfg.Loadouts.Count() == 0)
			return;

		OS_Loadout loadout = PickLoadout();
		if (!loadout)
			return;

		if (m_Cfg.ReplaceVanillaGear)
			player.RemoveAllItems();

		// oblečení nejdřív, aby se do něj vešly věci
		if (loadout.Clothes)
		{
			foreach (string clothEntry : loadout.Clothes)
			{
				OS_Util.GiveItem(player, OS_Util.PickVariant(clothEntry));
			}
		}

		if (loadout.Items)
		{
			foreach (string itemEntry : loadout.Items)
			{
				OS_Util.GiveItem(player, OS_Util.PickVariant(itemEntry));
			}
		}
	}

	protected OS_Loadout PickLoadout()
	{
		int total = 0;
		foreach (OS_Loadout candidate : m_Cfg.Loadouts)
		{
			if (candidate && candidate.Weight > 0)
				total += candidate.Weight;
		}

		if (total <= 0)
			return m_Cfg.Loadouts.GetRandomElement();

		int roll = Math.RandomInt(0, total);
		foreach (OS_Loadout loadout : m_Cfg.Loadouts)
		{
			if (!loadout || loadout.Weight <= 0)
				continue;

			roll -= loadout.Weight;
			if (roll < 0)
				return loadout;
		}

		return m_Cfg.Loadouts[0];
	}
}

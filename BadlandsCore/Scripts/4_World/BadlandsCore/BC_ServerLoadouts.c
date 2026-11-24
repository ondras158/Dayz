// Startovní výbava nových postav.

class BC_ServerLoadouts
{
	ref BC_LoadoutConfig m_Cfg;

	void BC_ServerLoadouts()
	{
		BC_LoadoutConfig defaults = new BC_LoadoutConfig();
		defaults.FillDefaults();
		m_Cfg = BC_ConfigFile<BC_LoadoutConfig>.Load("Loadouts.json", defaults, new BC_LoadoutConfig());
	}

	void Equip(PlayerBase player)
	{
		if (!player || !m_Cfg.Enabled || !m_Cfg.Loadouts || m_Cfg.Loadouts.Count() == 0)
			return;

		BC_Loadout loadout = PickLoadout();
		if (!loadout)
			return;

		if (m_Cfg.ReplaceVanillaGear)
			player.RemoveAllItems();

		// oblečení nejdřív, aby se do něj vešly věci
		if (loadout.Clothes)
		{
			foreach (string clothEntry : loadout.Clothes)
			{
				BC_Util.GiveItem(player, BC_Util.PickVariant(clothEntry));
			}
		}

		if (loadout.Items)
		{
			foreach (string itemEntry : loadout.Items)
			{
				BC_Util.GiveItem(player, BC_Util.PickVariant(itemEntry));
			}
		}
	}

	protected BC_Loadout PickLoadout()
	{
		int total = 0;
		foreach (BC_Loadout candidate : m_Cfg.Loadouts)
		{
			if (candidate && candidate.Weight > 0)
				total += candidate.Weight;
		}

		if (total <= 0)
			return m_Cfg.Loadouts.GetRandomElement();

		int roll = Math.RandomInt(0, total);
		foreach (BC_Loadout loadout : m_Cfg.Loadouts)
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

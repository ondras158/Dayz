// Projde všechny předměty v configu hry (vanilla i módy) a chybějící přidá do obchodu.

class OS_TraderScanner
{
	//! Vrací počet nově přidaných předmětů
	static int AddNewItems(OS_TraderConfig cfg)
	{
		float started = g_Game.GetTickTime();

		// co už v obchodě je
		map<string, bool> existing = new map<string, bool>;
		foreach (OS_TraderCategory category : cfg.Categories)
		{
			if (!category || !category.Items)
				continue;
			foreach (OS_TraderItem item : category.Items)
			{
				if (item)
					existing.Set(Lower(item.ClassName), true);
			}
		}

		array<string> excludes = LowerAll(cfg.ExcludeContains);

		// nové předměty roztříděné podle pravidla (index pravidla -> classnames)
		map<int, ref array<string>> found = new map<int, ref array<string>>;
		int scanned = 0;

		array<string> roots = {"CfgVehicles", "CfgWeapons", "CfgMagazines"};
		foreach (string root : roots)
		{
			int count = g_Game.ConfigGetChildrenCount(root);
			for (int i = 0; i < count; i++)
			{
				string name;
				if (!g_Game.ConfigGetChildName(root, i, name))
					continue;

				string cfgPath = root + " " + name;
				if (g_Game.ConfigGetInt(cfgPath + " scope") != 2)
					continue;

				string lowerName = Lower(name);
				if (existing.Contains(lowerName) || IsExcluded(lowerName, excludes))
					continue;

				TStringArray parents = new TStringArray;
				g_Game.ConfigGetFullPath(cfgPath, parents);
				array<string> lowerParents = LowerAll(parents);

				// jen předměty, které jde mít v inventáři
				if (lowerParents.Find("inventory_base") == -1 && lowerParents.Find("weapon_base") == -1 && lowerParents.Find("magazine_base") == -1)
					continue;

				scanned++;
				int ruleIndex = MatchRule(cfg, cfgPath, lowerName, lowerParents);
				if (ruleIndex < 0)
					continue;

				if (!found.Contains(ruleIndex))
					found.Set(ruleIndex, new array<string>);
				found.Get(ruleIndex).Insert(name);
				existing.Set(lowerName, true);
			}
		}

		// přidat v pořadí pravidel, v každé kategorii abecedně
		int added = 0;
		for (int r = 0; r < cfg.CategoryRules.Count(); r++)
		{
			if (!found.Contains(r))
				continue;

			OS_CategoryRule rule = cfg.CategoryRules[r];
			array<string> names = found.Get(r);
			names.Sort();

			OS_TraderCategory target = cfg.GetOrCreateCategory(rule.Category);
			foreach (string className : names)
			{
				target.Add(className, rule.DefaultBuy, rule.DefaultSell);
				added++;
			}
		}

		float took = g_Game.GetTickTime() - started;
		Print(OS_TAG + "Sken predmetu: " + scanned.ToString() + " novych kandidatu, pridano " + added.ToString() + " (" + took.ToString() + " s)");
		return added;
	}

	protected static int MatchRule(OS_TraderConfig cfg, string cfgPath, string lowerName, array<string> lowerParents)
	{
		array<string> slots;   // načte se jen když je potřeba

		for (int r = 0; r < cfg.CategoryRules.Count(); r++)
		{
			OS_CategoryRule rule = cfg.CategoryRules[r];
			if (!rule)
				continue;

			bool hasCriteria = false;

			// vyloučené názvy (např. konzervy u ryb)
			bool excluded = false;
			if (rule.NameExcludes)
			{
				foreach (string excludePart : rule.NameExcludes)
				{
					if (lowerName.Contains(Lower(excludePart)))
						excluded = true;
				}
			}
			if (excluded)
				continue;

			if (rule.BaseClasses && rule.BaseClasses.Count() > 0)
			{
				hasCriteria = true;
				foreach (string baseClass : rule.BaseClasses)
				{
					if (lowerParents.Find(Lower(baseClass)) > -1)
						return r;
				}
			}

			if (rule.NameContains && rule.NameContains.Count() > 0)
			{
				hasCriteria = true;
				foreach (string part : rule.NameContains)
				{
					if (lowerName.Contains(Lower(part)))
						return r;
				}
			}

			bool wantsSlots = (rule.Slots && rule.Slots.Count() > 0) || (rule.SlotContains && rule.SlotContains.Count() > 0);
			if (wantsSlots)
			{
				hasCriteria = true;
				if (!slots)
					slots = ReadSlots(cfgPath);

				foreach (string slot : slots)
				{
					if (rule.Slots)
					{
						foreach (string exact : rule.Slots)
						{
							if (slot == Lower(exact))
								return r;
						}
					}
					if (rule.SlotContains)
					{
						foreach (string slotPart : rule.SlotContains)
						{
							if (slot.Contains(Lower(slotPart)))
								return r;
						}
					}
				}
			}

			// pravidlo bez podmínek bere všechno
			if (!hasCriteria)
				return r;
		}
		return -1;
	}

	protected static array<string> ReadSlots(string cfgPath)
	{
		TStringArray raw = new TStringArray;
		g_Game.ConfigGetTextArray(cfgPath + " inventorySlot", raw);
		if (raw.Count() == 0)
		{
			string single;
			if (g_Game.ConfigGetText(cfgPath + " inventorySlot", single) && single != "")
				raw.Insert(single);
		}
		return LowerAll(raw);
	}

	protected static bool IsExcluded(string lowerName, array<string> lowerExcludes)
	{
		foreach (string ex : lowerExcludes)
		{
			if (ex != "" && lowerName.Contains(ex))
				return true;
		}
		return false;
	}

	static string Lower(string text)
	{
		string copy = text;
		copy.ToLower();
		return copy;
	}

	static array<string> LowerAll(array<string> values)
	{
		array<string> result = new array<string>;
		if (!values)
			return result;
		foreach (string v : values)
		{
			result.Insert(Lower(v));
		}
		return result;
	}
}

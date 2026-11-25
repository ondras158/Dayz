// Dovednosti: zkušenosti za hraní, úrovně a body do statů. Nastavení: $profile:OS_Core/Skills.json
// Data hráčů: $profile:OS_Core/Skills/<SteamID>.json (zůstávají i po smrti).
// Menu: klávesa O.

class OS_SkillDef
{
	string Id;            // toughness / stamina / regen / blood / metabolism / warmth
	string Name;
	int MaxRank = 10;
	float PerRank;        // účinek za jednu úroveň dovednosti (viz popis u Id)
	string Description;

	void OS_SkillDef(string id = "", string name = "", int maxRank = 10, float perRank = 0, string description = "")
	{
		Id = id;
		Name = name;
		MaxRank = maxRank;
		PerRank = perRank;
		Description = description;
	}
}

class OS_SkillsConfig
{
	bool Enabled = true;

	// zkušenosti
	int XpZombieKill = 10;
	int XpAnimalKill = 20;
	int XpPlayerKill = 40;
	int XpCookDish = 15;
	int XpFarmItem = 1;               // za každý vybraný kus z farmy
	int XpPerSurvivalMinute = 2;      // za každou minutu naživu na serveru

	// úrovně: na další úroveň je potřeba XpFirstLevel + úroveň * XpLevelIncrease
	int XpFirstLevel = 100;
	int XpLevelIncrease = 50;
	int MaxLevel = 60;
	int PointsPerLevel = 1;
	int ResetPrice = 5000;            // přerozdělení bodů za peníze (0 = zdarma, -1 = nejde)

	ref array<ref OS_SkillDef> Skills = new array<ref OS_SkillDef>;

	void FillDefaults()
	{
		// toughness: podíl zranění, který se vrátí (0.03 = 3 % za úroveň)
		Skills.Insert(new OS_SkillDef("toughness", "Odolnost", 10, 0.03, "Mene zraneni a ztraty krve"));
		// stamina: o kolik méně výdrže se spotřebuje (0.05 = 5 % za úroveň)
		Skills.Insert(new OS_SkillDef("stamina", "Vydrz", 10, 0.05, "Mene unavy pri behu a skakani"));
		// regen: zdraví za sekundu
		Skills.Insert(new OS_SkillDef("regen", "Regenerace", 10, 0.01, "Postupne doplnovani zdravi"));
		// blood: krev za sekundu
		Skills.Insert(new OS_SkillDef("blood", "Krev", 10, 0.5, "Rychlejsi doplnovani krve"));
		// metabolism: o kolik pomaleji ubývá jídlo a voda (0.05 = 5 % za úroveň)
		Skills.Insert(new OS_SkillDef("metabolism", "Metabolismus", 10, 0.05, "Pomalejsi hlad a zizen"));
		// warmth: podíl tepelné zásoby za sekundu
		Skills.Insert(new OS_SkillDef("warmth", "Otuzilost", 10, 0.002, "Mene zimy"));
	}

	OS_SkillDef Find(string id)
	{
		foreach (OS_SkillDef def : Skills)
		{
			if (def && def.Id == id)
				return def;
		}
		return null;
	}

	int XpForNextLevel(int level)
	{
		return XpFirstLevel + level * XpLevelIncrease;
	}
}

class OS_PlayerSkills
{
	int Level;
	int Xp;                 // zkušenosti v aktuální úrovni
	int TotalXp;
	int Points;             // nerozdělené body
	ref map<string, int> Ranks = new map<string, int>;

	int Rank(string id)
	{
		int rank;
		if (Ranks && Ranks.Find(id, rank))
			return rank;
		return 0;
	}
}

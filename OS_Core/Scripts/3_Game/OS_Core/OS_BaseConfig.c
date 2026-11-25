// Base u vanilla vlajky (TerritoryFlag): založení, vylepšování poloměru, rozestupy mezi basemi.
// Nastavení: $profile:OS_Core/Bases.json, data basí: $profile:OS_Core/BasesData.json
// Base spravuje majitel a jeho skupina (akce "Base" u postavené vlajky).

class OS_BaseLevel
{
	float Radius;
	int Price;

	void OS_BaseLevel(float radius = 30, int price = 0)
	{
		Radius = radius;
		Price = price;
	}
}

class OS_BaseConfig
{
	bool Enabled = true;
	int FoundPrice = 0;                 // cena založení base (úroveň 1)
	int MaxBasesPerPlayer = 1;          // kolik basí smí mít jeden hráč (0 = bez omezení)
	bool RequirePole = true;            // base jde založit až u vlajky s postaveným stožárem
	float MinDistanceBetweenBases = 150;// minimální vzdálenost vlajek dvou basí (m)
	float GapBetweenBases = 20;         // mezera mezi okraji basí - poloměry se nesmí překrývat (m)
	bool FarmRequiresBase = true;       // farma jde postavit jen ve vlastní base (nebo base skupiny)
	bool ShowOnMap = true;              // vlastní base a base skupiny jako kruh na mapě
	string MapColor = "70 140 255";
	ref array<ref OS_BaseLevel> Levels = new array<ref OS_BaseLevel>;

	void FillDefaults()
	{
		Levels.Insert(new OS_BaseLevel(30, 0));
		Levels.Insert(new OS_BaseLevel(50, 5000));
		Levels.Insert(new OS_BaseLevel(75, 15000));
		Levels.Insert(new OS_BaseLevel(100, 35000));
		Levels.Insert(new OS_BaseLevel(150, 75000));
	}

	float RadiusOf(int level)
	{
		if (!Levels || Levels.Count() == 0)
			return 30;
		int index = level - 1;
		if (index < 0)
			index = 0;
		if (index >= Levels.Count())
			index = Levels.Count() - 1;
		return Levels[index].Radius;
	}
}

class OS_Base
{
	int Id;
	string OwnerId;
	string OwnerName;
	float X;
	float Z;
	int Level = 1;
}

class OS_BasesData
{
	int NextId = 1;
	ref array<ref OS_Base> Bases = new array<ref OS_Base>;
}

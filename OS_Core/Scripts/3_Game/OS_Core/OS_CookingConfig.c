// Vaření podle receptů u ohně. Nastavení: $profile:OS_Core/Cooking.json
// U hořícího ohniště (nebo zapnutého plynového vařiče) akce "Vařit podle receptu".
//
// Suroviny: "Items" = možnosti oddělené |, např. "Potato|Tomato".
//   přesný classname nebo předek v configu (Edible_Base...),
//   *text  = classname obsahuje text (např. *SteakMeat = jakékoli maso ze zvěře),
//   -text  = vyloučit (např. -Psilocybe, aby lysohlávka nešla do polévky).
// Shnilé a spálené suroviny se nepočítají.
// Tool: "pot" (hrnec, kotlík), "pan" (pánev), "" = stačí oheň.
// Buff: "regen" (zdraví), "blood" (krev), "warmth" (teplo), "stamina" (výdrž), "" = nic.

class OS_CookIngredient
{
	string Items;
	int Count = 1;

	void OS_CookIngredient(string items = "", int count = 1)
	{
		Items = items;
		Count = count;
	}
}

class OS_CookRecipe
{
	string Name;
	string Result;            // classname hotového jídla (OS_Dish_... nebo cokoli jiného)
	int ResultCount = 1;
	string Tool = "pot";
	float CookSeconds = 30;
	string Buff = "";
	float BuffMinutes = 15;
	ref array<ref OS_CookIngredient> Ingredients = new array<ref OS_CookIngredient>;

	void OS_CookRecipe(string name = "", string result = "", string tool = "pot", float seconds = 30, string buff = "", float buffMinutes = 15)
	{
		Name = name;
		Result = result;
		Tool = tool;
		CookSeconds = seconds;
		Buff = buff;
		BuffMinutes = buffMinutes;
	}

	void Add(string items, int count = 1)
	{
		Ingredients.Insert(new OS_CookIngredient(items, count));
	}
}

class OS_CookingConfig
{
	bool Enabled = true;
	float FireRadius = 3;              // jak blízko ohně musí hráč stát (m)
	string PotItems = "Pot|Cauldron";  // co se počítá jako hrnec
	string PanItems = "FryingPan";     // co se počítá jako pánev

	// síla bonusů (za sekundu)
	float RegenHealthPerSecond = 0.15;   // zdraví (max 100)
	float RegenBloodPerSecond = 6;       // krev (max 5000)
	float WarmthPerSecond = 0.03;        // podíl tepelné zásoby (1 = plná)
	float StaminaPerSecond = 0.08;       // podíl výdrže (1 = plná)

	ref array<ref OS_CookRecipe> Recipes = new array<ref OS_CookRecipe>;

	void FillDefaults()
	{
		string meat = "*SteakMeat|ChickenBreastMeat|-HumanSteakMeat";
		string vegetable = "Potato|Tomato|Zucchini|GreenBellPepper|Pumpkin|SlicedPumpkin";
		string mushroom = "*Mushroom|-Psilocybe|-Amanita";
		string fish = "*FilletMeat|Carp|Mackerel";

		OS_CookRecipe r;
		r = new OS_CookRecipe("Gulas", "OS_Dish_Gulas", "pot", 45, "warmth", 30);
		r.Add("*SteakMeat|-HumanSteakMeat", 2);
		r.Add("Potato");
		r.Add("GreenBellPepper|Tomato");
		Recipes.Insert(r);

		r = new OS_CookRecipe("Masova polevka", "OS_Dish_MeatSoup", "pot", 40, "regen", 20);
		r.Add(meat);
		r.Add("Potato|Zucchini|Tomato", 2);
		Recipes.Insert(r);

		r = new OS_CookRecipe("Zeleninova polevka", "OS_Dish_VegSoup", "pot", 30, "stamina", 15);
		r.Add(vegetable, 3);
		Recipes.Insert(r);

		r = new OS_CookRecipe("Rybi polevka", "OS_Dish_FishSoup", "pot", 35, "regen", 20);
		r.Add(fish);
		r.Add("Potato");
		Recipes.Insert(r);

		r = new OS_CookRecipe("Houbova polevka", "OS_Dish_MushroomSoup", "pot", 30, "warmth", 20);
		r.Add(mushroom, 2);
		r.Add("Potato");
		Recipes.Insert(r);

		r = new OS_CookRecipe("Kure na paprice", "OS_Dish_ChickenPaprika", "pot", 45, "regen", 30);
		r.Add("ChickenBreastMeat");
		r.Add("GreenBellPepper");
		r.Add("PowderedMilk");
		Recipes.Insert(r);

		r = new OS_CookRecipe("Rizoto", "OS_Dish_Risotto", "pot", 40, "blood", 15);
		r.Add("Rice");
		r.Add(meat + "|" + mushroom);
		Recipes.Insert(r);

		r = new OS_CookRecipe("Ovocny kompot", "OS_Dish_Compote", "pot", 25, "stamina", 10);
		r.Add("Apple|Pear|Plum", 3);
		Recipes.Insert(r);

		r = new OS_CookRecipe("Pecene maso s bramborami", "OS_Dish_RoastMeat", "pan", 40, "blood", 20);
		r.Add("*SteakMeat|-HumanSteakMeat");
		r.Add("Potato", 2);
		Recipes.Insert(r);

		r = new OS_CookRecipe("Omeleta", "OS_Dish_Omelette", "pan", 25, "stamina", 20);
		r.Add("OS_ChickenEgg", 2);
		r.Add("Tomato|GreenBellPepper");
		Recipes.Insert(r);
	}

	OS_CookRecipe FindByResult(string className)
	{
		foreach (OS_CookRecipe recipe : Recipes)
		{
			if (recipe && recipe.Result == className)
				return recipe;
		}
		return null;
	}

	//! odpovídá classname zadání "A|*B|-C"?
	static bool Matches(string type, string spec)
	{
		array<string> tokens = new array<string>;
		spec.Split("|", tokens);
		string lowerType = type;
		lowerType.ToLower();

		// vyloučení mají přednost
		foreach (string excludeToken : tokens)
		{
			string ex = excludeToken.Trim();
			if (ex.Length() > 1 && ex.Substring(0, 1) == "-")
			{
				string exText = ex.Substring(1, ex.Length() - 1);
				exText.ToLower();
				if (lowerType.Contains(exText))
					return false;
			}
		}

		foreach (string rawToken : tokens)
		{
			string token = rawToken.Trim();
			if (token == "" || token.Substring(0, 1) == "-")
				continue;
			if (token.Substring(0, 1) == "*")
			{
				string part = token.Substring(1, token.Length() - 1);
				part.ToLower();
				if (part != "" && lowerType.Contains(part))
					return true;
				continue;
			}
			if (type == token)
				return true;
			// předek v configu - ne u předmětů módu (vejce je v configu odvozené od brambory)
			if (type.IndexOf("OS_") != 0 && g_Game.IsKindOf(type, token))
				return true;
		}
		return false;
	}
}

// Vaření podle receptů u ohně a bonusy z hotových jídel.

class OS_ServerCooking
{
	protected static const string CFG_FILE = "Cooking.json";

	ref OS_CookingConfig m_Cfg;
	protected ref map<string, float> m_CookingUntil = new map<string, float>;   // Steam ID -> konec vaření

	void OS_ServerCooking()
	{
		OS_CookingConfig defaults = new OS_CookingConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_CookingConfig>.Load(CFG_FILE, defaults, new OS_CookingConfig());
		if (!m_Cfg.Recipes)
			m_Cfg.Recipes = new array<ref OS_CookRecipe>;
	}

	// ------------------------------------------------------------ oheň, nádobí, suroviny

	//! hořící ohniště nebo zapnutý plynový vařič v dosahu
	EntityAI FindFire(PlayerBase player)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(player.GetPosition(), m_Cfg.FireRadius, objects, null);
		foreach (Object obj : objects)
		{
			FireplaceBase fireplace = FireplaceBase.Cast(obj);
			if (fireplace && fireplace.IsBurning())
				return fireplace;
			PortableGasStove stove = PortableGasStove.Cast(obj);
			if (stove && stove.GetCompEM() && stove.GetCompEM().IsWorking())
				return stove;
		}
		return null;
	}

	//! předměty hráče (inventář) + věci na ohništi (hrnec na stojanu)
	protected array<EntityAI> Items(PlayerBase player, EntityAI fire)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		if (fire)
		{
			array<EntityAI> fireItems = new array<EntityAI>;
			fire.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, fireItems);
			foreach (EntityAI fireItem : fireItems)
			{
				if (fireItem != fire)
					items.Insert(fireItem);
			}
		}
		return items;
	}

	protected bool Usable(EntityAI item)
	{
		if (!item || item.IsRuined())
			return false;
		Edible_Base food = Edible_Base.Cast(item);
		if (food && (food.IsFoodRotten() || food.IsFoodBurned()))
			return false;
		return true;
	}

	protected bool HasTool(OS_CookRecipe recipe, array<EntityAI> items)
	{
		string spec = "";
		if (recipe.Tool == "pot")
			spec = m_Cfg.PotItems;
		else if (recipe.Tool == "pan")
			spec = m_Cfg.PanItems;
		if (spec == "")
			return true;

		foreach (EntityAI item : items)
		{
			if (item && !item.IsRuined() && OS_CookingConfig.Matches(item.GetType(), spec))
				return true;
		}
		return false;
	}

	//! najde suroviny pro recept (každý kus jen jednou); prázdné = chybí
	protected array<EntityAI> FindIngredients(OS_CookRecipe recipe, array<EntityAI> items, out string missing)
	{
		array<EntityAI> used = new array<EntityAI>;
		missing = "";
		foreach (OS_CookIngredient ingredient : recipe.Ingredients)
		{
			int found = 0;
			foreach (EntityAI item : items)
			{
				if (found >= ingredient.Count)
					break;
				if (used.Find(item) > -1 || !Usable(item))
					continue;
				if (OS_CookingConfig.Matches(item.GetType(), ingredient.Items))
				{
					used.Insert(item);
					found++;
				}
			}
			if (found < ingredient.Count)
			{
				missing = ingredient.Items;
				return null;
			}
		}
		return used;
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_COOK_OPEN && rpcType != OS_RPC_COOK_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive() || !m_Cfg.Enabled)
			return true;

		EntityAI fire = FindFire(player);
		if (!fire)
		{
			OS_Util.Notify(player, OS_T("Vareni"), OS_T("Musis stat u horiciho ohne nebo zapnuteho varice."));
			return true;
		}

		if (rpcType == OS_RPC_COOK_ACTION)
		{
			int index;
			if (ctx.Read(index))
				Cook(player, fire, index);
		}

		SendData(player, fire);
		return true;
	}

	protected void Cook(PlayerBase player, EntityAI fire, int index)
	{
		if (index < 0 || index >= m_Cfg.Recipes.Count())
			return;
		OS_CookRecipe recipe = m_Cfg.Recipes[index];
		string id = OS_Util.SteamId(player);

		float busyUntil;
		if (m_CookingUntil.Find(id, busyUntil) && g_Game.GetTickTime() < busyUntil)
		{
			OS_Util.Notify(player, OS_T("Vareni"), OS_T("Uz neco varis - pockej, az to bude hotove."));
			return;
		}

		array<EntityAI> items = Items(player, fire);
		if (!HasTool(recipe, items))
		{
			if (recipe.Tool == "pan")
				OS_Util.Notify(player, OS_T("Vareni"), OS_T("Potrebujes panev."));
			else
				OS_Util.Notify(player, OS_T("Vareni"), OS_T("Potrebujes hrnec nebo kotlik."));
			return;
		}

		string missing;
		array<EntityAI> used = FindIngredients(recipe, items, missing);
		if (!used)
		{
			OS_Util.Notify(player, OS_T("Vareni"), OS_F("Chybi suroviny pro %1.", OS_T(recipe.Name)));
			return;
		}

		// suroviny se spotřebují hned, jídlo je hotové po CookSeconds
		foreach (EntityAI item : used)
		{
			g_Game.ObjectDelete(item);
		}

		float seconds = Math.Max(1, recipe.CookSeconds);
		m_CookingUntil.Set(id, g_Game.GetTickTime() + seconds);
		int shownSeconds = seconds;
		OS_Util.Notify(player, OS_T("Vareni"), OS_F("Varis %1 - hotovo za %2 s. Zustan u ohne.", OS_T(recipe.Name), shownSeconds.ToString()), 5);
		int delayMs = seconds * 1000;
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Finish, delayMs, false, id, index);
	}

	protected void Finish(string steamId, int index)
	{
		m_CookingUntil.Remove(steamId);
		if (index < 0 || index >= m_Cfg.Recipes.Count())
			return;
		OS_CookRecipe recipe = m_Cfg.Recipes[index];
		PlayerBase player = OS_Util.FindPlayerBySteamId(steamId);
		if (!player || !player.IsAlive())
			return;

		if (!FindFire(player))
		{
			OS_Util.Notify(player, OS_T("Vareni"), OS_F("%1 se spalilo - odesel jsi od ohne.", OS_T(recipe.Name)));
			return;
		}

		int count = Math.Max(1, recipe.ResultCount);
		for (int i = 0; i < count; i++)
		{
			if (!OS_Util.GiveItem(player, recipe.Result))
				Print(OS_TAG + "VARENI: nepodarilo se vytvorit '" + recipe.Result + "' (spatny classname v Cooking.json?)");
		}
		OS_Util.Notify(player, OS_T("Vareni"), OS_F("Hotovo: %1", OS_T(recipe.Name)), 5);
		if (OS_Server.Get() && OS_Server.Get().m_Skills)
			OS_Server.Get().m_Skills.OnCooked(player);
	}

	protected void SendData(PlayerBase player, EntityAI fire)
	{
		array<EntityAI> items = Items(player, fire);
		array<string> names = new array<string>;
		array<string> details = new array<string>;
		array<int> ready = new array<int>;

		foreach (OS_CookRecipe recipe : m_Cfg.Recipes)
		{
			if (!recipe)
				continue;
			names.Insert(OS_T(recipe.Name));

			string detail = "";
			foreach (OS_CookIngredient ingredient : recipe.Ingredients)
			{
				if (detail != "")
					detail = detail + ", ";
				detail = detail + ingredient.Count.ToString() + "x " + DescribeSpec(ingredient.Items);
			}
			string toolName = OS_T("jen ohen");
			if (recipe.Tool == "pot")
				toolName = OS_T("hrnec");
			else if (recipe.Tool == "pan")
				toolName = OS_T("panev");
			int seconds = recipe.CookSeconds;
			detail = OS_F("%1   |   %2, %3 s", detail, toolName, seconds.ToString());
			if (recipe.Buff != "")
			{
				int minutes = recipe.BuffMinutes;
				detail = OS_F("%1   |   bonus: %2 (%3 min)", detail, BuffName(recipe.Buff), minutes.ToString());
			}
			details.Insert(detail);

			string missing;
			bool ok = HasTool(recipe, items) && FindIngredients(recipe, items, missing) != null;
			if (ok)
				ready.Insert(1);
			else
				ready.Insert(0);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(names);
		rpc.Write(details);
		rpc.Write(ready);
		rpc.Send(null, OS_RPC_COOK_DATA, true, player.GetIdentity());
	}

	//! "*SteakMeat|ChickenBreastMeat|-Human..." -> čitelný popis
	protected string DescribeSpec(string spec)
	{
		array<string> tokens = new array<string>;
		spec.Split("|", tokens);
		string text = "";
		foreach (string raw : tokens)
		{
			string token = raw.Trim();
			if (token == "" || token.Substring(0, 1) == "-")
				continue;
			string label;
			if (token == "*SteakMeat")
				label = OS_T("maso");
			else if (token == "*Mushroom")
				label = OS_T("houby");
			else if (token == "*FilletMeat")
				label = OS_T("ryba");
			else if (token.Substring(0, 1) == "*")
				label = token.Substring(1, token.Length() - 1);
			else
				label = OS_Util.DisplayNameOf(token);
			if (text != "")
				text = text + " / ";
			text = text + label;
		}
		return text;
	}

	string BuffName(string buff)
	{
		if (buff == "regen")
			return OS_T("regenerace zdravi");
		if (buff == "blood")
			return OS_T("doplneni krve");
		if (buff == "warmth")
			return OS_T("teplo");
		if (buff == "stamina")
			return OS_T("vydrz");
		return buff;
	}

	// ------------------------------------------------------------ bonusy

	//! hráč snědl jídlo z receptu
	void OnConsumed(PlayerBase player, string className)
	{
		OS_CookRecipe recipe = m_Cfg.FindByResult(className);
		if (!recipe || recipe.Buff == "" || recipe.BuffMinutes <= 0)
			return;

		bool same = player.m_OS_BuffType == recipe.Buff && player.m_OS_BuffUntil > g_Game.GetTickTime();
		player.m_OS_BuffType = recipe.Buff;
		player.m_OS_BuffUntil = g_Game.GetTickTime() + recipe.BuffMinutes * 60;
		if (!same)
		{
			int minutes = recipe.BuffMinutes;
			OS_Util.Notify(player, OS_T("Bonus z jidla"), OS_F("%1 na %2 min", BuffName(recipe.Buff), minutes.ToString()), 5);
		}
	}

	void OnTick(array<Man> players, float delta)
	{
		float now = g_Game.GetTickTime();
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || player.m_OS_BuffType == "" || !player.IsAlive())
				continue;

			if (now > player.m_OS_BuffUntil)
			{
				OS_Util.Notify(player, OS_T("Bonus z jidla"), OS_F("Bonus skoncil: %1", BuffName(player.m_OS_BuffType)), 4);
				player.m_OS_BuffType = "";
				continue;
			}
			ApplyBuff(player, player.m_OS_BuffType, delta);
		}
	}

	protected void ApplyBuff(PlayerBase player, string buff, float delta)
	{
		if (buff == "regen")
		{
			player.AddHealth("", "", m_Cfg.RegenHealthPerSecond * delta);
		}
		else if (buff == "blood")
		{
			player.AddHealth("", "Blood", m_Cfg.RegenBloodPerSecond * delta);
		}
		else if (buff == "warmth")
		{
			if (player.GetStatHeatBuffer())
			{
				float heatMax = player.GetStatHeatBuffer().GetMax();
				float heat = player.GetStatHeatBuffer().Get() + heatMax * m_Cfg.WarmthPerSecond * delta;
				player.GetStatHeatBuffer().Set(Math.Min(heat, heatMax));
			}
		}
		else if (buff == "stamina")
		{
			StaminaHandler stamina = player.GetStaminaHandler();
			if (stamina)
			{
				float cap = stamina.GetStaminaCap();
				stamina.SetStamina(Math.Min(cap, stamina.GetStamina() + cap * m_Cfg.StaminaPerSecond * delta));
			}
		}
	}
}

modded class Edible_Base
{
	//! bonus z hotového jídla (jen server)
	override void OnConsume(float amount, PlayerBase consumer)
	{
		super.OnConsume(amount, consumer);
		if (g_Game.IsServer() && consumer && OS_Server.Get() && OS_Server.Get().m_Cooking)
			OS_Server.Get().m_Cooking.OnConsumed(consumer, GetType());
	}
}

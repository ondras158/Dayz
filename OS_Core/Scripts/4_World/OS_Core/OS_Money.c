// Hotovost jako bankovky. Které hodnoty se používají, určuje Bank.json (Denominations).
// Jeden předmět = balíček bankovek stejné hodnoty, počet kusů (quantity) = počet bankovek.
// Hodnota bankovky je v configu (bcValue). Částky menší než nejmenší bankovka jdou na účet.

class OS_Money : Inventory_Base
{
	//! hodnoty, pro které existuje třída OS_Money_<hodnota> v config.cpp
	static array<int> Supported()
	{
		array<int> values = {5000, 2000, 1000, 500, 200, 100, 50, 20, 10, 5, 2, 1};
		return values;
	}

	protected static ref array<int> s_Denominations;

	//! server: nastaví používané bankovky (z Bank.json), seřadí od největší
	static void SetDenominations(array<int> values)
	{
		s_Denominations = new array<int>;
		array<int> supportedValues = Supported();
		foreach (int supported : supportedValues)
		{
			if (values && values.Find(supported) > -1)
				s_Denominations.Insert(supported);
		}

		if (s_Denominations.Count() == 0)
		{
			Print(OS_TAG + "Bank.json: zadne platne Denominations, pouzivam euro (500..5)");
			s_Denominations.Insert(500);
			s_Denominations.Insert(200);
			s_Denominations.Insert(100);
			s_Denominations.Insert(50);
			s_Denominations.Insert(20);
			s_Denominations.Insert(10);
			s_Denominations.Insert(5);
		}
	}

	//! hodnoty od největší
	static array<int> Denominations()
	{
		if (!s_Denominations)
			SetDenominations(null);
		return s_Denominations;
	}

	static int Smallest()
	{
		array<int> values = Denominations();
		return values[values.Count() - 1];
	}

	//! název s měnou (klient zná měnu ze serveru po připojení)
	override string GetDisplayName()
	{
		string name = super.GetDisplayName();
		if (OS_ClientSettings.Currency != "")
			name = OS_F("Bankovka %1 %2", OS_GetDenominationText(), OS_ClientSettings.Currency);
		return name;
	}

	protected string OS_GetDenominationText()
	{
		return OS_GetDenomination().ToString();
	}

	int OS_GetDenomination()
	{
		int value = ConfigGetInt("bcValue");
		if (value <= 0)
			value = 1;
		return value;
	}

	int OS_GetNotes()
	{
		return Math.Round(GetQuantity());
	}

	int OS_GetAmount()
	{
		return OS_GetNotes() * OS_GetDenomination();
	}

	// --------------------------------------------------------- práce s hotovostí hráče (server)

	//! balíčky hráče seřazené od nejvyšší hodnoty
	protected static void Collect(PlayerBase player, array<OS_Money> result)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		// všechny hodnoty (i ty, které server už nevydává - hráč je pořád může utratit)
		array<int> allValues = Supported();
		foreach (int denomination : allValues)
		{
			foreach (EntityAI ent : items)
			{
				OS_Money money = OS_Money.Cast(ent);
				if (money && !money.IsRuined() && money.OS_GetDenomination() == denomination)
					result.Insert(money);
			}
		}
	}

	//! Kolik má hráč hotovosti u sebe (i v batohu)
	static int CountCash(PlayerBase player)
	{
		if (!player)
			return 0;

		array<OS_Money> stacks = new array<OS_Money>;
		Collect(player, stacks);

		int total = 0;
		foreach (OS_Money stack : stacks)
		{
			total += stack.OS_GetAmount();
		}
		return total;
	}

	protected static void RemoveNotes(OS_Money stack, int notes)
	{
		if (notes <= 0)
			return;
		if (notes >= stack.OS_GetNotes())
			g_Game.ObjectDelete(stack);
		else
			stack.SetQuantity(stack.OS_GetNotes() - notes);
	}

	//! Odebere hotovost a vrátí drobné. false = hráč nemá dost.
	//! leftover = část drobných pod 10 Kr, kterou nejde vrátit v bankovkách (volající ji dá na účet)
	static bool TakeCash(PlayerBase player, int amount, out int leftover)
	{
		leftover = 0;
		if (amount <= 0)
			return true;
		if (CountCash(player) < amount)
			return false;

		array<OS_Money> stacks = new array<OS_Money>;
		Collect(player, stacks);

		// 1) zaplatit co nejpřesněji od největších bankovek
		int remaining = amount;
		array<OS_Money> leftoverStacks = new array<OS_Money>;
		foreach (OS_Money stack : stacks)
		{
			int denomination = stack.OS_GetDenomination();
			int notes = stack.OS_GetNotes();
			int use = remaining / denomination;
			if (use > notes)
				use = notes;

			remaining -= use * denomination;
			if (use < notes)
				leftoverStacks.Insert(stack);
			RemoveNotes(stack, use);
		}

		if (remaining <= 0)
			return true;

		// 2) zbytek zaplatit nejmenší bankovkou, která stačí, a vrátit drobné
		OS_Money breakStack;
		foreach (OS_Money candidate : leftoverStacks)
		{
			if (candidate && candidate.OS_GetDenomination() >= remaining)
			{
				if (!breakStack || candidate.OS_GetDenomination() < breakStack.OS_GetDenomination())
					breakStack = candidate;
			}
		}

		if (!breakStack)
		{
			// nemělo by nastat (celkem má dost), pro jistotu vzít libovolnou zbývající bankovku
			if (leftoverStacks.Count() == 0)
				return false;
			breakStack = leftoverStacks[0];
		}

		int change = breakStack.OS_GetDenomination() - remaining;
		RemoveNotes(breakStack, 1);
		leftover = GiveCash(player, change);
		return true;
	}

	//! Vydá hotovost v bankovkách (do inventáře, co se nevejde, na zem).
	//! Vrací zbytek pod 10 Kr, který v bankovkách vyplatit nejde.
	static int GiveCash(PlayerBase player, int amount)
	{
		if (amount <= 0)
			return 0;

		array<int> denominations = Denominations();
		foreach (int denomination : denominations)
		{
			int notes = amount / denomination;
			amount -= notes * denomination;

			int safety = 0;
			while (notes > 0 && safety < 50)
			{
				safety++;

				OS_Money stack = OS_Money.Cast(OS_Util.GiveItem(player, "OS_Money_" + denomination.ToString()));
				if (!stack)
				{
					Print(OS_TAG + "CHYBA: nepodarilo se vytvorit OS_Money_" + denomination.ToString());
					return amount + notes * denomination;
				}

				int max = stack.GetQuantityMax();
				if (max <= 0)
					max = 100;

				int chunk = notes;
				if (chunk > max)
					chunk = max;

				stack.SetQuantity(chunk);
				notes -= chunk;
			}
		}
		return amount;
	}
}

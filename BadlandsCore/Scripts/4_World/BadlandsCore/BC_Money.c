// Hotovost jako bankovky 10 / 50 / 100 / 500 / 1000 Kr.
// Jeden předmět = balíček bankovek stejné hodnoty, počet kusů (quantity) = počet bankovek.
// Hodnota bankovky je v configu (bcValue). Částky pod 10 Kr nejde vyplatit v hotovosti - jdou na účet.

class BC_Money : Inventory_Base
{
	//! hodnoty od největší (musí odpovídat třídám BC_Money_<hodnota> v config.cpp)
	static array<int> Denominations()
	{
		array<int> values = {1000, 500, 100, 50, 10};
		return values;
	}

	int BC_GetDenomination()
	{
		int value = ConfigGetInt("bcValue");
		if (value <= 0)
			value = 1;
		return value;
	}

	int BC_GetNotes()
	{
		return Math.Round(GetQuantity());
	}

	int BC_GetAmount()
	{
		return BC_GetNotes() * BC_GetDenomination();
	}

	// --------------------------------------------------------- práce s hotovostí hráče (server)

	//! balíčky hráče seřazené od nejvyšší hodnoty
	protected static void Collect(PlayerBase player, array<BC_Money> result)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		array<int> denominations = Denominations();
		foreach (int denomination : denominations)
		{
			foreach (EntityAI ent : items)
			{
				BC_Money money = BC_Money.Cast(ent);
				if (money && !money.IsRuined() && money.BC_GetDenomination() == denomination)
					result.Insert(money);
			}
		}
	}

	//! Kolik má hráč hotovosti u sebe (i v batohu)
	static int CountCash(PlayerBase player)
	{
		if (!player)
			return 0;

		array<BC_Money> stacks = new array<BC_Money>;
		Collect(player, stacks);

		int total = 0;
		foreach (BC_Money stack : stacks)
		{
			total += stack.BC_GetAmount();
		}
		return total;
	}

	protected static void RemoveNotes(BC_Money stack, int notes)
	{
		if (notes <= 0)
			return;
		if (notes >= stack.BC_GetNotes())
			g_Game.ObjectDelete(stack);
		else
			stack.SetQuantity(stack.BC_GetNotes() - notes);
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

		array<BC_Money> stacks = new array<BC_Money>;
		Collect(player, stacks);

		// 1) zaplatit co nejpřesněji od největších bankovek
		int remaining = amount;
		array<BC_Money> leftoverStacks = new array<BC_Money>;
		foreach (BC_Money stack : stacks)
		{
			int denomination = stack.BC_GetDenomination();
			int notes = stack.BC_GetNotes();
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
		BC_Money breakStack;
		foreach (BC_Money candidate : leftoverStacks)
		{
			if (candidate && candidate.BC_GetDenomination() >= remaining)
			{
				if (!breakStack || candidate.BC_GetDenomination() < breakStack.BC_GetDenomination())
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

		int change = breakStack.BC_GetDenomination() - remaining;
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

				BC_Money stack = BC_Money.Cast(BC_Util.GiveItem(player, "BC_Money_" + denomination.ToString()));
				if (!stack)
				{
					Print(BC_TAG + "CHYBA: nepodarilo se vytvorit BC_Money_" + denomination.ToString());
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

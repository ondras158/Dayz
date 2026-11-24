// Hotovost jako předmět. Počet kusů (quantity) = částka.

class BC_Money : Inventory_Base
{
	int BC_GetAmount()
	{
		return Math.Round(GetQuantity());
	}

	// --------------------------------------------------------- práce s hotovostí hráče (server)

	protected static void Collect(PlayerBase player, array<BC_Money> result)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI ent : items)
		{
			BC_Money money = BC_Money.Cast(ent);
			if (money && !money.IsRuined())
				result.Insert(money);
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

	//! Odebere hotovost (z více balíčků). false = hráč nemá dost.
	static bool TakeCash(PlayerBase player, int amount)
	{
		if (amount <= 0)
			return true;
		if (CountCash(player) < amount)
			return false;

		array<BC_Money> stacks = new array<BC_Money>;
		Collect(player, stacks);

		int remaining = amount;
		foreach (BC_Money stack : stacks)
		{
			if (remaining <= 0)
				break;

			int have = stack.BC_GetAmount();
			if (have <= remaining)
			{
				remaining -= have;
				g_Game.ObjectDelete(stack);
			}
			else
			{
				stack.SetQuantity(have - remaining);
				remaining = 0;
			}
		}
		return true;
	}

	//! Vydá hotovost (do inventáře, co se nevejde, na zem)
	static void GiveCash(PlayerBase player, int amount)
	{
		int safety = 0;
		while (amount > 0 && safety < 100)
		{
			safety++;

			BC_Money stack = BC_Money.Cast(BC_Util.GiveItem(player, "BC_Money"));
			if (!stack)
			{
				Print(BC_TAG + "CHYBA: nepodarilo se vytvorit BC_Money pro " + BC_Util.PlayerName(player));
				return;
			}

			int max = stack.GetQuantityMax();
			if (max <= 0)
				max = 10000;

			int chunk = amount;
			if (chunk > max)
				chunk = max;

			stack.SetQuantity(chunk);
			amount -= chunk;
		}
	}
}

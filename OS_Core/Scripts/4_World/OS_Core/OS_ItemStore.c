// Uložení předmětu i s obsahem a přílohami do JSON a jeho znovuvytvoření (smrt, bazar).

class OS_ItemStore
{
	static OS_SavedItem SaveItem(EntityAI item, string slot)
	{
		OS_SavedItem saved = new OS_SavedItem();
		saved.Type = item.GetType();
		saved.Slot = slot;
		saved.Health = item.GetHealth("", "");
		ItemBase ib = ItemBase.Cast(item);
		if (ib)
		{
			if (ib.HasQuantity())
				saved.Quantity = ib.GetQuantity();
			if (ib.GetLiquidType() > 0)
				saved.Liquid = ib.GetLiquidType();
		}
		Magazine mag = Magazine.Cast(item);
		if (mag)
			saved.Ammo = mag.GetAmmoCount();
		SaveChildren(item, saved.Items);
		return saved;
	}

	static void SaveChildren(EntityAI parent, array<ref OS_SavedItem> list)
	{
		if (!parent.GetInventory())
			return;
		for (int i = 0; i < parent.GetInventory().AttachmentCount(); i++)
		{
			EntityAI attachment = parent.GetInventory().GetAttachmentFromIndex(i);
			if (!attachment)
				continue;
			InventoryLocation il = new InventoryLocation();
			string slotName = "";
			if (attachment.GetInventory().GetCurrentInventoryLocation(il))
				slotName = InventorySlots.GetSlotName(il.GetSlot());
			list.Insert(SaveItem(attachment, slotName));
		}
		CargoBase cargo = parent.GetInventory().GetCargo();
		if (cargo)
		{
			for (int c = 0; c < cargo.GetItemCount(); c++)
			{
				EntityAI cargoItem = cargo.GetItem(c);
				if (cargoItem)
					list.Insert(SaveItem(cargoItem, ""));
			}
		}
	}

	static void RestoreItem(EntityAI parent, OS_SavedItem saved, PlayerBase owner)
	{
		if (!saved || saved.Type == "")
			return;
		EntityAI item;
		if (saved.Slot != "")
		{
			int slotId = InventorySlots.GetSlotIdFromString(saved.Slot);
			if (slotId != InventorySlots.INVALID)
				item = parent.GetInventory().CreateAttachmentEx(saved.Type, slotId);
		}
		if (!item)
			item = parent.GetInventory().CreateInInventory(saved.Type);
		if (!item && parent != owner)
			item = owner.GetInventory().CreateInInventory(saved.Type);
		if (!item)
			item = EntityAI.Cast(g_Game.CreateObjectEx(saved.Type, owner.GetPosition(), ECE_PLACE_ON_SURFACE));
		if (item)
			Apply(item, saved, owner);
	}

	static void Apply(EntityAI item, OS_SavedItem saved, PlayerBase owner)
	{
		if (saved.Health >= 0)
			item.SetHealth("", "", saved.Health);
		ItemBase ib = ItemBase.Cast(item);
		if (ib)
		{
			if (saved.Liquid > 0)
				ib.SetLiquidType(saved.Liquid);
			if (saved.Quantity >= 0 && ib.HasQuantity())
				ib.SetQuantity(saved.Quantity);
		}
		Magazine mag = Magazine.Cast(item);
		if (mag && saved.Ammo >= 0)
			mag.ServerSetAmmoCount(saved.Ammo);
		foreach (OS_SavedItem child : saved.Items)
		{
			RestoreItem(item, child, owner);
		}
	}
}

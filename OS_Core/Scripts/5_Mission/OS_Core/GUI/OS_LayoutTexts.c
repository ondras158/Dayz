// VYGENEROVÁNO nástrojem tools/generate_layouts.py - neupravuj ručně.
// Texty z layoutů; OS_MenuBase je po otevření menu přeloží do jazyka serveru.

class OS_LayoutTexts
{
	static void Apply(string layoutName, Widget root)
	{
		if (!root)
			return;
		if (layoutName == "spawn_menu.layout")
		{
			Set(root, "Title", "Vyber misto spawnu");
			Set(root, "BtnSpawn", "Spawnout zde");
			Set(root, "BtnStay", "Zustat tady");
		}
		if (layoutName == "admin_menu.layout")
		{
			Set(root, "Title", "Admin menu");
			Set(root, "PlayersLabel", "Hraci online");
			Set(root, "BtnRefresh", "Obnovit seznam");
			Set(root, "Target", "Cil:");
			Set(root, "BtnHeal", "Vylecit");
			Set(root, "BtnKill", "Zabit");
			Set(root, "BtnTpTo", "Teleport k nemu");
			Set(root, "BtnTpHere", "Teleport ke mne");
			Set(root, "BtnGod", "Godmode");
			Set(root, "ItemLabel", "Predmet: classname [pocet]  napr. Apple 5");
			Set(root, "BtnSpawn", "Dat predmet cili");
			Set(root, "ArgLabel", "Castka penez / souradnice X Z / polomer [nazev]");
			Set(root, "BtnMoney", "Dat penize cili");
			Set(root, "BtnTpCoords", "Teleport na X Z");
			Set(root, "BtnUnlockCar", "Odemknout nejblizsi auto");
			Set(root, "BtnPvpAdd", "PvP zona zde");
			Set(root, "BtnPvpRemove", "Smazat PvP zonu");
			Set(root, "BtnTools", "Nastroje (COT)");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "trader_menu.layout")
		{
			Set(root, "Title", "Obchod");
			Set(root, "CatLabel", "Kategorie");
			Set(root, "BtnSearch", "Hledat");
			Set(root, "BtnBuy", "Koupit vybrane");
			Set(root, "SellLabel", "Tvoje veci k prodeji");
			Set(root, "BtnSell", "Prodat vybrane");
			Set(root, "BtnClose", "Zavrit");
			Set(root, "BtnMarket", "Bazar hracu");
			Set(root, "AdminLabel", "ADMIN - cena vybraneho predmetu (-1 = nelze, obe -1 = skryt) / nasobeni cele kategorie");
			Set(root, "BuyLabel", "Nakup");
			Set(root, "SellPriceLabel", "Vykup");
			Set(root, "BtnSavePrice", "Ulozit cenu");
			Set(root, "FactorLabel", "Nasobit kat.");
			Set(root, "BtnBulk", "Nasobit");
		}
		if (layoutName == "vehicle_menu.layout")
		{
			Set(root, "Title", "Autocentrum");
			Set(root, "DealerLabel", "Autobazar");
			Set(root, "BtnBuy", "Koupit auto");
			Set(root, "ServiceLabel", "Servis");
			Set(root, "BtnRepair", "Opravit");
			Set(root, "BtnRefuel", "Natankovat");
			Set(root, "BtnParts", "Doplnit dily");
			Set(root, "BtnClaim", "Prepsat auto na sebe");
			Set(root, "BtnStore", "Ulozit do garaze");
			Set(root, "BtnSetSpawn", "Nastavit misto spawnu");
			Set(root, "BtnClose", "Zavrit");
			Set(root, "GarageLabel", "Garaz");
			Set(root, "BtnRetrieve", "Vyzvednout");
		}
		if (layoutName == "atm_menu.layout")
		{
			Set(root, "Title", "Bankomat");
			Set(root, "AmountLabel", "Castka");
			Set(root, "BtnDeposit", "Vlozit");
			Set(root, "BtnWithdraw", "Vybrat");
			Set(root, "BtnDepositAll", "Vlozit vse");
			Set(root, "BtnWithdrawAll", "Vybrat vse");
			Set(root, "BtnGroupDeposit", "Vlozit do skupiny");
			Set(root, "BtnGroupWithdraw", "Vybrat ze skupiny");
			Set(root, "BtnClose", "Zavrit");
			Set(root, "TransferLabel", "Prevod jinemu hraci (online)");
			Set(root, "TargetIdLabel", "nebo Steam64 ID");
			Set(root, "TransferAmountLabel", "Castka");
			Set(root, "BtnTransfer", "Poslat z uctu");
		}
		if (layoutName == "group_menu.layout")
		{
			Set(root, "Title", "Skupina");
			Set(root, "MembersLabel", "Clenove");
			Set(root, "BtnKick", "Vyhodit");
			Set(root, "BtnSetAdmin", "Admin / clen");
			Set(root, "BtnPromote", "Predat vlastnictvi");
			Set(root, "BtnDisband", "Rozpustit skupinu");
			Set(root, "BtnLeave", "Opustit skupinu");
			Set(root, "BtnRadio", "Naladit vysilacku skupiny");
			Set(root, "FreeLabel", "Pozvat hrace (online, bez skupiny)");
			Set(root, "BtnInvite", "Pozvat");
			Set(root, "BtnBuySlot", "Koupit dalsi misto");
			Set(root, "CreateLabel", "Zalozit novou skupinu (nazev)");
			Set(root, "BtnCreate", "Zalozit skupinu");
			Set(root, "BtnAccept", "Prijmout");
			Set(root, "BtnDecline", "Odmitnout");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "admin_tools.layout")
		{
			Set(root, "Title", "Admin nastroje");
			Set(root, "Target", "Cil:");
			Set(root, "PlayerLabel", "Hrac (cil) a admin");
			Set(root, "BtnStrip", "Vysvleknout");
			Set(root, "BtnInvis", "Neviditelnost");
			Set(root, "BtnEsp", "ESP");
			Set(root, "BtnFreecam", "Freecam");
			Set(root, "ReasonLabel", "Duvod (kick/ban), Steam ID (unban, admin), pocet XP");
			Set(root, "BtnKick", "Kick");
			Set(root, "BtnBan", "Ban");
			Set(root, "BtnUnban", "Unban");
			Set(root, "CarLabel", "Vozidlo (do 15 m od cile)");
			Set(root, "BtnCarRepair", "Opravit");
			Set(root, "BtnCarDestroy", "Znicit");
			Set(root, "BtnCarDelete", "Smazat");
			Set(root, "BtnSpawner", "Spawner predmetu a vozidel");
			Set(root, "BtnAddAdmin", "Pridat admina");
			Set(root, "BtnRemoveAdmin", "Odebrat admina");
			Set(root, "BtnGiveXp", "Dat zkusenosti cili (pocet z pole)");
			Set(root, "TimeLabel", "Cas");
			Set(root, "BtnMorning", "6:00");
			Set(root, "BtnNoon", "12:00");
			Set(root, "BtnEvening", "18:00");
			Set(root, "BtnNight", "0:00");
			Set(root, "BtnSetTime", "Nastavit cas");
			Set(root, "WeatherLabel", "Pocasi");
			Set(root, "BtnClear", "Jasno");
			Set(root, "BtnCloudy", "Zatazeno");
			Set(root, "BtnRain", "Dest");
			Set(root, "BtnStorm", "Bourka");
			Set(root, "BtnFog", "Mlha");
			Set(root, "Hint1", "Mapa (M): dvojklik = teleport cile");
			Set(root, "Hint2", "Freecam: klavesa End, Shift+End = teleport");
			Set(root, "Hint3", "WASD, mys, Q/E dolu/nahoru, Shift rychle");
			Set(root, "BtnLockUnlock", "Odemknout zamek");
			Set(root, "BtnLockReset", "Smazat kod");
			Set(root, "BtnBaseRemove", "Smazat base");
			Set(root, "BtnAirdropHere", "Airdrop u cile");
			Set(root, "BtnAirdropRandom", "Airdrop nahodne");
			Set(root, "BtnConvoy", "Vyslat konvoj");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "admin_spawn.layout")
		{
			Set(root, "Title", "Spawner");
			Set(root, "Target", "Cil:");
			Set(root, "BtnSearch", "Hledat");
			Set(root, "CountLabel", "Pocet");
			Set(root, "BtnToInv", "Do inventare cile");
			Set(root, "BtnToGround", "Na zem k cili");
			Set(root, "BtnVehicle", "Spawnout vozidlo");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "base_menu.layout")
		{
			Set(root, "Title", "Base");
			Set(root, "BtnFound", "Zalozit base");
			Set(root, "BtnUpgrade", "Zvetsit polomer");
			Set(root, "BtnRemove", "Zrusit base");
			Set(root, "Hint", "Kodovy zamek na branu: majitel a skupina odemknou bez kodu");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "cook_menu.layout")
		{
			Set(root, "Title", "Vareni podle receptu");
			Set(root, "Hint", "Suroviny a hrnec / panev mej u sebe (nebo hrnec na ohnisti). Hotove jidlo da bonus.");
			Set(root, "Detail1", "Vyber recept vlevo.");
			Set(root, "BtnCook", "Uvarit");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "skills_menu.layout")
		{
			Set(root, "Title", "Dovednosti");
			Set(root, "Detail1", "Vyber dovednost v seznamu.");
			Set(root, "BtnUpgrade", "Vylepsit (1 bod)");
			Set(root, "BtnReset", "Prerozdelit body");
			Set(root, "Hint", "Zkusenosti: zombici, zvirata, vareni, farma a cas na serveru. Vylepseni zustava i po smrti.");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "emote_menu.layout")
		{
			Set(root, "Title", "Emoce a spanek");
			Set(root, "BtnPlay", "Prehrat");
			Set(root, "BtnClose", "Zavrit");
			Set(root, "Hint", "Dvojklik = prehrat. Pohyb emoci ukonci.");
		}
		if (layoutName == "market_menu.layout")
		{
			Set(root, "Title", "Bazar hracu");
			Set(root, "OffersLabel", "Nabidky hracu");
			Set(root, "BtnSearch", "Hledat");
			Set(root, "BtnBuy", "Koupit vybrane");
			Set(root, "BtnWithdraw", "Stahnout moji nabidku");
			Set(root, "SellLabel", "Prodat: tvoje veci a auta v garazi");
			Set(root, "PriceLabel", "Cena");
			Set(root, "BtnList", "Vystavit vybrane");
			Set(root, "BtnClaim", "Vyzvednout vracene veci");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "farm_menu.layout")
		{
			Set(root, "Title", "Farma");
			Set(root, "AnimalsLabel", "Zvirata, stromy a zahony (vyber a kup)");
			Set(root, "BtnBuyAnimal", "Koupit vybrane");
			Set(root, "StorageLabel", "Sklad (vyrobene suroviny)");
			Set(root, "BtnCollect", "Vybrat vybrane");
			Set(root, "BtnCollectAll", "Vybrat vse");
			Set(root, "BtnBuyFeed", "Koupit krmivo");
			Set(root, "BtnBuyFert", "Koupit hnojivo");
			Set(root, "BuildingsLabel", "Stavby na farme");
			Set(root, "BtnBuildStructure", "Postavit vybranou stavbu");
			Set(root, "BtnBuild", "Postavit farmu zde");
			Set(root, "BtnRemove", "Zrusit farmu");
			Set(root, "BtnClose", "Zavrit");
		}
		if (layoutName == "pgarage_menu.layout")
		{
			Set(root, "Title", "Osobni garaz");
			Set(root, "BtnBuild", "Postavit garaz zde");
			Set(root, "BtnSetSpawn", "Nastavit misto pro vozidlo");
			Set(root, "BtnRemove", "Zrusit garaz");
			Set(root, "BtnClose", "Zavrit");
		}
	}

	protected static void Set(Widget root, string widgetName, string text)
	{
		Widget w = root.FindAnyWidget(widgetName);
		TextWidget tw = TextWidget.Cast(w);
		if (tw)
		{
			tw.SetText(OS_T(text));
			return;
		}
		ButtonWidget bw = ButtonWidget.Cast(w);
		if (bw)
			bw.SetText(OS_T(text));
	}
}

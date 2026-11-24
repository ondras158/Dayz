// Autobazar, servis a garáže.

class OS_ServerVehicles
{
	ref OS_VehicleConfig m_Cfg;
	protected OS_Payments m_Payments;
	protected string m_Currency;

	void OS_ServerVehicles(OS_Payments payments, string currency)
	{
		m_Payments = payments;
		m_Currency = currency;

		OS_VehicleConfig defaults = new OS_VehicleConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_VehicleConfig>.Load("Vehicles.json", defaults, new OS_VehicleConfig());

		MakeDirectory(OS_DIR + "Garages");
	}

	// ------------------------------------------------------------ hledání

	OS_VehicleLocation NearLocation(PlayerBase player)
	{
		if (!m_Cfg.Enabled || !player || !m_Cfg.Locations)
			return null;

		foreach (OS_VehicleLocation loc : m_Cfg.Locations)
		{
			if (loc && OS_Util.Distance2D(player.GetPosition(), loc.X, loc.Z) <= loc.Radius)
				return loc;
		}
		return null;
	}

	//! Nejbližší auto u hráče
	protected CarScript FindCar(PlayerBase player)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(player.GetPosition(), m_Cfg.CarSearchRadius, objects, null);

		CarScript best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			CarScript car = CarScript.Cast(obj);
			if (!car || car.IsRuined())
				continue;

			float dist = vector.Distance(player.GetPosition(), car.GetPosition());
			if (dist < bestDist)
			{
				bestDist = dist;
				best = car;
			}
		}
		return best;
	}

	protected OS_VehicleOffer FindOffer(string className)
	{
		foreach (OS_VehicleOffer offer : m_Cfg.Offers)
		{
			if (offer && offer.ClassName == className)
				return offer;
		}

		// barevné varianty (např. Sedan_02_Red -> Sedan_02)
		foreach (OS_VehicleOffer parentOffer : m_Cfg.Offers)
		{
			if (parentOffer && g_Game.IsKindOf(className, parentOffer.ClassName))
				return parentOffer;
		}
		return null;
	}

	protected bool HasCrew(CarScript car)
	{
		for (int i = 0; i < car.CrewSize(); i++)
		{
			if (car.CrewMember(i))
				return true;
		}
		return false;
	}

	protected bool HasCargo(EntityAI entity)
	{
		CargoBase cargo = entity.GetInventory().GetCargo();
		if (cargo && cargo.GetItemCount() > 0)
			return true;

		for (int i = 0; i < entity.GetInventory().AttachmentCount(); i++)
		{
			EntityAI att = entity.GetInventory().GetAttachmentFromIndex(i);
			if (att && att.GetInventory().GetCargo() && att.GetInventory().GetCargo().GetItemCount() > 0)
				return true;
		}
		return false;
	}

	protected bool IsSpawnBlocked(vector pos)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(pos, 5, objects, null);
		foreach (Object obj : objects)
		{
			if (Transport.Cast(obj))
				return true;
		}
		return false;
	}

	// ------------------------------------------------------------ výpočty cen

	protected float DamageFraction(CarScript car)
	{
		float total = 1 - car.GetHealth01("", "");
		int count = 1;

		TStringArray zones = new TStringArray;
		car.GetDamageZones(zones);
		foreach (string zone : zones)
		{
			total += 1 - car.GetHealth01(zone, "");
			count++;
		}

		for (int i = 0; i < car.GetInventory().AttachmentCount(); i++)
		{
			EntityAI att = car.GetInventory().GetAttachmentFromIndex(i);
			if (att)
			{
				total += 1 - att.GetHealth01("", "");
				count++;
			}
		}

		return total / count;
	}

	protected int RepairPrice(CarScript car)
	{
		return Math.Ceil(m_Cfg.RepairFullPrice * DamageFraction(car));
	}

	protected float MissingFuel(CarScript car)
	{
		return car.GetFluidCapacity(CarFluid.FUEL) * (1 - car.GetFluidFraction(CarFluid.FUEL));
	}

	protected int RefuelPrice(CarScript car)
	{
		return Math.Ceil(MissingFuel(car) * m_Cfg.RefuelPricePerLiter);
	}

	// ------------------------------------------------------------ RPC

	//! Klávesa obchodu - pokud hráč stojí u auto-místa, otevře se menu aut
	bool TryOpen(PlayerBase player)
	{
		OS_VehicleLocation loc = NearLocation(player);
		if (!loc)
			return false;

		SendData(player, loc);
		return true;
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_VEHICLE_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		OS_VehicleLocation loc = NearLocation(player);
		if (!loc)
		{
			OS_Util.Notify(player, "Auta", "Musis stat u autobazaru, servisu nebo garaze.");
			return true;
		}

		int action;
		int arg;
		if (!ctx.Read(action) || !ctx.Read(arg))
			return true;

		if (action == OS_VehicleAction.BUY && loc.Dealer)
			Buy(player, loc, arg);
		else if (action == OS_VehicleAction.REPAIR && loc.Service)
			Repair(player);
		else if (action == OS_VehicleAction.REFUEL && loc.Service)
			Refuel(player);
		else if (action == OS_VehicleAction.PARTS && loc.Service)
			AddMissingParts(player);
		else if (action == OS_VehicleAction.STORE && loc.Garage)
			Store(player);
		else if (action == OS_VehicleAction.RETRIEVE && loc.Garage)
			Retrieve(player, loc, arg);
		else if (action == OS_VehicleAction.CLAIM && loc.Service)
			Claim(player);
		else if (action == OS_VehicleAction.KEY_COPY && loc.Service)
			KeyCopy(player);
		else if (action == OS_VehicleAction.CHANGE_LOCK && loc.Service)
			ChangeLock(player);

		SendData(player, loc);
		return true;
	}

	protected void SendData(PlayerBase player, OS_VehicleLocation loc)
	{
		string steamId = OS_Util.SteamId(player);

		array<string> offerClasses = new array<string>;
		array<int> offerPrices = new array<int>;
		if (loc.Dealer)
		{
			foreach (OS_VehicleOffer offer : m_Cfg.Offers)
			{
				offerClasses.Insert(offer.ClassName);
				offerPrices.Insert(offer.Price);
			}
		}

		bool hasCar = false;
		string carClass = "";
		int healthPct = 0;
		int fuelPct = 0;
		int repairPrice = 0;
		int refuelPrice = 0;
		int ownerState = 0;   // 0 = bez majitele, 1 = moje, 2 = cizí
		if (loc.Service || loc.Garage)
		{
			CarScript car = FindCar(player);
			if (car)
			{
				if (car.OS_HasOwner())
				{
					ownerState = 2;
					if (car.OS_GetOwnerId() == steamId)
						ownerState = 1;
				}
				hasCar = true;
				carClass = car.GetType();
				healthPct = Math.Round((1 - DamageFraction(car)) * 100);
				fuelPct = Math.Round(car.GetFluidFraction(CarFluid.FUEL) * 100);
				repairPrice = RepairPrice(car);
				refuelPrice = RefuelPrice(car);
			}
		}

		array<string> stored = new array<string>;
		if (loc.Garage)
		{
			OS_GarageData garage = LoadGarage(steamId);
			foreach (OS_StoredVehicle sv : garage.Vehicles)
			{
				stored.Insert(sv.ClassName);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(loc.Name);
		rpc.Write(m_Currency);
		rpc.Write(m_Payments.Available(player));
		rpc.Write(m_Payments.Cash(player));
		rpc.Write(m_Payments.Bank(player));
		rpc.Write(loc.Dealer);
		rpc.Write(loc.Service);
		rpc.Write(loc.Garage);
		rpc.Write(offerClasses);
		rpc.Write(offerPrices);
		rpc.Write(hasCar);
		rpc.Write(carClass);
		rpc.Write(healthPct);
		rpc.Write(fuelPct);
		rpc.Write(repairPrice);
		rpc.Write(refuelPrice);
		rpc.Write(m_Cfg.PartPrice);
		rpc.Write(m_Cfg.GarageStoreFee);
		rpc.Write(m_Cfg.GarageMaxVehicles);
		rpc.Write(stored);
		rpc.Write(ownerState);
		rpc.Write(m_Cfg.ClaimPrice);
		rpc.Write(m_Cfg.KeyCopyPrice);
		rpc.Write(m_Cfg.ChangeLockPrice);
		rpc.Send(null, OS_RPC_VEHICLE_DATA, true, player.GetIdentity());
	}

	// ------------------------------------------------------------ akce

	protected CarScript SpawnCar(string type, OS_VehicleLocation loc)
	{
		vector pos = OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ);
		Object obj = g_Game.CreateObjectEx(type, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI);
		CarScript car = CarScript.Cast(obj);
		if (!car)
		{
			if (obj)
				g_Game.ObjectDelete(obj);
			Print(OS_TAG + "Nepodarilo se vytvorit auto '" + type + "'");
			return null;
		}

		car.SetOrientation(Vector(loc.SpawnYaw, 0, 0));
		return car;
	}

	protected void Buy(PlayerBase player, OS_VehicleLocation loc, int offerIndex)
	{
		if (offerIndex < 0 || offerIndex >= m_Cfg.Offers.Count())
			return;

		OS_VehicleOffer offer = m_Cfg.Offers[offerIndex];
		if (IsSpawnBlocked(OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ)))
		{
			OS_Util.Notify(player, "Autobazar", "Parkovaci misto je obsazene - odjed s autem, ktere tam stoji.");
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (!m_Payments.TrySpend(player, offer.Price))
		{
			OS_Util.Notify(player, "Autobazar", "Nemas dost penez.");
			return;
		}

		CarScript car = SpawnCar(offer.ClassName, loc);
		if (!car)
		{
			m_Payments.Pay(player, offer.Price);
			OS_Util.Notify(player, "Autobazar", "Auto se nepodarilo vytvorit (spatny classname v Vehicles.json?).");
			return;
		}

		foreach (string part : offer.Parts)
		{
			car.GetInventory().CreateInInventory(part);
		}

		car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL));
		car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL));
		car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE));
		car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT));

		int keyCode = CarScript.OS_GenerateKeyCode();
		car.OS_SetOwnership(keyCode, steamId);
		OS_Util.GiveCarKey(player, keyCode);

		OS_Util.Notify(player, "Autobazar", "Auto je pripravene na parkovisti. Klic mas v inventari.");
		Print(OS_TAG + "AUTO " + OS_Util.PlayerName(player) + " koupil " + offer.ClassName + " za " + offer.Price.ToString());
	}

	protected void Repair(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		int price = RepairPrice(car);
		if (price <= 0)
		{
			OS_Util.Notify(player, "Servis", "Auto je v poradku.");
			return;
		}

		if (!m_Payments.TrySpend(player, price))
		{
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
			return;
		}

		car.SetHealthMax("", "");
		TStringArray zones = new TStringArray;
		car.GetDamageZones(zones);
		foreach (string zone : zones)
		{
			car.SetHealthMax(zone, "");
		}

		for (int i = 0; i < car.GetInventory().AttachmentCount(); i++)
		{
			EntityAI att = car.GetInventory().GetAttachmentFromIndex(i);
			if (att)
				att.SetHealthMax("", "");
		}

		OS_Util.Notify(player, "Servis", "Auto opraveno za " + price.ToString() + " " + m_Currency + ".");
	}

	protected void Refuel(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		int price = RefuelPrice(car);
		if (price <= 0)
		{
			OS_Util.Notify(player, "Servis", "Nadrz je plna.");
			return;
		}

		if (!m_Payments.TrySpend(player, price))
		{
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
			return;
		}

		car.Fill(CarFluid.FUEL, MissingFuel(car));
		OS_Util.Notify(player, "Servis", "Natankovano za " + price.ToString() + " " + m_Currency + ".");
	}

	protected void AddMissingParts(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		OS_VehicleOffer offer = FindOffer(car.GetType());
		if (!offer)
		{
			OS_Util.Notify(player, "Servis", "Pro tento typ auta nemame dily.");
			return;
		}

		// zkusit namontovat všechny díly - povede se jen u volných slotů
		array<EntityAI> created = new array<EntityAI>;
		foreach (string part : offer.Parts)
		{
			EntityAI att = car.GetInventory().CreateAttachment(part);
			if (att)
				created.Insert(att);
		}

		if (created.Count() == 0)
		{
			OS_Util.Notify(player, "Servis", "Auto ma vsechny dily.");
			return;
		}

		int price = created.Count() * m_Cfg.PartPrice;
		if (!m_Payments.TrySpend(player, price))
		{
			foreach (EntityAI undo : created)
			{
				g_Game.ObjectDelete(undo);
			}
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
			return;
		}

		OS_Util.Notify(player, "Servis", "Doplneno dilu: " + created.Count().ToString() + " za " + price.ToString() + " " + m_Currency + ".");
	}

	// ------------------------------------------------------------ klíče a vlastnictví

	protected void Claim(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}
		if (car.OS_HasOwner())
		{
			OS_Util.Notify(player, "Servis", "Auto uz ma majitele.");
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (!m_Payments.TrySpend(player, m_Cfg.ClaimPrice))
		{
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + m_Cfg.ClaimPrice.ToString() + " " + m_Currency + ").");
			return;
		}

		int keyCode = CarScript.OS_GenerateKeyCode();
		car.OS_SetOwnership(keyCode, steamId);
		OS_Util.GiveCarKey(player, keyCode);
		OS_Util.Notify(player, "Servis", "Auto je tvoje. Klic mas v inventari.");
		Print(OS_TAG + "AUTO " + OS_Util.PlayerName(player) + " prepsal na sebe " + car.GetType());
	}

	protected CarScript FindOwnCar(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return null;
		}
		if (!car.OS_HasOwner() || car.OS_GetOwnerId() != OS_Util.SteamId(player))
		{
			OS_Util.Notify(player, "Servis", "Tohle auto ti nepatri.");
			return null;
		}
		return car;
	}

	protected void KeyCopy(PlayerBase player)
	{
		CarScript car = FindOwnCar(player);
		if (!car)
			return;

		if (!m_Payments.TrySpend(player, m_Cfg.KeyCopyPrice))
		{
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + m_Cfg.KeyCopyPrice.ToString() + " " + m_Currency + ").");
			return;
		}

		OS_Util.GiveCarKey(player, car.OS_GetKeyCode());
		OS_Util.Notify(player, "Servis", "Kopie klice je v inventari.");
	}

	protected void ChangeLock(PlayerBase player)
	{
		CarScript car = FindOwnCar(player);
		if (!car)
			return;

		if (!m_Payments.TrySpend(player, m_Cfg.ChangeLockPrice))
		{
			OS_Util.Notify(player, "Servis", "Nemas dost penez (" + m_Cfg.ChangeLockPrice.ToString() + " " + m_Currency + ").");
			return;
		}

		int keyCode = CarScript.OS_GenerateKeyCode();
		car.OS_SetOwnership(keyCode, OS_Util.SteamId(player));
		car.OS_SetLocked(false);
		OS_Util.GiveCarKey(player, keyCode);
		OS_Util.Notify(player, "Servis", "Zamek vymenen - stare klice uz nefunguji. Novy klic mas v inventari.");
	}

	// ------------------------------------------------------------ garáž

	protected string GaragePath(string steamId)
	{
		return OS_DIR + "Garages/" + steamId + ".json";
	}

	protected OS_GarageData LoadGarage(string steamId)
	{
		OS_GarageData data = new OS_GarageData();
		string path = GaragePath(steamId);
		if (!FileExist(path))
			return data;

		string err;
		if (!JsonFileLoader<OS_GarageData>.LoadFile(path, data, err) || !data)
		{
			Print(OS_TAG + "CHYBA pri nacitani garaze " + path + ": " + err);
			data = new OS_GarageData();
		}
		if (!data.Vehicles)
			data.Vehicles = new array<ref OS_StoredVehicle>;
		return data;
	}

	protected bool SaveGarage(string steamId, OS_GarageData data)
	{
		string err;
		if (JsonFileLoader<OS_GarageData>.SaveFile(GaragePath(steamId), data, err))
			return true;

		Print(OS_TAG + "CHYBA pri ukladani garaze " + steamId + ": " + err);
		return false;
	}

	protected void Store(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, "Garaz", "Zadne auto pobliz.");
			return;
		}
		if (HasCrew(car))
		{
			OS_Util.Notify(player, "Garaz", "V aute nekdo sedi.");
			return;
		}
		if (HasCargo(car))
		{
			OS_Util.Notify(player, "Garaz", "Nejdriv vyndej vsechny veci z auta.");
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (m_Cfg.OnlyOwnerCanStore && car.OS_HasOwner() && car.OS_GetOwnerId() != steamId)
		{
			OS_Util.Notify(player, "Garaz", "Tohle auto ti nepatri.");
			return;
		}

		OS_GarageData garage = LoadGarage(steamId);
		if (garage.Vehicles.Count() >= m_Cfg.GarageMaxVehicles)
		{
			OS_Util.Notify(player, "Garaz", "Garaz je plna (max " + m_Cfg.GarageMaxVehicles.ToString() + ").");
			return;
		}

		if (!m_Payments.TrySpend(player, m_Cfg.GarageStoreFee))
		{
			OS_Util.Notify(player, "Garaz", "Nemas dost penez na poplatek.");
			return;
		}

		OS_StoredVehicle sv = new OS_StoredVehicle();
		sv.ClassName = car.GetType();
		sv.Health = car.GetHealth01("", "");

		TStringArray zones = new TStringArray;
		car.GetDamageZones(zones);
		foreach (string zone : zones)
		{
			sv.Zones.Insert(zone);
			sv.ZoneHealth.Insert(car.GetHealth01(zone, ""));
		}

		for (int i = 0; i < car.GetInventory().AttachmentCount(); i++)
		{
			EntityAI att = car.GetInventory().GetAttachmentFromIndex(i);
			if (att)
			{
				sv.Parts.Insert(att.GetType());
				sv.PartHealth.Insert(att.GetHealth01("", ""));
			}
		}

		sv.Fuel = car.GetFluidFraction(CarFluid.FUEL);
		sv.Oil = car.GetFluidFraction(CarFluid.OIL);
		sv.Brake = car.GetFluidFraction(CarFluid.BRAKE);
		sv.Coolant = car.GetFluidFraction(CarFluid.COOLANT);
		sv.KeyCode = car.OS_GetKeyCode();
		sv.OwnerId = car.OS_GetOwnerId();
		sv.Locked = car.OS_IsLocked();

		garage.Vehicles.Insert(sv);
		if (!SaveGarage(steamId, garage))
		{
			m_Payments.Pay(player, m_Cfg.GarageStoreFee);
			OS_Util.Notify(player, "Garaz", "Chyba pri ukladani - auto zustava venku.");
			return;
		}

		g_Game.ObjectDelete(car);
		OS_Util.Notify(player, "Garaz", "Auto ulozeno do garaze.");
		Print(OS_TAG + "GARAZ " + OS_Util.PlayerName(player) + " ulozil " + sv.ClassName);
	}

	protected void Retrieve(PlayerBase player, OS_VehicleLocation loc, int index)
	{
		string steamId = OS_Util.SteamId(player);
		OS_GarageData garage = LoadGarage(steamId);
		if (index < 0 || index >= garage.Vehicles.Count())
			return;

		if (IsSpawnBlocked(OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ)))
		{
			OS_Util.Notify(player, "Garaz", "Parkovaci misto je obsazene.");
			return;
		}

		OS_StoredVehicle sv = garage.Vehicles[index];
		CarScript car = SpawnCar(sv.ClassName, loc);
		if (!car)
		{
			OS_Util.Notify(player, "Garaz", "Auto se nepodarilo vytvorit.");
			return;
		}

		// nejdřív odebrat z garáže, aby nešlo auto zdvojit
		garage.Vehicles.Remove(index);
		if (!SaveGarage(steamId, garage))
		{
			g_Game.ObjectDelete(car);
			OS_Util.Notify(player, "Garaz", "Chyba pri ukladani garaze.");
			return;
		}

		for (int i = 0; i < sv.Parts.Count(); i++)
		{
			EntityAI att = car.GetInventory().CreateAttachment(sv.Parts[i]);
			if (!att)
				att = car.GetInventory().CreateInInventory(sv.Parts[i]);
			if (att && i < sv.PartHealth.Count())
				att.SetHealth01("", "", sv.PartHealth[i]);
		}

		for (int z = 0; z < sv.Zones.Count() && z < sv.ZoneHealth.Count(); z++)
		{
			car.SetHealth01(sv.Zones[z], "", sv.ZoneHealth[z]);
		}
		car.SetHealth01("", "", sv.Health);

		car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL) * sv.Fuel);
		car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL) * sv.Oil);
		car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE) * sv.Brake);
		car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT) * sv.Coolant);

		// auto z garáže patří tomu, kdo ho vyzvedl; staré klíče dál fungují
		if (sv.KeyCode != 0)
			car.OS_SetOwnership(sv.KeyCode, steamId);
		car.OS_SetLocked(false);

		OS_Util.Notify(player, "Garaz", "Auto vyzvednuto - stoji na parkovisti.");
		Print(OS_TAG + "GARAZ " + OS_Util.PlayerName(player) + " vyzvedl " + sv.ClassName);
	}
}

// Autobazar, servis a garáže.

class BC_ServerVehicles
{
	ref BC_VehicleConfig m_Cfg;
	protected BC_Wallets m_Wallets;
	protected string m_Currency;

	void BC_ServerVehicles(BC_Wallets wallets, string currency)
	{
		m_Wallets = wallets;
		m_Currency = currency;

		BC_VehicleConfig defaults = new BC_VehicleConfig();
		defaults.FillDefaults();
		m_Cfg = BC_ConfigFile<BC_VehicleConfig>.Load("Vehicles.json", defaults, new BC_VehicleConfig());

		MakeDirectory(BC_DIR + "Garages");
	}

	// ------------------------------------------------------------ hledání

	BC_VehicleLocation NearLocation(PlayerBase player)
	{
		if (!m_Cfg.Enabled || !player || !m_Cfg.Locations)
			return null;

		foreach (BC_VehicleLocation loc : m_Cfg.Locations)
		{
			if (loc && BC_Util.Distance2D(player.GetPosition(), loc.X, loc.Z) <= loc.Radius)
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

	protected BC_VehicleOffer FindOffer(string className)
	{
		foreach (BC_VehicleOffer offer : m_Cfg.Offers)
		{
			if (offer && offer.ClassName == className)
				return offer;
		}

		// barevné varianty (např. Sedan_02_Red -> Sedan_02)
		foreach (BC_VehicleOffer parentOffer : m_Cfg.Offers)
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
		BC_VehicleLocation loc = NearLocation(player);
		if (!loc)
			return false;

		SendData(player, loc);
		return true;
	}

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != BC_RPC_VEHICLE_ACTION)
			return false;

		PlayerBase player = BC_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		BC_VehicleLocation loc = NearLocation(player);
		if (!loc)
		{
			BC_Util.Notify(player, "Auta", "Musis stat u autobazaru, servisu nebo garaze.");
			return true;
		}

		int action;
		int arg;
		if (!ctx.Read(action) || !ctx.Read(arg))
			return true;

		if (action == BC_VehicleAction.BUY && loc.Dealer)
			Buy(player, loc, arg);
		else if (action == BC_VehicleAction.REPAIR && loc.Service)
			Repair(player);
		else if (action == BC_VehicleAction.REFUEL && loc.Service)
			Refuel(player);
		else if (action == BC_VehicleAction.PARTS && loc.Service)
			AddMissingParts(player);
		else if (action == BC_VehicleAction.STORE && loc.Garage)
			Store(player);
		else if (action == BC_VehicleAction.RETRIEVE && loc.Garage)
			Retrieve(player, loc, arg);

		SendData(player, loc);
		return true;
	}

	protected void SendData(PlayerBase player, BC_VehicleLocation loc)
	{
		string steamId = BC_Util.SteamId(player);

		array<string> offerClasses = new array<string>;
		array<int> offerPrices = new array<int>;
		if (loc.Dealer)
		{
			foreach (BC_VehicleOffer offer : m_Cfg.Offers)
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
		if (loc.Service || loc.Garage)
		{
			CarScript car = FindCar(player);
			if (car)
			{
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
			BC_GarageData garage = LoadGarage(steamId);
			foreach (BC_StoredVehicle sv : garage.Vehicles)
			{
				stored.Insert(sv.ClassName);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(loc.Name);
		rpc.Write(m_Currency);
		rpc.Write(m_Wallets.GetBalance(steamId));
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
		rpc.Send(null, BC_RPC_VEHICLE_DATA, true, player.GetIdentity());
	}

	// ------------------------------------------------------------ akce

	protected CarScript SpawnCar(string type, BC_VehicleLocation loc)
	{
		vector pos = BC_Util.GroundPos(loc.SpawnX, loc.SpawnZ);
		Object obj = g_Game.CreateObjectEx(type, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI);
		CarScript car = CarScript.Cast(obj);
		if (!car)
		{
			if (obj)
				g_Game.ObjectDelete(obj);
			Print(BC_TAG + "Nepodarilo se vytvorit auto '" + type + "'");
			return null;
		}

		car.SetOrientation(Vector(loc.SpawnYaw, 0, 0));
		return car;
	}

	protected void Buy(PlayerBase player, BC_VehicleLocation loc, int offerIndex)
	{
		if (offerIndex < 0 || offerIndex >= m_Cfg.Offers.Count())
			return;

		BC_VehicleOffer offer = m_Cfg.Offers[offerIndex];
		if (IsSpawnBlocked(BC_Util.GroundPos(loc.SpawnX, loc.SpawnZ)))
		{
			BC_Util.Notify(player, "Autobazar", "Parkovaci misto je obsazene - odjed s autem, ktere tam stoji.");
			return;
		}

		string steamId = BC_Util.SteamId(player);
		if (!m_Wallets.TrySpend(steamId, offer.Price))
		{
			BC_Util.Notify(player, "Autobazar", "Nemas dost penez.");
			return;
		}

		CarScript car = SpawnCar(offer.ClassName, loc);
		if (!car)
		{
			m_Wallets.Add(steamId, offer.Price);
			BC_Util.Notify(player, "Autobazar", "Auto se nepodarilo vytvorit (spatny classname v Vehicles.json?).");
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

		BC_Util.Notify(player, "Autobazar", "Auto je pripravene na parkovisti.");
		Print(BC_TAG + "AUTO " + BC_Util.PlayerName(player) + " koupil " + offer.ClassName + " za " + offer.Price.ToString());
	}

	protected void Repair(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			BC_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		int price = RepairPrice(car);
		if (price <= 0)
		{
			BC_Util.Notify(player, "Servis", "Auto je v poradku.");
			return;
		}

		if (!m_Wallets.TrySpend(BC_Util.SteamId(player), price))
		{
			BC_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
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

		BC_Util.Notify(player, "Servis", "Auto opraveno za " + price.ToString() + " " + m_Currency + ".");
	}

	protected void Refuel(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			BC_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		int price = RefuelPrice(car);
		if (price <= 0)
		{
			BC_Util.Notify(player, "Servis", "Nadrz je plna.");
			return;
		}

		if (!m_Wallets.TrySpend(BC_Util.SteamId(player), price))
		{
			BC_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
			return;
		}

		car.Fill(CarFluid.FUEL, MissingFuel(car));
		BC_Util.Notify(player, "Servis", "Natankovano za " + price.ToString() + " " + m_Currency + ".");
	}

	protected void AddMissingParts(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			BC_Util.Notify(player, "Servis", "Zadne auto pobliz.");
			return;
		}

		BC_VehicleOffer offer = FindOffer(car.GetType());
		if (!offer)
		{
			BC_Util.Notify(player, "Servis", "Pro tento typ auta nemame dily.");
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
			BC_Util.Notify(player, "Servis", "Auto ma vsechny dily.");
			return;
		}

		int price = created.Count() * m_Cfg.PartPrice;
		if (!m_Wallets.TrySpend(BC_Util.SteamId(player), price))
		{
			foreach (EntityAI undo : created)
			{
				g_Game.ObjectDelete(undo);
			}
			BC_Util.Notify(player, "Servis", "Nemas dost penez (" + price.ToString() + " " + m_Currency + ").");
			return;
		}

		BC_Util.Notify(player, "Servis", "Doplneno dilu: " + created.Count().ToString() + " za " + price.ToString() + " " + m_Currency + ".");
	}

	// ------------------------------------------------------------ garáž

	protected string GaragePath(string steamId)
	{
		return BC_DIR + "Garages/" + steamId + ".json";
	}

	protected BC_GarageData LoadGarage(string steamId)
	{
		BC_GarageData data = new BC_GarageData();
		string path = GaragePath(steamId);
		if (!FileExist(path))
			return data;

		string err;
		if (!JsonFileLoader<BC_GarageData>.LoadFile(path, data, err) || !data)
		{
			Print(BC_TAG + "CHYBA pri nacitani garaze " + path + ": " + err);
			data = new BC_GarageData();
		}
		if (!data.Vehicles)
			data.Vehicles = new array<ref BC_StoredVehicle>;
		return data;
	}

	protected bool SaveGarage(string steamId, BC_GarageData data)
	{
		string err;
		if (JsonFileLoader<BC_GarageData>.SaveFile(GaragePath(steamId), data, err))
			return true;

		Print(BC_TAG + "CHYBA pri ukladani garaze " + steamId + ": " + err);
		return false;
	}

	protected void Store(PlayerBase player)
	{
		CarScript car = FindCar(player);
		if (!car)
		{
			BC_Util.Notify(player, "Garaz", "Zadne auto pobliz.");
			return;
		}
		if (HasCrew(car))
		{
			BC_Util.Notify(player, "Garaz", "V aute nekdo sedi.");
			return;
		}
		if (HasCargo(car))
		{
			BC_Util.Notify(player, "Garaz", "Nejdriv vyndej vsechny veci z auta.");
			return;
		}

		string steamId = BC_Util.SteamId(player);
		BC_GarageData garage = LoadGarage(steamId);
		if (garage.Vehicles.Count() >= m_Cfg.GarageMaxVehicles)
		{
			BC_Util.Notify(player, "Garaz", "Garaz je plna (max " + m_Cfg.GarageMaxVehicles.ToString() + ").");
			return;
		}

		if (!m_Wallets.TrySpend(steamId, m_Cfg.GarageStoreFee))
		{
			BC_Util.Notify(player, "Garaz", "Nemas dost penez na poplatek.");
			return;
		}

		BC_StoredVehicle sv = new BC_StoredVehicle();
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

		garage.Vehicles.Insert(sv);
		if (!SaveGarage(steamId, garage))
		{
			m_Wallets.Add(steamId, m_Cfg.GarageStoreFee);
			BC_Util.Notify(player, "Garaz", "Chyba pri ukladani - auto zustava venku.");
			return;
		}

		g_Game.ObjectDelete(car);
		BC_Util.Notify(player, "Garaz", "Auto ulozeno do garaze.");
		Print(BC_TAG + "GARAZ " + BC_Util.PlayerName(player) + " ulozil " + sv.ClassName);
	}

	protected void Retrieve(PlayerBase player, BC_VehicleLocation loc, int index)
	{
		string steamId = BC_Util.SteamId(player);
		BC_GarageData garage = LoadGarage(steamId);
		if (index < 0 || index >= garage.Vehicles.Count())
			return;

		if (IsSpawnBlocked(BC_Util.GroundPos(loc.SpawnX, loc.SpawnZ)))
		{
			BC_Util.Notify(player, "Garaz", "Parkovaci misto je obsazene.");
			return;
		}

		BC_StoredVehicle sv = garage.Vehicles[index];
		CarScript car = SpawnCar(sv.ClassName, loc);
		if (!car)
		{
			BC_Util.Notify(player, "Garaz", "Auto se nepodarilo vytvorit.");
			return;
		}

		// nejdřív odebrat z garáže, aby nešlo auto zdvojit
		garage.Vehicles.Remove(index);
		if (!SaveGarage(steamId, garage))
		{
			g_Game.ObjectDelete(car);
			BC_Util.Notify(player, "Garaz", "Chyba pri ukladani garaze.");
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

		BC_Util.Notify(player, "Garaz", "Auto vyzvednuto - stoji na parkovisti.");
		Print(BC_TAG + "GARAZ " + BC_Util.PlayerName(player) + " vyzvedl " + sv.ClassName);
	}
}

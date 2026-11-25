// Autobazar, servis a garáže.

class OS_ServerVehicles
{
	ref OS_VehicleConfig m_Cfg;
	protected OS_Payments m_Payments;
	protected OS_ServerGroups m_Groups;
	protected OS_ServerAdmin m_Admin;
	protected string m_Currency;
	protected ref OS_PersonalGaragesData m_Personal;

	protected static const string PERSONAL_FILE = "PersonalGarages.json";

	void OS_ServerVehicles(OS_Payments payments, string currency, OS_ServerGroups groups, OS_ServerAdmin admin)
	{
		m_Groups = groups;
		m_Admin = admin;
		m_Payments = payments;
		m_Currency = currency;

		OS_VehicleConfig defaults = new OS_VehicleConfig();
		defaults.FillDefaults();
		m_Cfg = OS_ConfigFile<OS_VehicleConfig>.Load("Vehicles.json", defaults, new OS_VehicleConfig());

		MakeDirectory(OS_DIR + "Garages");
		LoadPersonal();
	}

	protected void SaveConfig()
	{
		string err;
		if (!JsonFileLoader<OS_VehicleConfig>.SaveFile(OS_DIR + "Vehicles.json", m_Cfg, err))
			Print(OS_TAG + "CHYBA pri ukladani Vehicles.json: " + err);
	}

	// ------------------------------------------------------------ osobní garáže

	protected void LoadPersonal()
	{
		m_Personal = new OS_PersonalGaragesData();
		string path = OS_DIR + PERSONAL_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_PersonalGaragesData>.LoadFile(path, m_Personal, err))
		{
			Print(OS_TAG + "CHYBA pri nacitani " + path + ": " + err);
			m_Personal = new OS_PersonalGaragesData();
		}
		if (!m_Personal.Garages)
			m_Personal.Garages = new array<ref OS_PersonalGarage>;
	}

	protected void SavePersonal()
	{
		string err;
		if (!JsonFileLoader<OS_PersonalGaragesData>.SaveFile(OS_DIR + PERSONAL_FILE, m_Personal, err))
			Print(OS_TAG + "CHYBA pri ukladani osobnich garazi: " + err);
	}

	protected OS_PersonalGarage GetPersonal(string steamId)
	{
		foreach (OS_PersonalGarage garage : m_Personal.Garages)
		{
			if (garage && garage.OwnerId == steamId)
				return garage;
		}
		return null;
	}

	//! osobní garáž hráče jako dočasné "místo" (jen garáž, žádný prodej ani servis)
	protected OS_VehicleLocation PersonalLocation(OS_PersonalGarage garage)
	{
		OS_VehicleLocation loc = new OS_VehicleLocation(OS_T("Moje garaz"), garage.X, garage.Z);
		loc.Radius = m_Cfg.PersonalGarageRadius;
		loc.Garage = true;
		loc.Personal = true;
		loc.SpawnX = garage.SpawnX;
		loc.SpawnZ = garage.SpawnZ;
		loc.SpawnYaw = garage.SpawnYaw;
		loc.SpawnClearRadius = 5;
		return loc;
	}

	//! klientovi poloha jeho garáže (mapa, menu K)
	void SendPersonalGarage(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		OS_PersonalGarage garage = GetPersonal(OS_Util.SteamId(player));
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Cfg.PersonalGarages);
		rpc.Write(m_Cfg.PersonalGarageBuildPrice);
		rpc.Write(m_Currency);
		bool hasGarage = garage != null;
		rpc.Write(hasGarage);
		float gx = 0;
		float gz = 0;
		if (garage)
		{
			gx = garage.X;
			gz = garage.Z;
		}
		rpc.Write(gx);
		rpc.Write(gz);
		rpc.Write(m_Cfg.PersonalGarageRadius);
		rpc.Send(null, OS_RPC_PGARAGE_DATA, true, player.GetIdentity());
	}

	protected bool TooCloseToOtherGarage(vector pos, string exceptOwner)
	{
		foreach (OS_PersonalGarage other : m_Personal.Garages)
		{
			if (other && other.OwnerId != exceptOwner && OS_Util.Distance2D(pos, other.X, other.Z) < m_Cfg.PersonalGarageMinDistance)
				return true;
		}
		foreach (OS_VehicleLocation loc : m_Cfg.Locations)
		{
			if (loc && OS_Util.Distance2D(pos, loc.X, loc.Z) < m_Cfg.PersonalGarageMinDistance + loc.Radius)
				return true;
		}
		return false;
	}

	protected void HandlePersonalAction(PlayerBase player, int action)
	{
		string steamId = OS_Util.SteamId(player);
		OS_PersonalGarage garage = GetPersonal(steamId);

		if (action == OS_PersonalGarageAction.BUILD)
		{
			if (!m_Cfg.PersonalGarages)
				return;
			if (garage)
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_T("Uz mas osobni garaz. Nejdriv ji zrus."));
				return;
			}
			if (TooCloseToOtherGarage(player.GetPosition(), steamId))
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_T("Tady garaz postavit nejde - je moc blizko jine garaze nebo obchodu."));
				return;
			}
			if (!m_Payments.TrySpend(player, m_Cfg.PersonalGarageBuildPrice))
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_F("Stavba stoji %1 %2.", m_Cfg.PersonalGarageBuildPrice.ToString(), m_Currency));
				return;
			}

			// místo pro vozidlo: 8 m před hráčem, ve směru pohledu (hráč si ho pak upraví)
			vector dir = player.GetDirection();
			dir[1] = 0;
			dir.Normalize();
			vector playerPos = player.GetPosition();
			vector spawn = playerPos + dir * 8;
			vector angles = dir.VectorToAngles();

			garage = new OS_PersonalGarage();
			garage.OwnerId = steamId;
			garage.X = playerPos[0];
			garage.Z = playerPos[2];
			garage.SpawnX = spawn[0];
			garage.SpawnZ = spawn[2];
			garage.SpawnYaw = angles[0];
			m_Personal.Garages.Insert(garage);
			SavePersonal();

			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Garaz postavena. Nastav si misto pro vozidlo v menu garaze (klavesa K)."));
			Print(OS_TAG + "GARAZ " + OS_Util.PlayerName(player) + " postavil osobni garaz na " + garage.X.ToString() + " " + garage.Z.ToString());
		}
		else if (action == OS_PersonalGarageAction.REMOVE)
		{
			if (!garage)
				return;
			m_Personal.Garages.RemoveItem(garage);
			SavePersonal();
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Osobni garaz zrusena. Ulozena auta ti zustavaji (vyzvednes je v jakekoli garazi)."));
		}
	}

	// ------------------------------------------------------------ místo spawnu

	protected void HandleSetSpawn(PlayerBase player, ParamsReadContext ctx)
	{
		int target;
		float x;
		float z;
		float yaw;
		if (!ctx.Read(target) || !ctx.Read(x) || !ctx.Read(z) || !ctx.Read(yaw))
			return;

		vector pos = Vector(x, 0, z);

		if (target == OS_SpawnTarget.PERSONAL_GARAGE)
		{
			OS_PersonalGarage garage = GetPersonal(OS_Util.SteamId(player));
			if (!garage)
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_T("Nemas osobni garaz."));
				return;
			}
			if (OS_Util.Distance2D(pos, garage.X, garage.Z) > m_Cfg.PersonalSpawnMaxDistance)
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_F("Misto je moc daleko od garaze (max %1 m).", m_Cfg.PersonalSpawnMaxDistance.ToString()));
				return;
			}
			garage.SpawnX = x;
			garage.SpawnZ = z;
			garage.SpawnYaw = yaw;
			SavePersonal();
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Misto pro vozidlo ulozeno."));
			return;
		}

		// místo z Vehicles.json - jen admin
		if (!m_Admin || !m_Admin.IsAdmin(player.GetIdentity()))
		{
			OS_Util.Notify(player, OS_T("Auta"), OS_T("Misto spawnu tady nastavuje jen admin."));
			return;
		}

		OS_VehicleLocation loc = NearestConfigLocation(pos);
		if (!loc)
		{
			OS_Util.Notify(player, OS_T("Auta"), OS_F("Zadne autocentrum, heliport, pristav ani verejna garaz pobliz (max %1 m).", m_Cfg.AdminSpawnMaxDistance.ToString()));
			return;
		}

		loc.SpawnX = x;
		loc.SpawnZ = z;
		loc.SpawnYaw = yaw;
		SaveConfig();
		OS_Util.Notify(player, OS_T("Auta"), OS_F("Misto spawnu ulozeno: %1", OS_T(loc.Name)));
		Print(OS_TAG + "ADMIN " + OS_Util.PlayerName(player) + " nastavil spawn " + loc.Name + " na " + x.ToString() + " " + z.ToString() + " (" + yaw.ToString() + ")");
	}

	protected OS_VehicleLocation NearestConfigLocation(vector pos)
	{
		OS_VehicleLocation best;
		float bestDist = m_Cfg.AdminSpawnMaxDistance;
		foreach (OS_VehicleLocation loc : m_Cfg.Locations)
		{
			if (!loc)
				continue;
			float dist = OS_Util.Distance2D(pos, loc.X, loc.Z);
			if (dist <= bestDist)
			{
				bestDist = dist;
				best = loc;
			}
		}
		return best;
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

		// vlastní osobní garáž
		if (m_Cfg.PersonalGarages)
		{
			OS_PersonalGarage garage = GetPersonal(OS_Util.SteamId(player));
			if (garage && OS_Util.Distance2D(player.GetPosition(), garage.X, garage.Z) <= m_Cfg.PersonalGarageRadius)
				return PersonalLocation(garage);
		}
		return null;
	}

	//! plné všechny kapaliny (vozidlo bez dané kapaliny má kapacitu 0)
	protected void FillAll(Transport vehicle)
	{
		vehicle.FillScript(ETransportFluid.FUEL, vehicle.GetFluidCapacityScript(ETransportFluid.FUEL));
		vehicle.FillScript(ETransportFluid.OIL, vehicle.GetFluidCapacityScript(ETransportFluid.OIL));
		vehicle.FillScript(ETransportFluid.BRAKE, vehicle.GetFluidCapacityScript(ETransportFluid.BRAKE));
		vehicle.FillScript(ETransportFluid.COOLANT, vehicle.GetFluidCapacityScript(ETransportFluid.COOLANT));
	}

	//! Nejbližší vozidlo u hráče (auto, loď, helikoptéra)
	protected Transport FindCar(PlayerBase player)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(player.GetPosition(), m_Cfg.CarSearchRadius, objects, null);

		Transport best;
		float bestDist = 999999;
		foreach (Object obj : objects)
		{
			Transport car = Transport.Cast(obj);
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

	// ------------------------------------------------------------ admin nástroje

	//! vozidlo pro admina: díly z nabídky (pokud ho Vehicles.json zná) a plné nádrže
	Transport AdminSpawn(string type, vector pos, float yaw)
	{
		Object obj = g_Game.CreateObjectEx(type, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI);
		Transport vehicle = Transport.Cast(obj);
		if (!vehicle)
		{
			if (obj)
				g_Game.ObjectDelete(obj);
			return null;
		}
		vehicle.SetOrientation(Vector(yaw, 0, 0));

		OS_VehicleOffer offer = FindOffer(type);
		if (offer)
		{
			foreach (string part : offer.Parts)
			{
				vehicle.GetInventory().CreateInInventory(part);
			}
		}
		FillAll(vehicle);
		return vehicle;
	}

	//! oprava všech částí vozidla a doplnění kapalin
	void AdminRepair(Transport car)
	{
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
		FillAll(car);
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

	protected bool HasCrew(Transport car)
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

	//! kategorie, které místo prodává ("car,bike" -> [car, bike])
	protected array<string> LocationCategories(OS_VehicleLocation loc)
	{
		array<string> parts = new array<string>;
		string raw = OS_TraderScanner.Lower(loc.DealerCategory);
		raw.Replace(" ", "");
		raw.Split(",", parts);
		if (parts.Count() == 0)
			parts.Insert("car");
		return parts;
	}

	//! indexy nabídek, které dané místo prodává (podle kategorie)
	protected array<int> OffersFor(OS_VehicleLocation loc)
	{
		array<int> result = new array<int>;
		array<string> wanted = LocationCategories(loc);
		for (int i = 0; i < m_Cfg.Offers.Count(); i++)
		{
			OS_VehicleOffer offer = m_Cfg.Offers[i];
			if (!offer)
				continue;
			string category = OS_TraderScanner.Lower(offer.Category);
			if (category == "")
				category = "car";
			if (wanted.Find(category) > -1)
				result.Insert(i);
		}
		return result;
	}

	//! "car" / "bike" / "heli" / "boat", víc kategorií = "mixed"
	protected string LocationCategory(OS_VehicleLocation loc)
	{
		array<string> categories = LocationCategories(loc);
		if (categories.Count() > 1)
			return "mixed";
		return categories[0];
	}

	protected string DealerTitle(OS_VehicleLocation loc)
	{
		string category = LocationCategory(loc);
		if (category == "heli")
			return OS_T("Helikoptery");
		if (category == "boat")
			return OS_T("Lode");
		if (category == "bike")
			return OS_T("Motorky");
		if (category == "mixed")
			return OS_T("Prodej vozidel");
		return OS_T("Autobazar");
	}

	protected bool IsSpawnBlocked(vector pos, float radius = 5)
	{
		array<Object> objects = new array<Object>;
		g_Game.GetObjectsAtPosition(pos, radius, objects, null);
		foreach (Object obj : objects)
		{
			if (Transport.Cast(obj))
				return true;
		}
		return false;
	}

	// ------------------------------------------------------------ výpočty cen

	protected float DamageFraction(Transport car)
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

	protected int RepairPrice(Transport car)
	{
		return Math.Ceil(m_Cfg.RepairFullPrice * DamageFraction(car));
	}

	protected float MissingFuel(Transport car)
	{
		return car.GetFluidCapacityScript(ETransportFluid.FUEL) * (1 - car.GetFluidFractionScript(ETransportFluid.FUEL));
	}

	protected int RefuelPrice(Transport car)
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
		if (rpcType != OS_RPC_VEHICLE_ACTION && rpcType != OS_RPC_VEHICLE_SET_SPAWN && rpcType != OS_RPC_PGARAGE_OPEN && rpcType != OS_RPC_PGARAGE_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !player.IsAlive())
			return true;

		if (rpcType == OS_RPC_VEHICLE_SET_SPAWN)
		{
			HandleSetSpawn(player, ctx);
			return true;
		}
		if (rpcType == OS_RPC_PGARAGE_OPEN || rpcType == OS_RPC_PGARAGE_ACTION)
		{
			int garageAction;
			if (rpcType == OS_RPC_PGARAGE_ACTION && ctx.Read(garageAction))
				HandlePersonalAction(player, garageAction);
			SendPersonalGarage(player);
			return true;
		}

		OS_VehicleLocation loc = NearLocation(player);
		if (!loc)
		{
			OS_Util.Notify(player, OS_T("Auta"), OS_T("Musis stat u autobazaru, servisu nebo garaze."));
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
			array<int> available = OffersFor(loc);
			foreach (int offerIdx : available)
			{
				offerClasses.Insert(m_Cfg.Offers[offerIdx].ClassName);
				offerPrices.Insert(m_Cfg.Offers[offerIdx].Price);
			}
		}

		bool hasCar = false;
		string carClass = "";
		int healthPct = 0;
		int fuelPct = 0;
		int repairPrice = 0;
		int refuelPrice = 0;
		int ownerState = 0;   // 0 = bez majitele, 1 = moje, 2 = cizí, 3 = auto mé skupiny
		if (loc.Service || loc.Garage)
		{
			Transport car = FindCar(player);
			if (car)
			{
				if (car.OS_HasOwner())
				{
					ownerState = 2;
					if (car.OS_GetOwnerId() == steamId)
						ownerState = 1;
					else if (car.OS_GetGroupId() != 0 && car.OS_GetGroupId() == PlayerGroupId(steamId))
						ownerState = 3;
				}
				hasCar = true;
				carClass = car.GetType();
				healthPct = Math.Round((1 - DamageFraction(car)) * 100);
				fuelPct = Math.Round(car.GetFluidFractionScript(ETransportFluid.FUEL) * 100);
				repairPrice = RepairPrice(car);
				refuelPrice = RefuelPrice(car);
			}
		}

		array<string> stored = new array<string>;
		array<int> storedGroup = new array<int>;   // 1 = auto ze skupinové garáže
		int garageMax = m_Cfg.GarageMaxVehicles;
		if (loc.Garage)
		{
			OS_GarageData personal = LoadGarage(steamId);
			foreach (OS_StoredVehicle sv : personal.Vehicles)
			{
				stored.Insert(sv.ClassName);
				storedGroup.Insert(0);
			}

			string groupKey = GroupGarageKey(steamId);
			if (groupKey != "")
			{
				garageMax = m_Cfg.GroupGarageMaxVehicles;
				OS_GarageData shared = LoadGarage(groupKey);
				foreach (OS_StoredVehicle gsv : shared.Vehicles)
				{
					stored.Insert(gsv.ClassName);
					storedGroup.Insert(1);
				}
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(OS_T(loc.Name));
		rpc.Write(LocationCategory(loc));
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
		rpc.Write(garageMax);
		rpc.Write(stored);
		rpc.Write(ownerState);
		rpc.Write(m_Cfg.ClaimPrice);
		rpc.Write(storedGroup);

		// nastavení místa spawnu: admin u míst z konfigu, hráč u své garáže
		bool isAdmin = m_Admin && m_Admin.IsAdmin(player.GetIdentity());
		rpc.Write(loc.Personal);
		rpc.Write(isAdmin && !loc.Personal && (loc.Dealer || loc.Garage));
		rpc.Write(loc.X);
		rpc.Write(loc.Z);
		rpc.Write(loc.SpawnX);
		rpc.Write(loc.SpawnZ);
		rpc.Write(loc.SpawnYaw);
		rpc.Write(loc.SpawnOnWater);
		rpc.Send(null, OS_RPC_VEHICLE_DATA, true, player.GetIdentity());
	}

	// ------------------------------------------------------------ akce

	//! vytvoří libovolné vozidlo (auto i helikoptéru z módu)
	protected Transport SpawnVehicle(string type, OS_VehicleLocation loc)
	{
		vector pos = OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ);
		int flags = ECE_PLACE_ON_SURFACE | ECE_INITAI;
		if (loc.SpawnOnWater)
		{
			// loď na hladinu (ne na dno)
			float sea = g_Game.SurfaceGetSeaLevel();
			if (pos[1] < sea)
				pos[1] = sea;
			flags = ECE_CREATEPHYSICS | ECE_INITAI | ECE_KEEPHEIGHT;
		}
		Object obj = g_Game.CreateObjectEx(type, pos, flags);
		Transport vehicle = Transport.Cast(obj);
		if (!vehicle)
		{
			if (obj)
				g_Game.ObjectDelete(obj);
			Print(OS_TAG + "Nepodarilo se vytvorit vozidlo '" + type + "' (neexistuje, nebo to neni vozidlo)");
			return null;
		}

		vehicle.SetOrientation(Vector(loc.SpawnYaw, 0, 0));
		return vehicle;
	}

	protected void Buy(PlayerBase player, OS_VehicleLocation loc, int listIndex)
	{
		// index v nabídce tohoto místa -> index v Offers
		array<int> available = OffersFor(loc);
		if (listIndex < 0 || listIndex >= available.Count())
			return;

		OS_VehicleOffer offer = m_Cfg.Offers[available[listIndex]];
		string title = DealerTitle(loc);

		if (IsSpawnBlocked(OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ), loc.SpawnClearRadius))
		{
			OS_Util.Notify(player, title, OS_T("Misto pro vozidlo je obsazene - nejdriv odvez to, ktere tam stoji."));
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (!m_Payments.TrySpend(player, offer.Price))
		{
			OS_Util.Notify(player, OS_T("Autobazar"), OS_T("Nemas dost penez."));
			return;
		}

		Transport vehicle = SpawnVehicle(offer.ClassName, loc);
		if (!vehicle)
		{
			m_Payments.Pay(player, offer.Price);
			OS_Util.Notify(player, title, OS_T("Vozidlo se nepodarilo vytvorit (spatny classname v Vehicles.json, nebo chybi mod?)."));
			return;
		}

		foreach (string part : offer.Parts)
		{
			vehicle.GetInventory().CreateInInventory(part);
		}

		// plné nádrže (auta, lodě i helikoptéry - jen kapaliny, které vozidlo má) + majitel
		FillAll(vehicle);
		vehicle.OS_SetOwner(steamId);
		OS_Util.Notify(player, title, OS_T("Vozidlo je pripravene a patri tobe. Zamknes ho klavesou F."));
		Print(OS_TAG + "AUTO " + OS_Util.PlayerName(player) + " koupil " + offer.ClassName + " za " + offer.Price.ToString());
	}

	protected void Repair(PlayerBase player)
	{
		Transport car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Zadne auto pobliz."));
			return;
		}

		int price = RepairPrice(car);
		if (price <= 0)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Auto je v poradku."));
			return;
		}

		if (!m_Payments.TrySpend(player, price))
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_F("Nemas dost penez (%1 %2).", price.ToString(), m_Currency));
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

		OS_Util.Notify(player, OS_T("Servis"), OS_F("Auto opraveno za %1 %2.", price.ToString(), m_Currency));
	}

	protected void Refuel(PlayerBase player)
	{
		Transport car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Zadne auto pobliz."));
			return;
		}

		int price = RefuelPrice(car);
		if (price <= 0)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Nadrz je plna."));
			return;
		}

		if (!m_Payments.TrySpend(player, price))
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_F("Nemas dost penez (%1 %2).", price.ToString(), m_Currency));
			return;
		}

		car.FillScript(ETransportFluid.FUEL, MissingFuel(car));
		OS_Util.Notify(player, OS_T("Servis"), OS_F("Natankovano za %1 %2.", price.ToString(), m_Currency));
	}

	protected void AddMissingParts(PlayerBase player)
	{
		Transport car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Zadne auto pobliz."));
			return;
		}

		OS_VehicleOffer offer = FindOffer(car.GetType());
		if (!offer)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Pro tento typ auta nemame dily."));
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
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Auto ma vsechny dily."));
			return;
		}

		int price = created.Count() * m_Cfg.PartPrice;
		if (!m_Payments.TrySpend(player, price))
		{
			foreach (EntityAI undo : created)
			{
				g_Game.ObjectDelete(undo);
			}
			OS_Util.Notify(player, OS_T("Servis"), OS_F("Nemas dost penez (%1 %2).", price.ToString(), m_Currency));
			return;
		}

		OS_Util.Notify(player, OS_T("Servis"), OS_F("Doplneno dilu: %1 za %2 %3.", created.Count().ToString(), price.ToString(), m_Currency));
	}

	// ------------------------------------------------------------ vlastnictví

	//! ID skupiny hráče (0 = není ve skupině)
	protected int PlayerGroupId(string steamId)
	{
		if (!m_Groups)
			return 0;
		OS_Group group = m_Groups.GetGroupOf(steamId);
		if (!group)
			return 0;
		return group.Id;
	}

	protected void Claim(PlayerBase player)
	{
		Transport car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Zadne auto pobliz."));
			return;
		}
		if (car.OS_HasOwner())
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_T("Auto uz ma majitele."));
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (!m_Payments.TrySpend(player, m_Cfg.ClaimPrice))
		{
			OS_Util.Notify(player, OS_T("Servis"), OS_F("Nemas dost penez (%1 %2).", m_Cfg.ClaimPrice.ToString(), m_Currency));
			return;
		}

		car.OS_SetOwner(steamId);
		OS_Util.Notify(player, OS_T("Servis"), OS_T("Auto je tvoje. Zamknes ho u auta klavesou F."));
		Print(OS_TAG + "AUTO " + OS_Util.PlayerName(player) + " prepsal na sebe " + car.GetType());
	}

	// ------------------------------------------------------------ garáž

	//! klíč skupinové garáže ("group_<id>"), "" = hráč není ve skupině / sdílení vypnuto
	protected string GroupGarageKey(string steamId)
	{
		if (!m_Cfg.SharedGroupGarage || !m_Groups)
			return "";
		OS_Group group = m_Groups.GetGroupOf(steamId);
		if (!group)
			return "";
		return "group_" + group.Id.ToString();
	}

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
		Transport car = FindCar(player);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Zadne auto pobliz."));
			return;
		}
		if (HasCrew(car))
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("V aute nekdo sedi."));
			return;
		}
		if (HasCargo(car))
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Nejdriv vyndej vsechny veci z auta."));
			return;
		}

		string steamId = OS_Util.SteamId(player);
		if (m_Cfg.OnlyOwnerCanStore && car.OS_HasOwner() && car.OS_GetOwnerId() != steamId)
		{
			// auto spoluhráče ze skupiny (nebo auto sdílené s mou skupinou) uložit jde
			bool sharedWithMe = car.OS_GetGroupId() != 0 && car.OS_GetGroupId() == PlayerGroupId(steamId);
			if (!sharedWithMe && (!m_Groups || !m_Groups.SameGroup(steamId, car.OS_GetOwnerId())))
			{
				OS_Util.Notify(player, OS_T("Garaz"), OS_T("Tohle auto ti nepatri."));
				return;
			}
		}

		// ve skupině se ukládá do společné garáže skupiny
		string garageKey = steamId;
		int garageMax = m_Cfg.GarageMaxVehicles;
		string groupKey = GroupGarageKey(steamId);
		if (groupKey != "")
		{
			garageKey = groupKey;
			garageMax = m_Cfg.GroupGarageMaxVehicles;
		}

		OS_GarageData garage = LoadGarage(garageKey);
		if (garage.Vehicles.Count() >= garageMax)
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_F("Garaz je plna (max %1).", garageMax.ToString()));
			return;
		}

		if (!m_Payments.TrySpend(player, m_Cfg.GarageStoreFee))
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Nemas dost penez na poplatek."));
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

		sv.Fuel = car.GetFluidFractionScript(ETransportFluid.FUEL);
		sv.Oil = car.GetFluidFractionScript(ETransportFluid.OIL);
		sv.Brake = car.GetFluidFractionScript(ETransportFluid.BRAKE);
		sv.Coolant = car.GetFluidFractionScript(ETransportFluid.COOLANT);
		sv.OwnerId = car.OS_GetOwnerId();
		sv.GroupId = car.OS_GetGroupId();
		sv.Locked = car.OS_IsLocked();
		// auto ve skupinové garáži se automaticky přiřadí skupině
		if (groupKey != "")
		{
			sv.GroupId = PlayerGroupId(steamId);
			if (sv.OwnerId == "")
				sv.OwnerId = steamId;
		}

		garage.Vehicles.Insert(sv);
		if (!SaveGarage(garageKey, garage))
		{
			m_Payments.Pay(player, m_Cfg.GarageStoreFee);
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Chyba pri ukladani - auto zustava venku."));
			return;
		}

		g_Game.ObjectDelete(car);
		if (groupKey != "")
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Auto ulozeno do garaze skupiny."));
		else
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Auto ulozeno do garaze."));
		Print(OS_TAG + "GARAZ " + OS_Util.PlayerName(player) + " ulozil " + sv.ClassName + " (" + garageKey + ")");
	}

	protected void Retrieve(PlayerBase player, OS_VehicleLocation loc, int index)
	{
		string steamId = OS_Util.SteamId(player);

		// seznam = nejdřív osobní auta, potom auta skupiny
		string garageKey = steamId;
		bool fromGroup = false;
		OS_GarageData garage = LoadGarage(steamId);
		if (index >= garage.Vehicles.Count())
		{
			string groupKey = GroupGarageKey(steamId);
			if (groupKey == "")
				return;
			index -= garage.Vehicles.Count();
			garageKey = groupKey;
			fromGroup = true;
			garage = LoadGarage(groupKey);
		}
		if (index < 0 || index >= garage.Vehicles.Count())
			return;

		if (IsSpawnBlocked(OS_Util.GroundPos(loc.SpawnX, loc.SpawnZ), loc.SpawnClearRadius))
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Parkovaci misto je obsazene."));
			return;
		}

		OS_StoredVehicle sv = garage.Vehicles[index];
		Transport car = SpawnVehicle(sv.ClassName, loc);
		if (!car)
		{
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Auto se nepodarilo vytvorit."));
			return;
		}

		// nejdřív odebrat z garáže, aby nešlo auto zdvojit
		garage.Vehicles.Remove(index);
		if (!SaveGarage(garageKey, garage))
		{
			g_Game.ObjectDelete(car);
			OS_Util.Notify(player, OS_T("Garaz"), OS_T("Chyba pri ukladani garaze."));
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

		car.FillScript(ETransportFluid.FUEL, car.GetFluidCapacityScript(ETransportFluid.FUEL) * sv.Fuel);
		car.FillScript(ETransportFluid.OIL, car.GetFluidCapacityScript(ETransportFluid.OIL) * sv.Oil);
		car.FillScript(ETransportFluid.BRAKE, car.GetFluidCapacityScript(ETransportFluid.BRAKE) * sv.Brake);
		car.FillScript(ETransportFluid.COOLANT, car.GetFluidCapacityScript(ETransportFluid.COOLANT) * sv.Coolant);

		// osobní garáž: auto patří tomu, kdo ho vyzvedl
		// skupinová garáž: auto dál patří původnímu majiteli a je sdílené se skupinou
		string owner = steamId;
		if (fromGroup && sv.OwnerId != "")
			owner = sv.OwnerId;
		if (sv.OwnerId != "" || fromGroup)
			car.OS_SetOwner(owner);
		if (fromGroup)
			car.OS_SetGroup(sv.GroupId);
		car.OS_SetLocked(false);

		OS_Util.Notify(player, OS_T("Garaz"), OS_T("Auto vyzvednuto - stoji na parkovisti."));
		Print(OS_TAG + "GARAZ " + OS_Util.PlayerName(player) + " vyzvedl " + sv.ClassName + " (" + garageKey + ")");
	}
}

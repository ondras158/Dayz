// Data menu aut - pořadí čtení musí odpovídat OS_ServerVehicles.SendData

class OS_VehicleMenuData
{
	string LocationName;
	string Category;   // car / bike / heli / boat / mixed
	string Currency;
	int Balance;   // kolik jde utratit
	int Cash;
	int Bank;
	bool Dealer;
	bool Service;
	bool Garage;
	ref array<string> OfferClasses = new array<string>;
	ref array<int> OfferPrices = new array<int>;
	bool HasCar;
	string CarClass;
	int HealthPct;
	int FuelPct;
	int RepairPrice;
	int RefuelPrice;
	int PartPrice;
	int StoreFee;
	int GarageMax;
	ref array<string> Stored = new array<string>;
	int OwnerState;      // 0 = bez majitele, 1 = moje, 2 = cizí, 3 = auto mé skupiny
	int ClaimPrice;
	ref array<int> StoredGroup = new array<int>;

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(LocationName) || !ctx.Read(Category) || !ctx.Read(Currency) || !ctx.Read(Balance) || !ctx.Read(Cash) || !ctx.Read(Bank))
			return false;
		if (!ctx.Read(Dealer) || !ctx.Read(Service) || !ctx.Read(Garage))
			return false;
		if (!ctx.Read(OfferClasses) || !ctx.Read(OfferPrices))
			return false;
		if (!ctx.Read(HasCar) || !ctx.Read(CarClass) || !ctx.Read(HealthPct) || !ctx.Read(FuelPct))
			return false;
		if (!ctx.Read(RepairPrice) || !ctx.Read(RefuelPrice) || !ctx.Read(PartPrice))
			return false;
		if (!ctx.Read(StoreFee) || !ctx.Read(GarageMax) || !ctx.Read(Stored))
			return false;
		if (!ctx.Read(OwnerState) || !ctx.Read(ClaimPrice))
			return false;
		if (!ctx.Read(StoredGroup))
			return false;
		return true;
	}
}

class OS_VehicleMenu : OS_MenuBase
{
	protected ref OS_VehicleMenuData m_Data;

	protected TextWidget m_Title;
	protected TextWidget m_Balance;
	protected TextListboxWidget m_OfferList;
	protected TextListboxWidget m_GarageList;
	protected TextWidget m_CarName;
	protected TextWidget m_CarHealth;
	protected TextWidget m_CarFuel;
	protected TextWidget m_CarOwner;
	protected ButtonWidget m_BtnClaim;
	protected ButtonWidget m_BtnBuy;
	protected ButtonWidget m_BtnRepair;
	protected ButtonWidget m_BtnRefuel;
	protected ButtonWidget m_BtnParts;
	protected ButtonWidget m_BtnStore;
	protected ButtonWidget m_BtnRetrieve;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "vehicle_menu.layout";
	}

	void SetData(OS_VehicleMenuData data)
	{
		m_Data = data;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_Title = FindText("Title");
		m_Balance = FindText("Balance");
		m_OfferList = FindList("OfferList");
		m_GarageList = FindList("GarageList");
		m_CarName = FindText("CarName");
		m_CarHealth = FindText("CarHealth");
		m_CarFuel = FindText("CarFuel");
		m_CarOwner = FindText("CarOwner");
		m_BtnClaim = FindButton("BtnClaim");
		m_BtnBuy = FindButton("BtnBuy");
		m_BtnRepair = FindButton("BtnRepair");
		m_BtnRefuel = FindButton("BtnRefuel");
		m_BtnParts = FindButton("BtnParts");
		m_BtnStore = FindButton("BtnStore");
		m_BtnRetrieve = FindButton("BtnRetrieve");
		m_BtnClose = FindButton("BtnClose");

		Refresh();
	}

	protected void ShowWidget(string name, bool visible)
	{
		Widget w = layoutRoot.FindAnyWidget(name);
		if (w)
			w.Show(visible);
	}

	protected void Refresh()
	{
		if (!m_Data)
			return;

		string cur = " " + m_Data.Currency;

		if (m_Title)
			m_Title.SetText(m_Data.LocationName);
		if (m_Balance)
			m_Balance.SetText("Hotovost: " + m_Data.Cash.ToString() + "  Ucet: " + m_Data.Bank.ToString() + cur);

		// sekce podle toho, co místo nabízí
		ShowWidget("DealerLabel", m_Data.Dealer);
		TextWidget dealerLabel = FindText("DealerLabel");
		if (dealerLabel)
		{
			if (m_Data.Category == "heli")
				dealerLabel.SetText("Helikoptery");
			else if (m_Data.Category == "boat")
				dealerLabel.SetText("Lode");
			else if (m_Data.Category == "bike")
				dealerLabel.SetText("Motorky");
			else if (m_Data.Category == "mixed")
				dealerLabel.SetText("Prodej vozidel");
			else
				dealerLabel.SetText("Autobazar");
		}
		if (m_BtnBuy)
		{
			if (m_Data.Category == "heli")
				m_BtnBuy.SetText("Koupit helikopteru");
			else if (m_Data.Category == "boat")
				m_BtnBuy.SetText("Koupit lod");
			else if (m_Data.Category == "bike")
				m_BtnBuy.SetText("Koupit motorku");
			else if (m_Data.Category == "mixed")
				m_BtnBuy.SetText("Koupit vozidlo");
			else
				m_BtnBuy.SetText("Koupit auto");
		}
		ShowWidget("OfferListBg", m_Data.Dealer);
		ShowWidget("OfferList", m_Data.Dealer);
		ShowWidget("BtnBuy", m_Data.Dealer);

		ShowWidget("ServiceLabel", m_Data.Service || m_Data.Garage);
		ShowWidget("CarName", m_Data.Service || m_Data.Garage);
		ShowWidget("CarHealth", m_Data.Service || m_Data.Garage);
		ShowWidget("CarFuel", m_Data.Service || m_Data.Garage);
		ShowWidget("CarOwner", m_Data.Service || m_Data.Garage);
		bool hasCar = m_Data.HasCar;
		ShowWidget("BtnClaim", m_Data.Service && hasCar && m_Data.OwnerState == 0);
		ShowWidget("BtnRepair", m_Data.Service);
		ShowWidget("BtnRefuel", m_Data.Service);
		ShowWidget("BtnParts", m_Data.Service);
		ShowWidget("BtnStore", m_Data.Garage);

		ShowWidget("GarageLabel", m_Data.Garage);
		ShowWidget("GarageListBg", m_Data.Garage);
		ShowWidget("GarageList", m_Data.Garage);
		ShowWidget("BtnRetrieve", m_Data.Garage);

		if (m_OfferList)
		{
			int offerRow = m_OfferList.GetSelectedRow();
			m_OfferList.ClearItems();
			for (int i = 0; i < m_Data.OfferClasses.Count(); i++)
			{
				m_OfferList.AddItem(OS_Client.DisplayName(m_Data.OfferClasses[i]) + "   " + m_Data.OfferPrices[i].ToString() + cur, null, 0);
			}
			if (offerRow >= 0 && offerRow < m_Data.OfferClasses.Count())
				m_OfferList.SelectRow(offerRow);
		}

		if (m_GarageList)
		{
			m_GarageList.ClearItems();
			for (int g = 0; g < m_Data.Stored.Count(); g++)
			{
				string storedLabel = OS_Client.DisplayName(m_Data.Stored[g]);
				if (g < m_Data.StoredGroup.Count() && m_Data.StoredGroup[g] == 1)
					storedLabel = storedLabel + "  (skupina)";
				m_GarageList.AddItem(storedLabel, null, 0);
			}
		}

		if (m_Data.HasCar)
		{
			SetLabel(m_CarName, "Auto: " + OS_Client.DisplayName(m_Data.CarClass));
			SetLabel(m_CarHealth, "Stav: " + m_Data.HealthPct.ToString() + " %");
			SetLabel(m_CarFuel, "Palivo: " + m_Data.FuelPct.ToString() + " %");
			if (m_Data.OwnerState == 0)
				SetLabel(m_CarOwner, "Majitel: nikdo");
			else if (m_Data.OwnerState == 1)
				SetLabel(m_CarOwner, "Majitel: ty");
			else if (m_Data.OwnerState == 3)
				SetLabel(m_CarOwner, "Majitel: tvoje skupina");
			else
				SetLabel(m_CarOwner, "Majitel: jiny hrac");
		}
		else
		{
			SetLabel(m_CarName, "Zadne auto pobliz - prijed s nim blize");
			SetLabel(m_CarHealth, "");
			SetLabel(m_CarFuel, "");
			SetLabel(m_CarOwner, "");
		}

		if (m_BtnRepair)
			m_BtnRepair.SetText("Opravit (" + m_Data.RepairPrice.ToString() + cur + ")");
		if (m_BtnRefuel)
			m_BtnRefuel.SetText("Natankovat (" + m_Data.RefuelPrice.ToString() + cur + ")");
		if (m_BtnParts)
			m_BtnParts.SetText("Doplnit chybejici dily (" + m_Data.PartPrice.ToString() + cur + "/ks)");
		if (m_BtnClaim)
			m_BtnClaim.SetText("Prepsat auto na sebe (" + m_Data.ClaimPrice.ToString() + cur + ")");
		if (m_BtnStore)
			m_BtnStore.SetText("Ulozit do garaze (" + m_Data.StoreFee.ToString() + cur + ")");
		if (m_BtnRetrieve)
			m_BtnRetrieve.SetText("Vyzvednout (aut v garazi: " + m_Data.Stored.Count().ToString() + ")");
	}

	protected void SetLabel(TextWidget w, string text)
	{
		if (w)
			w.SetText(text);
	}

	protected void SendAction(int action, int arg)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(arg);
		rpc.Send(null, OS_RPC_VEHICLE_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Data)
			return super.OnClick(w, x, y, button);

		if (w == m_BtnBuy && m_OfferList)
		{
			int offer = m_OfferList.GetSelectedRow();
			if (offer < 0)
				SetStatus("Vyber auto.");
			else if (m_Data.OfferPrices[offer] > m_Data.Balance)
				SetStatus("Nemas dost penez.");
			else
				SendAction(OS_VehicleAction.BUY, offer);
			return true;
		}
		if (w == m_BtnRepair)
		{
			SendAction(OS_VehicleAction.REPAIR, 0);
			return true;
		}
		if (w == m_BtnRefuel)
		{
			SendAction(OS_VehicleAction.REFUEL, 0);
			return true;
		}
		if (w == m_BtnParts)
		{
			SendAction(OS_VehicleAction.PARTS, 0);
			return true;
		}
		if (w == m_BtnClaim)
		{
			SendAction(OS_VehicleAction.CLAIM, 0);
			return true;
		}
		if (w == m_BtnStore)
		{
			SendAction(OS_VehicleAction.STORE, 0);
			return true;
		}
		if (w == m_BtnRetrieve && m_GarageList)
		{
			int stored = m_GarageList.GetSelectedRow();
			if (stored < 0)
				SetStatus("Vyber auto v garazi.");
			else
				SendAction(OS_VehicleAction.RETRIEVE, stored);
			return true;
		}
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}

		return super.OnClick(w, x, y, button);
	}
}

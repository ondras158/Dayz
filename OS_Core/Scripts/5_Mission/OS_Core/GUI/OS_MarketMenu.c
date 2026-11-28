// Bazar hráčů - pořadí čtení odpovídá OS_ServerMarket.SendData

class OS_MarketMenu : OS_MenuBase
{
	protected string m_Currency;
	protected float m_FeePercent;
	protected ref array<int> m_Ids = new array<int>;
	protected ref array<string> m_Classes = new array<string>;
	protected ref array<int> m_Prices = new array<int>;
	protected ref array<string> m_Sellers = new array<string>;
	protected ref array<int> m_Vehicles = new array<int>;
	protected ref array<int> m_Mine = new array<int>;
	protected ref array<string> m_ItemClasses = new array<string>;
	protected ref array<int> m_ItemLow = new array<int>;
	protected ref array<int> m_ItemHigh = new array<int>;
	protected ref array<string> m_Garage = new array<string>;
	protected int m_Returns;

	// co je v seznamech (po filtru)
	protected ref array<int> m_ShownOffers = new array<int>;     // index do m_Ids
	protected ref array<int> m_SellKinds = new array<int>;       // 0 = věc, 1 = auto
	protected ref array<int> m_SellIndexes = new array<int>;

	protected TextListboxWidget m_OfferList;
	protected TextListboxWidget m_SellList;
	protected EditBoxWidget m_EditSearch;
	protected EditBoxWidget m_EditPrice;

	override string LayoutName()
	{
		return "market_menu.layout";
	}

	bool ReadData(ParamsReadContext ctx)
	{
		if (!ctx.Read(m_Currency) || !ctx.Read(m_FeePercent))
			return false;
		if (!ctx.Read(m_Ids) || !ctx.Read(m_Classes) || !ctx.Read(m_Prices) || !ctx.Read(m_Sellers) || !ctx.Read(m_Vehicles) || !ctx.Read(m_Mine))
			return false;
		if (!ctx.Read(m_ItemClasses) || !ctx.Read(m_ItemLow) || !ctx.Read(m_ItemHigh) || !ctx.Read(m_Garage) || !ctx.Read(m_Returns))
			return false;
		if (layoutRoot)
			Refresh();
		return true;
	}

	override void OnCreated()
	{
		m_OfferList = FindList("OfferList");
		m_SellList = FindList("SellList");
		m_EditSearch = FindEdit("EditSearch");
		m_EditPrice = FindEdit("EditPrice");
		Refresh();
	}

	protected string Label(string className, bool vehicle)
	{
		string name = OS_Client.DisplayName(className);
		if (vehicle)
			name = OS_F("%1 [auto]", name);
		return name;
	}

	protected void Refresh()
	{
		if (!m_OfferList || !m_SellList)
			return;

		string query = "";
		if (m_EditSearch)
		{
			query = m_EditSearch.GetText();
			query.TrimInPlace();
			query.ToLower();
		}

		m_OfferList.ClearItems();
		m_ShownOffers.Clear();
		for (int i = 0; i < m_Ids.Count(); i++)
		{
			string name = Label(m_Classes[i], m_Vehicles[i] == 1);
			if (query != "")
			{
				string lower = name + " " + m_Classes[i];
				lower.ToLower();
				if (!lower.Contains(query))
					continue;
			}
			string line = OS_F("%1   %2 %3   (%4)", name, m_Prices[i].ToString(), m_Currency, m_Sellers[i]);
			if (m_Mine[i] == 1)
				line = OS_F("%1   [moje]", line);
			m_OfferList.AddItem(line, null, 0);
			m_ShownOffers.Insert(i);
		}

		m_SellList.ClearItems();
		m_SellKinds.Clear();
		m_SellIndexes.Clear();
		for (int g = 0; g < m_Garage.Count(); g++)
		{
			m_SellList.AddItem(OS_F("%1 (v garazi)", Label(m_Garage[g], true)), null, 0);
			m_SellKinds.Insert(1);
			m_SellIndexes.Insert(g);
		}
		for (int j = 0; j < m_ItemClasses.Count(); j++)
		{
			m_SellList.AddItem(OS_Client.DisplayName(m_ItemClasses[j]), null, 0);
			m_SellKinds.Insert(0);
			m_SellIndexes.Insert(j);
		}

		TextWidget fee = FindText("FeeText");
		if (fee)
		{
			int feePct = Math.Round(m_FeePercent);
			fee.SetText(OS_F("Poplatek z prodeje %1 %, penize prijdou na ucet.", feePct.ToString()));
		}
		ButtonWidget claim = FindButton("BtnClaim");
		if (claim)
		{
			claim.Show(m_Returns > 0);
			claim.SetText(OS_F("Vyzvednout vracene veci (%1)", m_Returns.ToString()));
		}
	}

	protected void Send(int action, int a, int b, int price)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(a);
		rpc.Write(b);
		rpc.Write(price);
		rpc.Send(null, OS_RPC_MARKET_ACTION, true, null);
	}

	protected int SelectedOffer()
	{
		if (!m_OfferList)
			return -1;
		int row = m_OfferList.GetSelectedRow();
		if (row < 0 || row >= m_ShownOffers.Count())
			return -1;
		return m_ShownOffers[row];
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string name = w.GetName();
		if (name == "BtnSearch")
		{
			Refresh();
			return true;
		}
		if (name == "BtnBuy" || name == "BtnWithdraw")
		{
			int offer = SelectedOffer();
			if (offer < 0)
			{
				SetStatus(OS_T("Vyber nabidku vlevo."));
				return true;
			}
			if (name == "BtnBuy")
				Send(OS_MarketAction.BUY, m_Ids[offer], 0, 0);
			else if (m_Mine[offer] == 1)
				Send(OS_MarketAction.WITHDRAW, m_Ids[offer], 0, 0);
			else
				SetStatus(OS_T("Stahnout jde jen tvoje nabidka."));
			return true;
		}
		if (name == "BtnList")
		{
			int row = -1;
			if (m_SellList)
				row = m_SellList.GetSelectedRow();
			if (row < 0 || row >= m_SellKinds.Count())
			{
				SetStatus(OS_T("Vyber vec nebo auto vpravo."));
				return true;
			}
			int price = 0;
			if (m_EditPrice)
			{
				string text = m_EditPrice.GetText();
				text.TrimInPlace();
				price = text.ToInt();
			}
			if (price <= 0)
			{
				SetStatus(OS_T("Zadej cenu."));
				return true;
			}
			int index = m_SellIndexes[row];
			if (m_SellKinds[row] == 1)
				Send(OS_MarketAction.LIST_VEHICLE, index, 0, price);
			else
				Send(OS_MarketAction.LIST_ITEM, m_ItemLow[index], m_ItemHigh[index], price);
			return true;
		}
		if (name == "BtnClaim")
		{
			Send(OS_MarketAction.CLAIM, 0, 0, 0);
			return true;
		}
		if (name == "BtnClose")
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}
}

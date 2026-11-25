class OS_TraderMenu : OS_MenuBase
{
	protected string m_TraderName;
	protected string m_Currency;
	protected bool m_IsAdmin;
	protected ref array<string> m_Categories = new array<string>;
	protected ref array<int> m_CategoryIndexes = new array<int>;   // řádek seznamu -> skutečná kategorie na serveru
	protected float m_BuyMult = 1;    // násobitele cen obchodníka (katalog má základní ceny)
	protected float m_SellMult = 1;
	protected bool m_CanBuy = true;    // obchodník prodává
	protected bool m_CanSell = true;   // obchodník vykupuje
	protected ref array<int> m_CategoryCounts = new array<int>;

	// aktuálně zobrazené předměty (kategorie nebo hledání)
	protected int m_ViewCategory = -1;
	protected ref array<int> m_ItemCat = new array<int>;
	protected ref array<int> m_ItemIndex = new array<int>;
	protected ref array<string> m_ItemClass = new array<string>;
	protected ref array<int> m_ItemBuy = new array<int>;
	protected ref array<int> m_ItemSell = new array<int>;

	protected int m_Balance;   // kolik jde utratit
	protected int m_Cash;
	protected int m_Bank;
	protected ref array<string> m_SellClasses = new array<string>;
	protected ref array<int> m_SellCounts = new array<int>;
	protected ref array<int> m_SellPrices = new array<int>;

	protected int m_RequestedCategory = -2;
	protected int m_LastItemRow = -2;
	protected string m_LastSearch;

	protected TextWidget m_Title;
	protected TextWidget m_BalanceText;
	protected TextListboxWidget m_CategoryList;
	protected TextListboxWidget m_ItemList;
	protected TextListboxWidget m_SellList;
	protected EditBoxWidget m_EditSearch;
	protected EditBoxWidget m_EditBuy;
	protected EditBoxWidget m_EditSell;
	protected EditBoxWidget m_EditFactor;
	protected ButtonWidget m_BtnBuy;
	protected ButtonWidget m_BtnSell;
	protected ButtonWidget m_BtnSearch;
	protected ButtonWidget m_BtnClose;
	protected ButtonWidget m_BtnSavePrice;
	protected ButtonWidget m_BtnBulk;

	override string LayoutName()
	{
		return "trader_menu.layout";
	}

	// ------------------------------------------------------------ data ze serveru

	void SetHeader(string traderName, string currency, bool isAdmin, array<string> categories, array<int> counts, array<int> categoryIndexes, float buyMult, float sellMult, bool canBuy, bool canSell)
	{
		m_CanBuy = canBuy;
		m_CanSell = canSell;
		m_BuyMult = buyMult;
		m_SellMult = sellMult;
		m_CategoryIndexes.Clear();
		m_CategoryIndexes.Copy(categoryIndexes);
		m_TraderName = traderName;
		m_Currency = currency;
		m_IsAdmin = isAdmin;
		m_Categories.Clear();
		m_Categories.Copy(categories);
		m_CategoryCounts.Clear();
		m_CategoryCounts.Copy(counts);
		m_RequestedCategory = -2;

		if (layoutRoot)
			RefreshHeader();
	}

	void SetItems(int viewCategory, array<int> cats, array<int> indexes, array<string> classes, array<int> buys, array<int> sells)
	{
		m_ViewCategory = viewCategory;
		m_ItemCat.Clear();
		m_ItemCat.Copy(cats);
		m_ItemIndex.Clear();
		m_ItemIndex.Copy(indexes);
		m_ItemClass.Clear();
		m_ItemClass.Copy(classes);
		m_ItemBuy.Clear();
		m_ItemBuy.Copy(buys);
		m_ItemSell.Clear();
		m_ItemSell.Copy(sells);

		if (layoutRoot)
			RefreshItems();
	}

	void SetPlayerState(int balance, int cash, int bank, array<string> classes, array<int> counts, array<int> prices)
	{
		m_Balance = balance;
		m_Cash = cash;
		m_Bank = bank;
		m_SellClasses.Clear();
		m_SellClasses.Copy(classes);
		m_SellCounts.Clear();
		m_SellCounts.Copy(counts);
		m_SellPrices.Clear();
		m_SellPrices.Copy(prices);

		if (layoutRoot)
			RefreshPlayer();
	}

	// ------------------------------------------------------------ zobrazení

	override void OnCreated()
	{
		m_Title = FindText("Title");
		m_BalanceText = FindText("Balance");
		m_CategoryList = FindList("CategoryList");
		m_ItemList = FindList("ItemList");
		m_SellList = FindList("SellList");
		m_EditSearch = FindEdit("EditSearch");
		m_EditBuy = FindEdit("EditBuy");
		m_EditSell = FindEdit("EditSell");
		m_EditFactor = FindEdit("EditFactor");
		m_BtnBuy = FindButton("BtnBuy");
		m_BtnSell = FindButton("BtnSell");
		m_BtnSearch = FindButton("BtnSearch");
		m_BtnClose = FindButton("BtnClose");
		m_BtnSavePrice = FindButton("BtnSavePrice");
		m_BtnBulk = FindButton("BtnBulk");

		RefreshHeader();
		RefreshItems();
		RefreshPlayer();
	}

	protected void RefreshHeader()
	{
		if (m_Title)
		{
			string title = OS_F("Obchod - %1", OS_T(m_TraderName));
			if (m_BuyMult != 1 || m_SellMult != 1)
			{
				int buyPct = Math.Round(m_BuyMult * 100);
				int sellPct = Math.Round(m_SellMult * 100);
				title = OS_F("%1  (nakup %2 %, vykup %3 %)", title, buyPct.ToString(), sellPct.ToString());
			}
			m_Title.SetText(title);
		}

		// obchodník, který jen vykupuje (nebo jen prodává)
		array<string> buyWidgets = {"CatLabel", "CategoryListBg", "CategoryList", "EditSearchBg", "EditSearch", "BtnSearch", "ItemListBg", "ItemList", "BtnBuy"};
		foreach (string buyWidget : buyWidgets)
		{
			Widget bw = layoutRoot.FindAnyWidget(buyWidget);
			if (bw)
				bw.Show(m_CanBuy);
		}
		array<string> sellWidgets = {"SellLabel", "SellListBg", "SellList", "BtnSell"};
		foreach (string sellWidget : sellWidgets)
		{
			Widget sw = layoutRoot.FindAnyWidget(sellWidget);
			if (sw)
				sw.Show(m_CanSell);
		}
		if (!m_CanBuy)
			SetStatus(OS_T("Tento obchodnik jen vykupuje - vpravo vyber, co chces prodat."));

		// admin panel vidí jen admin
		array<string> adminWidgets = {"AdminLabel", "BuyLabel", "EditBuyBg", "EditBuy", "SellPriceLabel", "EditSellBg", "EditSell", "BtnSavePrice", "FactorLabel", "EditFactorBg", "EditFactor", "BtnBulk"};
		foreach (string widgetName : adminWidgets)
		{
			Widget w = layoutRoot.FindAnyWidget(widgetName);
			if (w)
				w.Show(m_IsAdmin);
		}

		if (!m_CategoryList)
			return;

		int previous = m_CategoryList.GetSelectedRow();
		m_CategoryList.ClearItems();
		for (int i = 0; i < m_Categories.Count(); i++)
		{
			m_CategoryList.AddItem(OS_T(m_Categories[i]) + " (" + m_CategoryCounts[i].ToString() + ")", null, 0);
		}

		if (previous < 0 || previous >= m_Categories.Count())
			previous = 0;
		if (m_Categories.Count() > 0)
			m_CategoryList.SelectRow(previous);
	}

	protected int EffectiveBuy(int row)
	{
		return Effective(m_ItemBuy[row], m_BuyMult);
	}

	protected int Effective(int basePrice, float mult)
	{
		if (basePrice < 0)
			return basePrice;
		return Math.Round(basePrice * mult);
	}

	protected string PriceText(int price)
	{
		if (price < 0)
			return "-";
		return price.ToString();
	}

	protected void RefreshItems()
	{
		if (!m_ItemList)
			return;

		m_ItemList.ClearItems();
		for (int i = 0; i < m_ItemClass.Count(); i++)
		{
			string label = OS_F("%1   [koupit %2 / vykup %3]", OS_Client.DisplayName(m_ItemClass[i]), PriceText(Effective(m_ItemBuy[i], m_BuyMult)), PriceText(Effective(m_ItemSell[i], m_SellMult)));
			if (m_IsAdmin)
			{
				label = label + "  " + m_ItemClass[i];
				if (m_ItemBuy[i] < 0 && m_ItemSell[i] < 0)
					label = OS_F("%1  (SKRYTO)", label);
			}
			m_ItemList.AddItem(label, null, 0);
		}
		m_LastItemRow = -2;

		if (m_ViewCategory < 0)
			SetStatus(OS_F("Nalezeno: %1", m_ItemClass.Count().ToString()));
	}

	protected void RefreshPlayer()
	{
		if (m_BalanceText)
			m_BalanceText.SetText(OS_F("Hotovost: %1  Ucet: %2 %3", m_Cash.ToString(), m_Bank.ToString(), m_Currency));

		if (!m_SellList)
			return;

		m_SellList.ClearItems();
		for (int i = 0; i < m_SellClasses.Count(); i++)
		{
			string label = OS_Client.DisplayName(m_SellClasses[i]) + " x" + m_SellCounts[i].ToString();
			if (i < m_SellPrices.Count() && m_SellPrices[i] >= 0)
				label = OS_F("%1   (az %2)", label, m_SellPrices[i].ToString());
			m_SellList.AddItem(label, null, 0);
		}
	}

	// ------------------------------------------------------------ ovládání

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		// výběr kategorie -> požádat server o její předměty
		if (m_CategoryList)
		{
			int selected = m_CategoryList.GetSelectedRow();
			if (selected >= 0 && selected != m_RequestedCategory && selected < m_CategoryIndexes.Count())
			{
				m_RequestedCategory = selected;
				RequestCategory(m_CategoryIndexes[selected]);
			}
		}

		// admin: vybraný předmět -> předvyplnit ceny
		if (m_IsAdmin && m_ItemList)
		{
			int row = m_ItemList.GetSelectedRow();
			if (row != m_LastItemRow)
			{
				m_LastItemRow = row;
				if (row >= 0 && row < m_ItemClass.Count())
				{
					if (m_EditBuy)
						m_EditBuy.SetText(m_ItemBuy[row].ToString());
					if (m_EditSell)
						m_EditSell.SetText(m_ItemSell[row].ToString());
				}
			}
		}
	}

	protected void RequestCategory(int categoryIndex)
	{
		m_LastSearch = "";
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(categoryIndex);
		rpc.Send(null, OS_RPC_TRADER_CATEGORY, true, null);
	}

	protected void RequestSearch(string text)
	{
		m_LastSearch = text;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(text);
		rpc.Send(null, OS_RPC_TRADER_SEARCH, true, null);
	}

	//! po admin úpravě znovu načíst aktuální pohled
	protected void ReloadView()
	{
		if (m_ViewCategory >= 0)
			RequestCategory(m_ViewCategory);
		else
			RequestSearch(m_LastSearch);
	}

	protected int SelectedItemRow()
	{
		if (!m_ItemList)
			return -1;
		int row = m_ItemList.GetSelectedRow();
		if (row < 0 || row >= m_ItemClass.Count())
			return -1;
		return row;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnBuy)
		{
			Buy();
			return true;
		}
		if (w == m_BtnSell)
		{
			Sell();
			return true;
		}
		if (w == m_BtnSearch && m_EditSearch)
		{
			string text = m_EditSearch.GetText();
			string trimmed = text.Trim();
			if (trimmed.Length() < 2)
			{
				SetStatus(OS_T("Zadej aspon 2 znaky."));
			}
			else
			{
				// zrušit výběr kategorie, aby šlo kliknutím zase vybrat kategorii
				if (m_CategoryList)
					m_CategoryList.SelectRow(-1);
				m_RequestedCategory = -1;
				RequestSearch(trimmed);
			}
			return true;
		}
		if (w == m_BtnSavePrice)
		{
			AdminSavePrice();
			return true;
		}
		if (w == m_BtnBulk)
		{
			AdminBulk();
			return true;
		}
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	protected void Buy()
	{
		int row = SelectedItemRow();
		if (row < 0)
		{
			SetStatus(OS_T("Vyber predmet, ktery chces koupit."));
			return;
		}
		if (m_ItemBuy[row] < 0)
		{
			SetStatus(OS_T("Tento predmet se neprodava."));
			return;
		}
		if (EffectiveBuy(row) > m_Balance)
		{
			SetStatus(OS_T("Nemas dost penez."));
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ItemCat[row]);
		rpc.Write(m_ItemIndex[row]);
		rpc.Send(null, OS_RPC_TRADER_BUY, true, null);
		SetStatus(OS_F("Koupeno: %1", OS_Client.DisplayName(m_ItemClass[row])));
		OS_ClientAnim.Queue(EmoteConstants.ID_EMOTE_NOD);
	}

	protected void Sell()
	{
		if (!m_SellList)
			return;

		int row = m_SellList.GetSelectedRow();
		if (row < 0 || row >= m_SellClasses.Count())
		{
			SetStatus(OS_T("Vyber vec, kterou chces prodat."));
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_SellClasses[row]);
		rpc.Send(null, OS_RPC_TRADER_SELL, true, null);
		SetStatus(OS_F("Prodano: %1", OS_Client.DisplayName(m_SellClasses[row])));
	}

	protected void AdminSavePrice()
	{
		int row = SelectedItemRow();
		if (row < 0 || !m_EditBuy || !m_EditSell)
		{
			SetStatus(OS_T("Vyber predmet, kteremu chces zmenit cenu."));
			return;
		}

		string buyText = m_EditBuy.GetText();
		string sellText = m_EditSell.GetText();
		buyText.TrimInPlace();
		sellText.TrimInPlace();
		int buy = buyText.ToInt();
		int sell = sellText.ToInt();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ItemCat[row]);
		rpc.Write(m_ItemIndex[row]);
		rpc.Write(buy);
		rpc.Write(sell);
		rpc.Send(null, OS_RPC_TRADER_ADMIN_EDIT, true, null);

		ReloadView();
	}

	protected void AdminBulk()
	{
		if (m_ViewCategory < 0 || !m_EditFactor)
		{
			SetStatus(OS_T("Nasobit jde jen celou kategorii - vyber ji vlevo."));
			return;
		}

		string factorText = m_EditFactor.GetText();
		factorText.Replace(",", ".");
		factorText.TrimInPlace();
		float factor = factorText.ToFloat();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ViewCategory);
		rpc.Write(factor);
		rpc.Send(null, OS_RPC_TRADER_ADMIN_BULK, true, null);

		ReloadView();
	}
}

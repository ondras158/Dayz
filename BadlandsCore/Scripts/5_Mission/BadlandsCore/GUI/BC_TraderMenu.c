class BC_TraderMenu : BC_MenuBase
{
	protected string m_TraderName;
	protected string m_Currency;
	protected bool m_IsAdmin;
	protected ref array<string> m_Categories = new array<string>;
	protected ref array<int> m_CategoryCounts = new array<int>;

	// aktuálně zobrazené předměty (kategorie nebo hledání)
	protected int m_ViewCategory = -1;
	protected ref array<int> m_ItemCat = new array<int>;
	protected ref array<int> m_ItemIndex = new array<int>;
	protected ref array<string> m_ItemClass = new array<string>;
	protected ref array<int> m_ItemBuy = new array<int>;
	protected ref array<int> m_ItemSell = new array<int>;

	protected int m_Balance;
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

	void SetHeader(string traderName, string currency, bool isAdmin, array<string> categories, array<int> counts)
	{
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

	void SetPlayerState(int balance, array<string> classes, array<int> counts, array<int> prices)
	{
		m_Balance = balance;
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
			m_Title.SetText("Obchod - " + m_TraderName);

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
			m_CategoryList.AddItem(m_Categories[i] + " (" + m_CategoryCounts[i].ToString() + ")", null, 0);
		}

		if (previous < 0 || previous >= m_Categories.Count())
			previous = 0;
		if (m_Categories.Count() > 0)
			m_CategoryList.SelectRow(previous);
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
			string label = BC_Client.DisplayName(m_ItemClass[i]) + "   [koupit " + PriceText(m_ItemBuy[i]) + " / vykup " + PriceText(m_ItemSell[i]) + "]";
			if (m_IsAdmin)
			{
				label = label + "  " + m_ItemClass[i];
				if (m_ItemBuy[i] < 0 && m_ItemSell[i] < 0)
					label = label + "  (SKRYTO)";
			}
			m_ItemList.AddItem(label, null, 0);
		}
		m_LastItemRow = -2;

		if (m_ViewCategory < 0)
			SetStatus("Nalezeno: " + m_ItemClass.Count().ToString());
	}

	protected void RefreshPlayer()
	{
		if (m_BalanceText)
			m_BalanceText.SetText("Penize: " + m_Balance.ToString() + " " + m_Currency);

		if (!m_SellList)
			return;

		m_SellList.ClearItems();
		for (int i = 0; i < m_SellClasses.Count(); i++)
		{
			string label = BC_Client.DisplayName(m_SellClasses[i]) + " x" + m_SellCounts[i].ToString();
			if (i < m_SellPrices.Count() && m_SellPrices[i] >= 0)
				label = label + "   (az " + m_SellPrices[i].ToString() + ")";
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
			if (selected >= 0 && selected != m_RequestedCategory)
			{
				m_RequestedCategory = selected;
				RequestCategory(selected);
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
		rpc.Send(null, BC_RPC_TRADER_CATEGORY, true, null);
	}

	protected void RequestSearch(string text)
	{
		m_LastSearch = text;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(text);
		rpc.Send(null, BC_RPC_TRADER_SEARCH, true, null);
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
				SetStatus("Zadej aspon 2 znaky.");
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
			SetStatus("Vyber predmet, ktery chces koupit.");
			return;
		}
		if (m_ItemBuy[row] < 0)
		{
			SetStatus("Tento predmet se neprodava.");
			return;
		}
		if (m_ItemBuy[row] > m_Balance)
		{
			SetStatus("Nemas dost penez.");
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ItemCat[row]);
		rpc.Write(m_ItemIndex[row]);
		rpc.Send(null, BC_RPC_TRADER_BUY, true, null);
		SetStatus("Koupeno: " + BC_Client.DisplayName(m_ItemClass[row]));
	}

	protected void Sell()
	{
		if (!m_SellList)
			return;

		int row = m_SellList.GetSelectedRow();
		if (row < 0 || row >= m_SellClasses.Count())
		{
			SetStatus("Vyber vec, kterou chces prodat.");
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_SellClasses[row]);
		rpc.Send(null, BC_RPC_TRADER_SELL, true, null);
		SetStatus("Prodano: " + BC_Client.DisplayName(m_SellClasses[row]));
	}

	protected void AdminSavePrice()
	{
		int row = SelectedItemRow();
		if (row < 0 || !m_EditBuy || !m_EditSell)
		{
			SetStatus("Vyber predmet, kteremu chces zmenit cenu.");
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
		rpc.Send(null, BC_RPC_TRADER_ADMIN_EDIT, true, null);

		ReloadView();
	}

	protected void AdminBulk()
	{
		if (m_ViewCategory < 0 || !m_EditFactor)
		{
			SetStatus("Nasobit jde jen celou kategorii - vyber ji vlevo.");
			return;
		}

		string factorText = m_EditFactor.GetText();
		factorText.Replace(",", ".");
		factorText.TrimInPlace();
		float factor = factorText.ToFloat();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ViewCategory);
		rpc.Write(factor);
		rpc.Send(null, BC_RPC_TRADER_ADMIN_BULK, true, null);

		ReloadView();
	}
}

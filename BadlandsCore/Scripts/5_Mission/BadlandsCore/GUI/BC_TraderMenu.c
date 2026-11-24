class BC_TraderMenu : BC_MenuBase
{
	protected string m_TraderName;
	protected string m_Currency;
	protected ref array<string> m_Categories = new array<string>;
	protected ref array<int> m_ItemCategory = new array<int>;
	protected ref array<int> m_ItemIndexInCategory = new array<int>;
	protected ref array<string> m_ItemClass = new array<string>;
	protected ref array<string> m_ItemName = new array<string>;
	protected ref array<int> m_ItemBuy = new array<int>;
	protected ref array<int> m_ItemSell = new array<int>;
	protected ref array<int> m_VisibleItems = new array<int>;   // řádek seznamu -> index předmětu

	protected int m_Balance;
	protected ref array<string> m_SellClasses = new array<string>;
	protected ref array<int> m_SellCounts = new array<int>;

	protected int m_ShownCategory = -2;

	protected TextWidget m_Title;
	protected TextWidget m_BalanceText;
	protected TextListboxWidget m_CategoryList;
	protected TextListboxWidget m_ItemList;
	protected TextListboxWidget m_SellList;
	protected ButtonWidget m_BtnBuy;
	protected ButtonWidget m_BtnSell;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "trader_menu.layout";
	}

	void SetCatalog(string traderName, string currency, array<string> categories, array<int> itemCategories, array<string> itemClasses, array<int> itemBuy, array<int> itemSell)
	{
		m_TraderName = traderName;
		m_Currency = currency;
		m_Categories.Clear();
		m_Categories.Copy(categories);
		m_ItemCategory.Clear();
		m_ItemCategory.Copy(itemCategories);
		m_ItemClass.Clear();
		m_ItemClass.Copy(itemClasses);
		m_ItemBuy.Clear();
		m_ItemBuy.Copy(itemBuy);
		m_ItemSell.Clear();
		m_ItemSell.Copy(itemSell);

		// pořadí předmětu v rámci kategorie (to server očekává při nákupu) + čitelné názvy
		m_ItemIndexInCategory.Clear();
		m_ItemName.Clear();
		map<int, int> perCategory = new map<int, int>;
		for (int i = 0; i < m_ItemClass.Count(); i++)
		{
			int cat = m_ItemCategory[i];
			int n = 0;
			perCategory.Find(cat, n);
			m_ItemIndexInCategory.Insert(n);
			perCategory.Set(cat, n + 1);
			m_ItemName.Insert(BC_Client.DisplayName(m_ItemClass[i]));
		}

		m_ShownCategory = -2;
		if (layoutRoot)
			RefreshCatalog();
	}

	void SetPlayerState(int balance, array<string> classes, array<int> counts)
	{
		m_Balance = balance;
		m_SellClasses.Clear();
		m_SellClasses.Copy(classes);
		m_SellCounts.Clear();
		m_SellCounts.Copy(counts);

		if (layoutRoot)
			RefreshPlayer();
	}

	override void OnCreated()
	{
		m_Title = FindText("Title");
		m_BalanceText = FindText("Balance");
		m_CategoryList = FindList("CategoryList");
		m_ItemList = FindList("ItemList");
		m_SellList = FindList("SellList");
		m_BtnBuy = FindButton("BtnBuy");
		m_BtnSell = FindButton("BtnSell");
		m_BtnClose = FindButton("BtnClose");

		RefreshCatalog();
		RefreshPlayer();
	}

	protected void RefreshCatalog()
	{
		if (m_Title)
			m_Title.SetText("Obchod - " + m_TraderName);

		if (!m_CategoryList)
			return;

		int previous = m_CategoryList.GetSelectedRow();
		m_CategoryList.ClearItems();
		foreach (string category : m_Categories)
		{
			m_CategoryList.AddItem(category, null, 0);
		}

		if (previous < 0 || previous >= m_Categories.Count())
			previous = 0;
		if (m_Categories.Count() > 0)
			m_CategoryList.SelectRow(previous);

		RefreshItems();
	}

	protected void RefreshItems()
	{
		if (!m_ItemList || !m_CategoryList)
			return;

		m_ShownCategory = m_CategoryList.GetSelectedRow();
		m_ItemList.ClearItems();
		m_VisibleItems.Clear();

		for (int i = 0; i < m_ItemClass.Count(); i++)
		{
			if (m_ItemCategory[i] != m_ShownCategory)
				continue;

			string buy = "-";
			if (m_ItemBuy[i] >= 0)
				buy = m_ItemBuy[i].ToString();
			string sell = "-";
			if (m_ItemSell[i] >= 0)
				sell = m_ItemSell[i].ToString();

			m_ItemList.AddItem(m_ItemName[i] + "   [koupit " + buy + " / vykup " + sell + "]", null, 0);
			m_VisibleItems.Insert(i);
		}
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
			string cls = m_SellClasses[i];
			string label = BC_Client.DisplayName(cls) + " x" + m_SellCounts[i].ToString();
			int price = SellPrice(cls);
			if (price >= 0)
				label = label + "   (az " + price.ToString() + ")";
			m_SellList.AddItem(label, null, 0);
		}
	}

	protected int SellPrice(string className)
	{
		string wanted = className;
		wanted.ToLower();
		for (int i = 0; i < m_ItemClass.Count(); i++)
		{
			string cls = m_ItemClass[i];
			cls.ToLower();
			if (cls == wanted)
				return m_ItemSell[i];
		}
		return -1;
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		// změna kategorie -> přestavět seznam předmětů
		if (m_CategoryList && m_CategoryList.GetSelectedRow() != m_ShownCategory)
			RefreshItems();
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
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	protected void Buy()
	{
		if (!m_ItemList)
			return;

		int row = m_ItemList.GetSelectedRow();
		if (row < 0 || row >= m_VisibleItems.Count())
		{
			SetStatus("Vyber predmet, ktery chces koupit.");
			return;
		}

		int index = m_VisibleItems[row];
		if (m_ItemBuy[index] < 0)
		{
			SetStatus("Tento predmet se neprodava.");
			return;
		}
		if (m_ItemBuy[index] > m_Balance)
		{
			SetStatus("Nemas dost penez.");
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ItemCategory[index]);
		rpc.Write(m_ItemIndexInCategory[index]);
		rpc.Send(null, BC_RPC_TRADER_BUY, true, null);
		SetStatus("Koupeno: " + m_ItemName[index]);
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
}

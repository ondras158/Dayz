// Menu farmy (klávesa U). Pořadí čtení dat musí odpovídat OS_ServerFarms.SendData.

class OS_FarmData
{
	bool Enabled;
	string Currency;
	int BuildPrice;
	bool HasOwn;
	float OwnX;
	float OwnZ;
	bool IsNear;
	bool IsOwner;
	float FarmX;
	float FarmZ;
	int Feed;
	int MaxFeed;
	int FeedPrice;
	int FeedPerHour;
	ref array<string> AnimalIds = new array<string>;
	ref array<string> AnimalNames = new array<string>;
	ref array<int> AnimalCounts = new array<int>;
	ref array<int> AnimalMax = new array<int>;
	ref array<int> AnimalPrices = new array<int>;
	ref array<string> StoredClasses = new array<string>;
	ref array<int> StoredCounts = new array<int>;
	bool AtTrader;              // hráč stojí u Farmáře
	string TraderName;
	bool AnimalsOnlyAtTrader;
	bool FeedAtFarm;
	ref array<string> AnimalNeeds = new array<string>;   // chybějící stavba pro druh ("" = nic)
	int Fertilizer;
	int MaxFertilizer;
	int FertilizerPrice;
	int FertilizerPerHour;
	ref array<string> BuildingIds = new array<string>;
	ref array<string> BuildingNames = new array<string>;
	ref array<int> BuildingPrices = new array<int>;
	ref array<int> BuildingBuilt = new array<int>;

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Enabled) || !ctx.Read(Currency) || !ctx.Read(BuildPrice) || !ctx.Read(HasOwn) || !ctx.Read(OwnX) || !ctx.Read(OwnZ))
			return false;
		if (!ctx.Read(IsNear) || !ctx.Read(IsOwner) || !ctx.Read(FarmX) || !ctx.Read(FarmZ))
			return false;
		if (!ctx.Read(Feed) || !ctx.Read(MaxFeed) || !ctx.Read(FeedPrice) || !ctx.Read(FeedPerHour))
			return false;
		if (!ctx.Read(AnimalIds) || !ctx.Read(AnimalNames) || !ctx.Read(AnimalCounts) || !ctx.Read(AnimalMax) || !ctx.Read(AnimalPrices))
			return false;
		if (!ctx.Read(StoredClasses) || !ctx.Read(StoredCounts))
			return false;
		if (!ctx.Read(AtTrader) || !ctx.Read(TraderName) || !ctx.Read(AnimalsOnlyAtTrader) || !ctx.Read(FeedAtFarm))
			return false;
		if (!ctx.Read(AnimalNeeds) || !ctx.Read(Fertilizer) || !ctx.Read(MaxFertilizer) || !ctx.Read(FertilizerPrice) || !ctx.Read(FertilizerPerHour))
			return false;
		if (!ctx.Read(BuildingIds) || !ctx.Read(BuildingNames) || !ctx.Read(BuildingPrices) || !ctx.Read(BuildingBuilt))
			return false;
		return true;
	}
}

class OS_FarmMenu : OS_MenuBase
{
	protected ref OS_FarmData m_Data;

	protected TextWidget m_Info;
	protected TextWidget m_Info2;
	protected TextWidget m_FeedText;
	protected TextWidget m_FertText;
	protected TextListboxWidget m_BuildingList;
	protected ButtonWidget m_BtnBuyFert;
	protected ButtonWidget m_BtnBuildStructure;
	protected TextListboxWidget m_AnimalList;
	protected TextListboxWidget m_StorageList;
	protected EditBoxWidget m_EditFeed;
	protected ButtonWidget m_BtnBuyAnimal;
	protected ButtonWidget m_BtnBuyFeed;
	protected ButtonWidget m_BtnCollect;
	protected ButtonWidget m_BtnCollectAll;
	protected ButtonWidget m_BtnBuild;
	protected ButtonWidget m_BtnRemove;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "farm_menu.layout";
	}

	void SetData(OS_FarmData data)
	{
		m_Data = data;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_Info = FindText("Info");
		m_Info2 = FindText("Info2");
		m_FeedText = FindText("FeedText");
		m_FertText = FindText("FertText");
		m_BuildingList = FindList("BuildingList");
		m_BtnBuyFert = FindButton("BtnBuyFert");
		m_BtnBuildStructure = FindButton("BtnBuildStructure");
		m_AnimalList = FindList("AnimalList");
		m_StorageList = FindList("StorageList");
		m_EditFeed = FindEdit("EditFeed");
		m_BtnBuyAnimal = FindButton("BtnBuyAnimal");
		m_BtnBuyFeed = FindButton("BtnBuyFeed");
		m_BtnCollect = FindButton("BtnCollect");
		m_BtnCollectAll = FindButton("BtnCollectAll");
		m_BtnBuild = FindButton("BtnBuild");
		m_BtnRemove = FindButton("BtnRemove");
		m_BtnClose = FindButton("BtnClose");
		Refresh();
	}

	protected void ShowNames(array<string> names, bool visible)
	{
		foreach (string name : names)
		{
			Widget w = layoutRoot.FindAnyWidget(name);
			if (w)
				w.Show(visible);
		}
	}

	protected void Label(TextWidget w, string text)
	{
		if (w)
			w.SetText(text);
	}

	protected void Refresh()
	{
		if (!m_Data)
			return;

		string cur = " " + m_Data.Currency;
		bool atTrader = m_Data.Enabled && m_Data.AtTrader;
		bool atFarm = m_Data.Enabled && m_Data.IsNear && !atTrader;
		bool traderShop = atTrader && m_Data.HasOwn;

		// u Farmáře: zvířata + krmivo, na farmě: sklad (+ zvířata/krmivo podle nastavení)
		array<string> animalWidgets = {"AnimalsLabel", "AnimalListBg", "AnimalList"};
		array<string> feedWidgets = {"FeedText", "FertText", "EditFeedBg", "EditFeed", "BtnBuyFeed", "BtnBuyFert"};
		array<string> buildingWidgets = {"BuildingsLabel", "BuildingListBg", "BuildingList", "BtnBuildStructure"};
		ShowNames(buildingWidgets, atFarm);
		array<string> storageWidgets = {"StorageLabel", "StorageListBg", "StorageList", "BtnCollect", "BtnCollectAll"};
		ShowNames(animalWidgets, traderShop || atFarm);
		ShowNames(feedWidgets, traderShop || (atFarm && m_Data.FeedAtFarm));
		ShowNames(storageWidgets, atFarm);
		if (m_BtnBuyAnimal)
			m_BtnBuyAnimal.Show(traderShop || (atFarm && !m_Data.AnimalsOnlyAtTrader));
		if (m_BtnBuild)
			m_BtnBuild.Show(m_Data.Enabled && !m_Data.HasOwn && !atTrader && !atFarm);
		if (m_BtnRemove)
			m_BtnRemove.Show(m_Data.HasOwn && !atTrader && (!m_Data.IsNear || m_Data.IsOwner));

		if (atTrader)
		{
			Label(m_Info, m_Data.TraderName + " - zvirata, stromky, sazenice, krmivo a hnojivo pro tvoji farmu");
			if (m_Data.HasOwn)
				Label(m_Info2, "Koupena zvirata a krmivo se dovezou na tvoji farmu.");
			else
				Label(m_Info2, "Nejdriv si postav farmu (klavesa U), pak se sem vrat.");
		}
		else if (!m_Data.Enabled)
		{
			Label(m_Info, "Farmy jsou na serveru vypnute.");
			Label(m_Info2, "");
		}
		else if (m_Data.IsNear)
		{
			if (m_Data.IsOwner)
				Label(m_Info, "Tvoje farma");
			else
				Label(m_Info, "Farma spoluhrace ze skupiny");
			if (m_Data.AnimalsOnlyAtTrader)
				Label(m_Info2, "Zvirata koupis u Farmare. Vyrabeji i kdyz nejsi online - jen kdyz maji krmivo.");
			else
				Label(m_Info2, "Zvirata vyrabeji i kdyz nejsi online - jen kdyz maji krmivo.");
		}
		else if (m_Data.HasOwn)
		{
			int fx = Math.Round(m_Data.OwnX);
			int fz = Math.Round(m_Data.OwnZ);
			Label(m_Info, "Tvoje farma je na " + fx.ToString() + " / " + fz.ToString() + " (i na mape).");
			Label(m_Info2, "Dojdi k ni a otevri menu znovu (klavesa U).");
		}
		else
		{
			Label(m_Info, "Nemas farmu. Postav ji tam, kde stojis.");
			Label(m_Info2, "");
		}

		if (m_BtnBuild)
			m_BtnBuild.SetText("Postavit farmu zde (" + m_Data.BuildPrice.ToString() + cur + ")");

		// zvířata
		if (m_AnimalList)
		{
			int animalRow = m_AnimalList.GetSelectedRow();
			m_AnimalList.ClearItems();
			for (int i = 0; i < m_Data.AnimalIds.Count(); i++)
			{
				string label = m_Data.AnimalNames[i] + ": " + m_Data.AnimalCounts[i].ToString() + " / " + m_Data.AnimalMax[i].ToString() + "   (dalsi " + m_Data.AnimalPrices[i].ToString() + cur + ")";
				if (i < m_Data.AnimalNeeds.Count() && m_Data.AnimalNeeds[i] != "")
					label = label + "   - potreba: " + m_Data.AnimalNeeds[i];
				m_AnimalList.AddItem(label, null, 0);
			}
			if (animalRow >= 0 && animalRow < m_Data.AnimalIds.Count())
				m_AnimalList.SelectRow(animalRow);
		}

		// krmivo
		string feedInfo = "Krmivo: " + m_Data.Feed.ToString() + " / " + m_Data.MaxFeed.ToString() + "   spotreba " + m_Data.FeedPerHour.ToString() + " / hod";
		if (m_Data.FeedPerHour > 0)
		{
			int hours = m_Data.Feed / m_Data.FeedPerHour;
			feedInfo = feedInfo + "   (vydrzi ~" + hours.ToString() + " h)";
		}
		feedInfo = feedInfo + "   cena " + m_Data.FeedPrice.ToString() + cur + "/ks";
		Label(m_FeedText, feedInfo);

		string fertInfo = "Hnojivo (sad, sklenik): " + m_Data.Fertilizer.ToString() + " / " + m_Data.MaxFertilizer.ToString() + "   spotreba " + m_Data.FertilizerPerHour.ToString() + " / hod";
		if (m_Data.FertilizerPerHour > 0)
		{
			int fertHours = m_Data.Fertilizer / m_Data.FertilizerPerHour;
			fertInfo = fertInfo + "   (vydrzi ~" + fertHours.ToString() + " h)";
		}
		fertInfo = fertInfo + "   cena " + m_Data.FertilizerPrice.ToString() + cur + "/ks";
		Label(m_FertText, fertInfo);

		// stavby (sad, skleník...)
		if (m_BuildingList)
		{
			int buildingRow = m_BuildingList.GetSelectedRow();
			m_BuildingList.ClearItems();
			for (int b = 0; b < m_Data.BuildingIds.Count(); b++)
			{
				string buildingLabel = m_Data.BuildingNames[b];
				if (m_Data.BuildingBuilt[b] == 1)
					buildingLabel = buildingLabel + "   - postaveno";
				else
					buildingLabel = buildingLabel + "   (" + m_Data.BuildingPrices[b].ToString() + cur + ")";
				m_BuildingList.AddItem(buildingLabel, null, 0);
			}
			if (buildingRow >= 0 && buildingRow < m_Data.BuildingIds.Count())
				m_BuildingList.SelectRow(buildingRow);
		}

		// sklad
		if (m_StorageList)
		{
			m_StorageList.ClearItems();
			for (int s = 0; s < m_Data.StoredClasses.Count(); s++)
			{
				m_StorageList.AddItem(OS_Client.DisplayName(m_Data.StoredClasses[s]) + " x" + m_Data.StoredCounts[s].ToString(), null, 0);
			}
		}
	}

	protected void SendAction(int action, string arg, int amount)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(arg);
		rpc.Write(amount);
		rpc.Send(null, OS_RPC_FARM_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Data)
			return super.OnClick(w, x, y, button);

		if (w == m_BtnBuild)
		{
			SendAction(OS_FarmAction.BUILD, "", 0);
			return true;
		}
		if (w == m_BtnRemove)
		{
			SendAction(OS_FarmAction.REMOVE, "", 0);
			return true;
		}
		if (w == m_BtnBuyAnimal && m_AnimalList)
		{
			int row = m_AnimalList.GetSelectedRow();
			if (row < 0 || row >= m_Data.AnimalIds.Count())
				SetStatus("Vyber zvire.");
			else
				SendAction(OS_FarmAction.BUY_ANIMAL, m_Data.AnimalIds[row], 0);
			return true;
		}
		if (w == m_BtnBuyFeed && m_EditFeed)
		{
			string text = m_EditFeed.GetText();
			text.TrimInPlace();
			int amount = text.ToInt();
			if (amount <= 0)
				SetStatus("Zadej mnozstvi krmiva.");
			else
				SendAction(OS_FarmAction.BUY_FEED, "", amount);
			return true;
		}
		if (w == m_BtnCollect && m_StorageList)
		{
			int stored = m_StorageList.GetSelectedRow();
			if (stored < 0 || stored >= m_Data.StoredClasses.Count())
				SetStatus("Vyber surovinu.");
			else
				SendAction(OS_FarmAction.COLLECT, m_Data.StoredClasses[stored], -1);
			return true;
		}
		if (w == m_BtnBuyFert && m_EditFeed)
		{
			string fertText = m_EditFeed.GetText();
			fertText.TrimInPlace();
			int fertAmount = fertText.ToInt();
			if (fertAmount <= 0)
				SetStatus("Zadej mnozstvi hnojiva.");
			else
				SendAction(OS_FarmAction.BUY_FERTILIZER, "", fertAmount);
			return true;
		}
		if (w == m_BtnBuildStructure && m_BuildingList)
		{
			int buildRow = m_BuildingList.GetSelectedRow();
			if (buildRow < 0 || buildRow >= m_Data.BuildingIds.Count())
				SetStatus("Vyber stavbu (sad, sklenik).");
			else
				SendAction(OS_FarmAction.BUILD_STRUCTURE, m_Data.BuildingIds[buildRow], 0);
			return true;
		}
		if (w == m_BtnCollectAll)
		{
			SendAction(OS_FarmAction.COLLECT_ALL, "", 0);
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

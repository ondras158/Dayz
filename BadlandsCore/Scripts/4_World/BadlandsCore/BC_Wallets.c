// Peněženky hráčů. Každý hráč má soubor $profile:BadlandsCore/Wallets/<SteamID>.json

class BC_WalletData
{
	int Balance;
}

class BC_Wallets
{
	protected ref map<string, ref BC_WalletData> m_Cache = new map<string, ref BC_WalletData>;
	protected int m_StartingBalance;

	void BC_Wallets(int startingBalance)
	{
		m_StartingBalance = startingBalance;
		MakeDirectory(BC_DIR);
		MakeDirectory(BC_DIR + "Wallets");
	}

	protected string PathFor(string steamId)
	{
		return BC_DIR + "Wallets/" + steamId + ".json";
	}

	protected BC_WalletData GetData(string steamId)
	{
		BC_WalletData data;
		if (m_Cache.Find(steamId, data))
			return data;

		data = new BC_WalletData();
		string path = PathFor(steamId);
		string err;

		if (FileExist(path))
		{
			if (!JsonFileLoader<BC_WalletData>.LoadFile(path, data, err) || !data)
			{
				Print(BC_TAG + "CHYBA pri nacitani penezenky " + path + ": " + err);
				data = new BC_WalletData();
			}
		}
		else
		{
			data.Balance = m_StartingBalance;
			Save(steamId, data);
		}

		m_Cache.Set(steamId, data);
		return data;
	}

	protected void Save(string steamId, BC_WalletData data)
	{
		string err;
		if (!JsonFileLoader<BC_WalletData>.SaveFile(PathFor(steamId), data, err))
			Print(BC_TAG + "CHYBA pri ukladani penezenky " + steamId + ": " + err);
	}

	int GetBalance(string steamId)
	{
		if (steamId == "")
			return 0;
		return GetData(steamId).Balance;
	}

	void Add(string steamId, int amount)
	{
		if (steamId == "" || amount == 0)
			return;

		BC_WalletData data = GetData(steamId);
		data.Balance = data.Balance + amount;
		if (data.Balance < 0)
			data.Balance = 0;
		Save(steamId, data);
	}

	bool TrySpend(string steamId, int amount)
	{
		if (steamId == "" || amount < 0)
			return false;

		BC_WalletData data = GetData(steamId);
		if (data.Balance < amount)
			return false;

		data.Balance = data.Balance - amount;
		Save(steamId, data);
		return true;
	}

	//! Uvolní paměť po odpojení hráče (data jsou už uložená)
	void Forget(string steamId)
	{
		m_Cache.Remove(steamId);
	}
}

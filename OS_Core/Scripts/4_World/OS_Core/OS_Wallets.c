// Peněženky hráčů. Každý hráč má soubor $profile:OS_Core/Wallets/<SteamID>.json

class OS_WalletData
{
	int Balance;
}

class OS_Wallets
{
	protected ref map<string, ref OS_WalletData> m_Cache = new map<string, ref OS_WalletData>;
	protected int m_StartingBalance;

	void OS_Wallets(int startingBalance)
	{
		m_StartingBalance = startingBalance;
		MakeDirectory(OS_DIR);
		MakeDirectory(OS_DIR + "Wallets");
	}

	//! Steam64 ID = přesně 17 číslic (chrání i před zneužitím názvu souboru)
	static bool IsValidSteamId(string steamId)
	{
		if (steamId.Length() != 17)
			return false;
		string digits = "0123456789";
		for (int i = 0; i < steamId.Length(); i++)
		{
			string ch = steamId.Get(i);
			if (digits.IndexOf(ch) == -1)
				return false;
		}
		return true;
	}

	//! Má hráč účet? (už někdy hrál na serveru)
	bool HasAccount(string steamId)
	{
		if (!IsValidSteamId(steamId))
			return false;
		if (m_Cache.Contains(steamId))
			return true;
		return FileExist(PathFor(steamId));
	}

	protected string PathFor(string steamId)
	{
		return OS_DIR + "Wallets/" + steamId + ".json";
	}

	protected OS_WalletData GetData(string steamId)
	{
		OS_WalletData data;
		if (m_Cache.Find(steamId, data))
			return data;

		data = new OS_WalletData();
		string path = PathFor(steamId);
		string err;

		if (FileExist(path))
		{
			if (!JsonFileLoader<OS_WalletData>.LoadFile(path, data, err) || !data)
			{
				Print(OS_TAG + "CHYBA pri nacitani penezenky " + path + ": " + err);
				data = new OS_WalletData();
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

	protected void Save(string steamId, OS_WalletData data)
	{
		string err;
		if (!JsonFileLoader<OS_WalletData>.SaveFile(PathFor(steamId), data, err))
			Print(OS_TAG + "CHYBA pri ukladani penezenky " + steamId + ": " + err);
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

		OS_WalletData data = GetData(steamId);
		data.Balance = data.Balance + amount;
		if (data.Balance < 0)
			data.Balance = 0;
		Save(steamId, data);
	}

	bool TrySpend(string steamId, int amount)
	{
		if (steamId == "" || amount < 0)
			return false;

		OS_WalletData data = GetData(steamId);
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

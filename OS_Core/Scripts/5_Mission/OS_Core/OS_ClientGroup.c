// Polohy spoluhráčů ze serveru (pro mapu a jména nad hlavou).

class OS_ClientGroup
{
	static bool ShowMapMarkers = true;
	static bool ShowNameTags = true;
	static float NameTagMaxDistance = 1000;
	static bool NameTagShowDistance = true;

	static ref array<string> Names = new array<string>;
	static ref array<vector> Positions = new array<vector>;
	static ref array<int> NetLow = new array<int>;
	static ref array<int> NetHigh = new array<int>;
	static float LastUpdate = -1000;

	protected static const float EXPIRE_SECONDS = 12;   // bez aktualizace = data neplatí (spoluhráč odešel)

	static bool Read(ParamsReadContext ctx)
	{
		array<string> names = new array<string>;
		array<float> posX = new array<float>;
		array<float> posY = new array<float>;
		array<float> posZ = new array<float>;
		array<int> low = new array<int>;
		array<int> high = new array<int>;

		if (!ctx.Read(ShowMapMarkers) || !ctx.Read(ShowNameTags) || !ctx.Read(NameTagMaxDistance) || !ctx.Read(NameTagShowDistance))
			return false;
		if (!ctx.Read(names) || !ctx.Read(posX) || !ctx.Read(posY) || !ctx.Read(posZ) || !ctx.Read(low) || !ctx.Read(high))
			return false;

		Names.Clear();
		Positions.Clear();
		NetLow.Clear();
		NetHigh.Clear();
		for (int i = 0; i < names.Count(); i++)
		{
			Names.Insert(names[i]);
			Positions.Insert(Vector(posX[i], posY[i], posZ[i]));
			NetLow.Insert(low[i]);
			NetHigh.Insert(high[i]);
		}
		LastUpdate = g_Game.GetTickTime();
		return true;
	}

	static int Count()
	{
		if (g_Game.GetTickTime() - LastUpdate > EXPIRE_SECONDS)
			return 0;
		return Names.Count();
	}

	//! postava spoluhráče, pokud je v dosahu sítě (jinak null)
	static PlayerBase GetEntity(int index)
	{
		return PlayerBase.Cast(g_Game.GetObjectByNetworkId(NetLow[index], NetHigh[index]));
	}

	//! aktuální poloha: živá postava, jinak poslední poloha ze serveru
	static vector GetPosition(int index)
	{
		PlayerBase entity = GetEntity(index);
		if (entity)
			return entity.GetPosition();
		return Positions[index];
	}
}

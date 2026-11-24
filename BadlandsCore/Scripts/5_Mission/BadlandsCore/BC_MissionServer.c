modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		BC_Server.Start();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (BC_Server.Get())
			BC_Server.Get().OnUpdate(timeslice);
	}

	override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
	{
		PlayerBase player = super.OnClientNewEvent(identity, pos, ctx);

		if (player && BC_Server.Get())
			BC_Server.Get().OnNewCharacter(player);

		return player;
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (player && BC_Server.Get())
			BC_Server.Get().OnConnect(player);
	}

	override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		string steamId = "";
		if (identity)
			steamId = identity.GetPlainId();

		super.PlayerDisconnected(player, identity, uid);

		if (BC_Server.Get())
			BC_Server.Get().OnDisconnect(steamId);
	}
}

modded class MissionGameplay
{
	override void OnInit()
	{
		super.OnInit();
		BC_Client.Start();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		// klávesy jen když není otevřené žádné jiné menu (inventář, chat...)
		if (g_Game.GetUIManager().GetMenu() || !g_Game.GetPlayer())
			return;

		UAInput traderInput = GetUApi().GetInputByName("UABCTraderMenu");
		if (traderInput && traderInput.LocalPress())
			BC_Client.Get().RequestTrader();

		UAInput adminInput = GetUApi().GetInputByName("UABCAdminMenu");
		if (adminInput && adminInput.LocalPress())
			BC_Client.Get().RequestAdmin();
	}
}

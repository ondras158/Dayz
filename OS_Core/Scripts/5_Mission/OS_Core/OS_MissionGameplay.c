modded class MissionGameplay
{
	override void OnInit()
	{
		super.OnInit();
		OS_Client.Start();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		// klávesy jen když není otevřené žádné jiné menu (inventář, chat...)
		if (g_Game.GetUIManager().GetMenu() || !g_Game.GetPlayer())
			return;

		UAInput traderInput = GetUApi().GetInputByName("UAOSTraderMenu");
		if (traderInput && traderInput.LocalPress())
			OS_Client.Get().RequestTrader();

		UAInput adminInput = GetUApi().GetInputByName("UAOSAdminMenu");
		if (adminInput && adminInput.LocalPress())
			OS_Client.Get().RequestAdmin();
	}
}

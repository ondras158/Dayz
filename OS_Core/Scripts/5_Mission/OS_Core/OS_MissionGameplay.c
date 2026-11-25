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

		if (OS_Client.Get())
			OS_Client.Get().OnFrame();

		// klávesy jen když není otevřené žádné jiné menu (inventář, chat...)
		if (g_Game.GetUIManager().GetMenu() || !g_Game.GetPlayer())
			return;

		UAInput traderInput = GetUApi().GetInputByName("UAOSTraderMenu");
		if (traderInput && traderInput.LocalPress())
			OS_Client.Get().RequestTrader();

		UAInput farmInput = GetUApi().GetInputByName("UAOSFarmMenu");
		if (farmInput && farmInput.LocalPress())
			OS_Client.Get().RequestFarm();

		UAInput garageInput = GetUApi().GetInputByName("UAOSGarageMenu");
		if (garageInput && garageInput.LocalPress())
			OS_Client.Get().RequestPersonalGarage();

		UAInput groupInput = GetUApi().GetInputByName("UAOSGroupMenu");
		if (groupInput && groupInput.LocalPress())
			OS_Client.Get().RequestGroup();

		UAInput adminInput = GetUApi().GetInputByName("UAOSAdminMenu");
		if (adminInput && adminInput.LocalPress())
			OS_Client.Get().RequestAdmin();

		UAInput skillsInput = GetUApi().GetInputByName("UAOSSkillsMenu");
		if (skillsInput && skillsInput.LocalPress())
			OS_Client.SendToServer(OS_RPC_SKILLS_OPEN);

		UAInput freecamInput = GetUApi().GetInputByName("UAOSFreecam");
		if (freecamInput && freecamInput.LocalPress() && OS_ClientAdmin.IsAdmin && OS_Client.Get())
			OS_Client.Get().GetFreecam().Toggle();

		UAInput mapInput = GetUApi().GetInputByName("UAOSMap");
		if (mapInput && mapInput.LocalPress() && OS_ClientZones.MapWithoutItem)
			OS_OpenMap();
	}

	//! mapa bez mapy v inventáři (PvP.json: MapWithoutItem)
	protected void OS_OpenMap()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsAlive() || player.IsUnconscious() || player.IsRestrained())
			return;
		if (g_Game.GetUIManager().IsMenuOpen(MENU_MAP))
			return;

		if (g_Game.GetUIManager().EnterScriptedMenu(MENU_MAP, null))
		{
			MapMenu.s_OS_OpenedByKey = true;
			MapMenu.s_OS_OpenedAt = g_Game.GetTickTime();
			AddActiveInputExcludes({"map"});
			AddActiveInputRestriction(EInputRestrictors.MAP);
		}
	}
}

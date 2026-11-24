// Zamknutí / odemknutí auta bez klíče: podívej se na auto a stiskni F.
// Může majitel nebo člen skupiny, se kterou je auto sdílené.

class ActionOSLockCar : ActionInteractBase
{
	void ActionOSLockCar()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "Zamknout";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	//! auto, na které se hráč dívá (karoserie nebo dveře)
	static Transport GetTargetVehicle(ActionTarget target)
	{
		if (!target)
			return null;

		Transport car = Transport.Cast(target.GetObject());
		if (!car)
			car = Transport.Cast(target.GetParent());
		return car;
	}

	//! má hráč k autu přístup? (server: skutečná skupina, klient: skupina ze serveru)
	static bool HasAccess(PlayerBase player, Transport car)
	{
		if (!player || !car || !player.GetIdentity())
			return false;

		string id = player.GetIdentity().GetPlainId();
		int groupId = 0;
		if (g_Game.IsServer())
		{
			if (OS_Server.Get() && OS_Server.Get().m_Groups)
			{
				OS_Group group = OS_Server.Get().m_Groups.GetGroupOf(id);
				if (group)
					groupId = group.Id;
			}
		}
		else
		{
			groupId = OS_ClientSettings.GroupId;
		}
		return car.OS_CanAccess(id, groupId);
	}

	//! true = zamykat, false = odemykat
	bool WantsLocked()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Transport car = GetTargetVehicle(target);
		if (!car || player.GetCommand_Vehicle())
			return false;
		if (!IsInReach(player, target, UAMaxDistances.DEFAULT))
			return false;
		if (car.OS_IsLocked() == WantsLocked())
			return false;
		if (!HasAccess(player, car))
			return false;

		// zamknout jen prázdné auto
		if (WantsLocked())
		{
			for (int i = 0; i < car.CrewSize(); i++)
			{
				if (car.CrewMember(i))
					return false;
			}
		}
		return true;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		Transport car = GetTargetVehicle(action_data.m_Target);
		if (!car || !HasAccess(action_data.m_Player, car))
			return;

		car.OS_SetLocked(WantsLocked());
		if (WantsLocked())
			OS_Util.Notify(action_data.m_Player, "Auto", "Zamceno", 3);
		else
			OS_Util.Notify(action_data.m_Player, "Auto", "Odemceno", 3);
	}
}

class ActionOSUnlockCar : ActionOSLockCar
{
	void ActionOSUnlockCar()
	{
		m_Text = "Odemknout";
	}

	override bool WantsLocked()
	{
		return false;
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionOSLockCar);
		actions.Insert(ActionOSUnlockCar);
	}
}

// akce bez předmětu v ruce patří hráči
modded class PlayerBase
{
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(ActionOSLockCar, InputActionMap);
		AddAction(ActionOSUnlockCar, InputActionMap);
	}
}

// zamčené dveře nejdou otevřít zvenku
modded class ActionOpenCarDoorsOutside
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Transport car = Transport.Cast(target.GetParent());
		if (car && car.OS_IsLocked())
			return false;
		return super.ActionCondition(player, target, item);
	}
}

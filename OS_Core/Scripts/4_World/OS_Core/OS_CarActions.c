// Akce s klíčem v rukou: podívej se na auto a drž levé tlačítko.

class ActionOSLockCarCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.DEFAULT);
	}
}

class ActionOSLockCar : ActionContinuousBase
{
	void ActionOSLockCar()
	{
		m_CallbackClass = ActionOSLockCarCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "Zamknout auto";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTCursor;
	}

	//! auto, na které se hráč dívá (karoserie nebo dveře)
	static CarScript GetTargetCar(ActionTarget target)
	{
		if (!target)
			return null;

		CarScript car = CarScript.Cast(target.GetObject());
		if (!car)
			car = CarScript.Cast(target.GetParent());
		return car;
	}

	//! true = zamykat, false = odemykat
	bool WantsLocked()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		OS_CarKey key = OS_CarKey.Cast(item);
		CarScript car = GetTargetCar(target);
		if (!key || !car)
			return false;
		if (player.GetCommand_Vehicle())
			return false;
		if (!IsInReach(player, target, UAMaxDistances.DEFAULT))
			return false;
		if (!key.OS_Fits(car))
			return false;

		// zamknout jen odemčené (a prázdné) auto, odemknout jen zamčené
		if (car.OS_IsLocked() == WantsLocked())
			return false;
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

	override void OnFinishProgressServer(ActionData action_data)
	{
		OS_CarKey key = OS_CarKey.Cast(action_data.m_MainItem);
		CarScript car = GetTargetCar(action_data.m_Target);
		if (!key || !car || !key.OS_Fits(car))
			return;

		car.OS_SetLocked(WantsLocked());
		if (WantsLocked())
			OS_Util.Notify(action_data.m_Player, "Auto", "Zamceno", 3);
		else
			OS_Util.Notify(action_data.m_Player, "Auto", "Odemceno", 3);
	}

	override bool IsLockTargetOnUse()
	{
		return false;
	}
}

class ActionOSUnlockCar : ActionOSLockCar
{
	void ActionOSUnlockCar()
	{
		m_Text = "Odemknout auto";
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

// zamčené dveře nejdou otevřít zvenku
modded class ActionOpenCarDoorsOutside
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = CarScript.Cast(target.GetParent());
		if (car && car.OS_IsLocked())
			return false;
		return super.ActionCondition(player, target, item);
	}
}

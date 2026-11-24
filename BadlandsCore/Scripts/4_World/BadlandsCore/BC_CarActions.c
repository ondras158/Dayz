// Akce s klíčem v rukou: podívej se na auto a drž levé tlačítko.

class ActionBCLockCarCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.DEFAULT);
	}
}

class ActionBCLockCar : ActionContinuousBase
{
	void ActionBCLockCar()
	{
		m_CallbackClass = ActionBCLockCarCB;
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
		BC_CarKey key = BC_CarKey.Cast(item);
		CarScript car = GetTargetCar(target);
		if (!key || !car)
			return false;
		if (player.GetCommand_Vehicle())
			return false;
		if (!IsInReach(player, target, UAMaxDistances.DEFAULT))
			return false;
		if (!key.BC_Fits(car))
			return false;

		// zamknout jen odemčené (a prázdné) auto, odemknout jen zamčené
		if (car.BC_IsLocked() == WantsLocked())
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
		BC_CarKey key = BC_CarKey.Cast(action_data.m_MainItem);
		CarScript car = GetTargetCar(action_data.m_Target);
		if (!key || !car || !key.BC_Fits(car))
			return;

		car.BC_SetLocked(WantsLocked());
		if (WantsLocked())
			BC_Util.Notify(action_data.m_Player, "Auto", "Zamceno", 3);
		else
			BC_Util.Notify(action_data.m_Player, "Auto", "Odemceno", 3);
	}

	override bool IsLockTargetOnUse()
	{
		return false;
	}
}

class ActionBCUnlockCar : ActionBCLockCar
{
	void ActionBCUnlockCar()
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
		actions.Insert(ActionBCLockCar);
		actions.Insert(ActionBCUnlockCar);
	}
}

// zamčené dveře nejdou otevřít zvenku
modded class ActionOpenCarDoorsOutside
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = CarScript.Cast(target.GetParent());
		if (car && car.BC_IsLocked())
			return false;
		return super.ActionCondition(player, target, item);
	}
}

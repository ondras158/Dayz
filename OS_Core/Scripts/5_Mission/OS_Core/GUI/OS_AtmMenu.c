// Data bankomatu - pořadí čtení musí odpovídat OS_ServerBank.SendData

class OS_AtmData
{
	string Currency;
	int Cash;
	int Bank;
	float DepositFeePercent;
	bool AllowTransfers;
	float TransferFeePercent;
	ref array<string> PlayerNames = new array<string>;
	ref array<string> PlayerIds = new array<string>;
	string GroupName;
	int GroupBalance;
	bool GroupCanWithdraw;

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Currency) || !ctx.Read(Cash) || !ctx.Read(Bank) || !ctx.Read(DepositFeePercent))
			return false;
		if (!ctx.Read(AllowTransfers) || !ctx.Read(TransferFeePercent) || !ctx.Read(PlayerNames) || !ctx.Read(PlayerIds))
			return false;
		if (!ctx.Read(GroupName) || !ctx.Read(GroupBalance) || !ctx.Read(GroupCanWithdraw))
			return false;
		return true;
	}
}

class OS_AtmMenu : OS_MenuBase
{
	protected ref OS_AtmData m_Data;

	protected TextWidget m_CashText;
	protected TextWidget m_BankText;
	protected TextWidget m_FeeText;
	protected EditBoxWidget m_EditAmount;
	protected ButtonWidget m_BtnDeposit;
	protected ButtonWidget m_BtnDepositAll;
	protected ButtonWidget m_BtnWithdraw;
	protected ButtonWidget m_BtnWithdrawAll;
	protected ButtonWidget m_BtnClose;

	protected TextListboxWidget m_PlayerList;
	protected EditBoxWidget m_EditTargetId;
	protected EditBoxWidget m_EditTransferAmount;
	protected ButtonWidget m_BtnTransfer;
	protected TextWidget m_TransferFeeText;
	protected TextWidget m_GroupText;
	protected ButtonWidget m_BtnGroupDeposit;
	protected ButtonWidget m_BtnGroupWithdraw;

	override string LayoutName()
	{
		return "atm_menu.layout";
	}

	void SetData(OS_AtmData data)
	{
		m_Data = data;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_CashText = FindText("CashText");
		m_BankText = FindText("BankText");
		m_FeeText = FindText("FeeText");
		m_EditAmount = FindEdit("EditAmount");
		m_BtnDeposit = FindButton("BtnDeposit");
		m_BtnDepositAll = FindButton("BtnDepositAll");
		m_BtnWithdraw = FindButton("BtnWithdraw");
		m_BtnWithdrawAll = FindButton("BtnWithdrawAll");
		m_BtnClose = FindButton("BtnClose");

		m_PlayerList = FindList("PlayerList");
		m_EditTargetId = FindEdit("EditTargetId");
		m_EditTransferAmount = FindEdit("EditTransferAmount");
		m_BtnTransfer = FindButton("BtnTransfer");
		m_TransferFeeText = FindText("TransferFeeText");
		m_GroupText = FindText("GroupText");
		m_BtnGroupDeposit = FindButton("BtnGroupDeposit");
		m_BtnGroupWithdraw = FindButton("BtnGroupWithdraw");

		Refresh();
	}

	protected void Refresh()
	{
		if (!m_Data)
			return;

		string cur = " " + m_Data.Currency;
		if (m_CashText)
			m_CashText.SetText("Hotovost u sebe: " + m_Data.Cash.ToString() + cur);
		if (m_BankText)
			m_BankText.SetText("Na uctu: " + m_Data.Bank.ToString() + cur);
		if (m_FeeText)
		{
			if (m_Data.DepositFeePercent > 0)
				m_FeeText.SetText("Poplatek za vklad: " + m_Data.DepositFeePercent.ToString() + " %");
			else
				m_FeeText.SetText("Vklad bez poplatku");
		}

		// skupinový účet
		bool hasGroup = m_Data.GroupName != "";
		if (m_GroupText)
		{
			if (hasGroup)
				m_GroupText.SetText("Ucet skupiny " + m_Data.GroupName + ": " + m_Data.GroupBalance.ToString() + cur);
			else
				m_GroupText.SetText("Nejsi ve skupine (klavesa J)");
		}
		Widget groupDepositW = layoutRoot.FindAnyWidget("BtnGroupDeposit");
		if (groupDepositW)
			groupDepositW.Show(hasGroup);
		Widget groupWithdrawW = layoutRoot.FindAnyWidget("BtnGroupWithdraw");
		if (groupWithdrawW)
			groupWithdrawW.Show(hasGroup && m_Data.GroupCanWithdraw);

		// sekce převodu
		array<string> transferWidgets = {"TransferLabel", "PlayerListBg", "PlayerList", "TargetIdLabel", "EditTargetIdBg", "EditTargetId", "TransferAmountLabel", "EditTransferAmountBg", "EditTransferAmount", "BtnTransfer", "TransferFeeText"};
		foreach (string widgetName : transferWidgets)
		{
			Widget w = layoutRoot.FindAnyWidget(widgetName);
			if (w)
				w.Show(m_Data.AllowTransfers);
		}

		if (m_TransferFeeText)
		{
			if (m_Data.TransferFeePercent > 0)
				m_TransferFeeText.SetText("Poplatek za prevod: " + m_Data.TransferFeePercent.ToString() + " %");
			else
				m_TransferFeeText.SetText("Prevod z uctu na ucet, bez poplatku");
		}

		if (m_PlayerList)
		{
			string previous = SelectedPlayerId();
			m_PlayerList.ClearItems();
			for (int i = 0; i < m_Data.PlayerNames.Count(); i++)
			{
				m_PlayerList.AddItem(m_Data.PlayerNames[i], null, 0);
				if (i < m_Data.PlayerIds.Count() && m_Data.PlayerIds[i] == previous)
					m_PlayerList.SelectRow(i);
			}
		}
	}

	protected string SelectedPlayerId()
	{
		if (!m_PlayerList || !m_Data)
			return "";
		int row = m_PlayerList.GetSelectedRow();
		if (row >= 0 && row < m_Data.PlayerIds.Count())
			return m_Data.PlayerIds[row];
		return "";
	}

	protected int ReadInt(EditBoxWidget edit)
	{
		if (!edit)
			return 0;
		string text = edit.GetText();
		text.TrimInPlace();
		return text.ToInt();
	}

	protected void SendAction(int action, int amount)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(amount);
		rpc.Send(null, OS_RPC_BANK_ACTION, true, null);
	}

	protected void Transfer()
	{
		// ručně zadané Steam ID má přednost před výběrem ze seznamu
		string targetId = "";
		if (m_EditTargetId)
		{
			targetId = m_EditTargetId.GetText();
			targetId.TrimInPlace();
		}
		if (targetId == "")
			targetId = SelectedPlayerId();

		if (targetId == "")
		{
			SetStatus("Vyber hrace ze seznamu nebo zadej jeho Steam64 ID.");
			return;
		}

		int amount = ReadInt(m_EditTransferAmount);
		if (amount <= 0)
		{
			SetStatus("Zadej castku k prevodu.");
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(targetId);
		rpc.Write(amount);
		rpc.Send(null, OS_RPC_BANK_TRANSFER, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnDeposit || w == m_BtnWithdraw)
		{
			int amount = ReadInt(m_EditAmount);
			if (amount <= 0)
			{
				SetStatus("Zadej castku.");
				return true;
			}
			if (w == m_BtnDeposit)
				SendAction(OS_BankAction.DEPOSIT, amount);
			else
				SendAction(OS_BankAction.WITHDRAW, amount);
			return true;
		}
		if (w == m_BtnGroupDeposit || w == m_BtnGroupWithdraw)
		{
			int groupAmount = ReadInt(m_EditAmount);
			if (groupAmount <= 0)
				groupAmount = -1;   // bez částky = vše
			if (w == m_BtnGroupDeposit)
				SendAction(OS_BankAction.GROUP_DEPOSIT, groupAmount);
			else
				SendAction(OS_BankAction.GROUP_WITHDRAW, groupAmount);
			return true;
		}
		if (w == m_BtnDepositAll)
		{
			SendAction(OS_BankAction.DEPOSIT, -1);
			return true;
		}
		if (w == m_BtnWithdrawAll)
		{
			SendAction(OS_BankAction.WITHDRAW, -1);
			return true;
		}
		if (w == m_BtnTransfer)
		{
			Transfer();
			return true;
		}
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}
		return super.OnClick(w, x, y, button);
	}
}

class BC_AtmMenu : BC_MenuBase
{
	protected string m_Currency;
	protected int m_Cash;
	protected int m_Bank;
	protected float m_FeePercent;

	protected TextWidget m_CashText;
	protected TextWidget m_BankText;
	protected TextWidget m_FeeText;
	protected EditBoxWidget m_EditAmount;
	protected ButtonWidget m_BtnDeposit;
	protected ButtonWidget m_BtnDepositAll;
	protected ButtonWidget m_BtnWithdraw;
	protected ButtonWidget m_BtnWithdrawAll;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "atm_menu.layout";
	}

	void SetData(string currency, int cash, int bank, float feePercent)
	{
		m_Currency = currency;
		m_Cash = cash;
		m_Bank = bank;
		m_FeePercent = feePercent;
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
		Refresh();
	}

	protected void Refresh()
	{
		if (m_CashText)
			m_CashText.SetText("Hotovost u sebe: " + m_Cash.ToString() + " " + m_Currency);
		if (m_BankText)
			m_BankText.SetText("Na uctu: " + m_Bank.ToString() + " " + m_Currency);
		if (m_FeeText)
		{
			if (m_FeePercent > 0)
				m_FeeText.SetText("Poplatek za vklad: " + m_FeePercent.ToString() + " %");
			else
				m_FeeText.SetText("Vklad bez poplatku");
		}
	}

	protected int ReadAmount()
	{
		if (!m_EditAmount)
			return 0;
		string text = m_EditAmount.GetText();
		text.TrimInPlace();
		return text.ToInt();
	}

	protected void SendAction(int action, int amount)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(amount);
		rpc.Send(null, BC_RPC_BANK_ACTION, true, null);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BtnDeposit || w == m_BtnWithdraw)
		{
			int amount = ReadAmount();
			if (amount <= 0)
			{
				SetStatus("Zadej castku.");
				return true;
			}
			if (w == m_BtnDeposit)
				SendAction(BC_BankAction.DEPOSIT, amount);
			else
				SendAction(BC_BankAction.WITHDRAW, amount);
			return true;
		}
		if (w == m_BtnDepositAll)
		{
			SendAction(BC_BankAction.DEPOSIT, -1);
			return true;
		}
		if (w == m_BtnWithdrawAll)
		{
			SendAction(BC_BankAction.WITHDRAW, -1);
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

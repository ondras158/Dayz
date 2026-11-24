// Data menu skupiny - pořadí čtení musí odpovídat OS_ServerGroups.SendData

class OS_GroupData
{
	string Currency;
	string MyId;
	string GroupName;
	string LeaderId;
	ref array<string> MemberNames = new array<string>;
	ref array<string> MemberIds = new array<string>;
	ref array<int> MemberOnline = new array<int>;
	ref array<int> MemberRoles = new array<int>;
	int MaxMembers;
	int MaxLimit;
	int SlotPrice;
	int Balance;
	bool MembersCanInvite;
	ref array<string> FreeNames = new array<string>;
	ref array<string> FreeIds = new array<string>;
	string InviteGroup;
	string InviteFrom;

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Currency) || !ctx.Read(MyId) || !ctx.Read(GroupName) || !ctx.Read(LeaderId))
			return false;
		if (!ctx.Read(MemberNames) || !ctx.Read(MemberIds) || !ctx.Read(MemberOnline) || !ctx.Read(MemberRoles))
			return false;
		if (!ctx.Read(MaxMembers) || !ctx.Read(MaxLimit) || !ctx.Read(SlotPrice) || !ctx.Read(Balance) || !ctx.Read(MembersCanInvite))
			return false;
		if (!ctx.Read(FreeNames) || !ctx.Read(FreeIds) || !ctx.Read(InviteGroup) || !ctx.Read(InviteFrom))
			return false;
		return true;
	}

	bool HasGroup()
	{
		return GroupName != "";
	}

	int RoleAt(int index)
	{
		if (index >= 0 && index < MemberRoles.Count())
			return MemberRoles[index];
		return OS_GROUP_ROLE_MEMBER;
	}

	int MyRole()
	{
		if (!HasGroup())
			return OS_GROUP_ROLE_NONE;
		return RoleAt(MemberIds.Find(MyId));
	}

	bool IsOwner()
	{
		return MyRole() == OS_GROUP_ROLE_OWNER;
	}

	bool IsManager()
	{
		return MyRole() >= OS_GROUP_ROLE_ADMIN;
	}
}

class OS_GroupMenu : OS_MenuBase
{
	protected ref OS_GroupData m_Data;

	protected TextWidget m_Title;
	protected TextWidget m_Info;
	protected TextListboxWidget m_MemberList;
	protected TextListboxWidget m_FreeList;
	protected TextWidget m_BalanceText;
	protected TextWidget m_InviteText;
	protected TextWidget m_InviteText2;
	protected EditBoxWidget m_EditName;
	protected ButtonWidget m_BtnKick;
	protected ButtonWidget m_BtnPromote;
	protected ButtonWidget m_BtnSetAdmin;
	protected ButtonWidget m_BtnLeave;
	protected ButtonWidget m_BtnDisband;
	protected ButtonWidget m_BtnInvite;
	protected ButtonWidget m_BtnBuySlot;
	protected ButtonWidget m_BtnCreate;
	protected ButtonWidget m_BtnAccept;
	protected ButtonWidget m_BtnDecline;
	protected ButtonWidget m_BtnClose;

	override string LayoutName()
	{
		return "group_menu.layout";
	}

	void SetData(OS_GroupData data)
	{
		m_Data = data;
		if (layoutRoot)
			Refresh();
	}

	override void OnCreated()
	{
		m_Title = FindText("Title");
		m_Info = FindText("InfoText");
		m_MemberList = FindList("MemberList");
		m_FreeList = FindList("FreeList");
		m_BalanceText = FindText("BalanceText");
		m_InviteText = FindText("InviteText");
		m_InviteText2 = FindText("InviteText2");
		m_EditName = FindEdit("EditName");
		m_BtnKick = FindButton("BtnKick");
		m_BtnPromote = FindButton("BtnPromote");
		m_BtnSetAdmin = FindButton("BtnSetAdmin");
		m_BtnLeave = FindButton("BtnLeave");
		m_BtnDisband = FindButton("BtnDisband");
		m_BtnInvite = FindButton("BtnInvite");
		m_BtnBuySlot = FindButton("BtnBuySlot");
		m_BtnCreate = FindButton("BtnCreate");
		m_BtnAccept = FindButton("BtnAccept");
		m_BtnDecline = FindButton("BtnDecline");
		m_BtnClose = FindButton("BtnClose");
		Refresh();
	}

	protected void ShowAll(array<string> names, bool visible)
	{
		foreach (string name : names)
		{
			Widget w = layoutRoot.FindAnyWidget(name);
			if (w)
				w.Show(visible);
		}
	}

	protected void Refresh()
	{
		if (!m_Data)
			return;

		bool inGroup = m_Data.HasGroup();
		bool owner = m_Data.IsOwner();
		bool manager = m_Data.IsManager();
		bool canInvite = inGroup && (manager || m_Data.MembersCanInvite);
		string cur = " " + m_Data.Currency;

		array<string> groupWidgets = {"MembersLabel", "MemberListBg", "MemberList", "BtnLeave", "BalanceText"};
		array<string> ownerWidgets = {"BtnPromote", "BtnDisband", "BtnSetAdmin"};
		array<string> managerWidgets = {"BtnKick", "BtnBuySlot"};
		array<string> inviteWidgets = {"FreeLabel", "FreeListBg", "FreeList", "BtnInvite"};
		array<string> noGroupWidgets = {"CreateLabel", "EditNameBg", "EditName", "BtnCreate", "InviteText", "InviteText2"};
		ShowAll(groupWidgets, inGroup);
		ShowAll(ownerWidgets, owner);
		ShowAll(managerWidgets, manager);
		ShowAll(inviteWidgets, canInvite);
		ShowAll(noGroupWidgets, !inGroup);

		bool hasInvite = !inGroup && m_Data.InviteGroup != "";
		array<string> inviteButtons = {"BtnAccept", "BtnDecline"};
		ShowAll(inviteButtons, hasInvite);

		if (m_Title)
		{
			if (inGroup)
				m_Title.SetText("Skupina " + m_Data.GroupName);
			else
				m_Title.SetText("Skupina");
		}

		if (m_Info)
		{
			if (inGroup)
			{
				string role = OS_Group.RoleName(m_Data.MyRole());
				m_Info.SetText("Clenove: " + m_Data.MemberNames.Count().ToString() + " / " + m_Data.MaxMembers.ToString() + "   Tvoje role: " + role);
			}
			else
			{
				m_Info.SetText("Nejsi v zadne skupine. Zaloz si vlastni nebo prijmi pozvanku.");
			}
		}

		if (m_MemberList)
		{
			string previous = SelectedMemberId();
			m_MemberList.ClearItems();
			for (int i = 0; i < m_Data.MemberNames.Count(); i++)
			{
				string label = m_Data.MemberNames[i];
				int memberRole = m_Data.RoleAt(i);
				if (memberRole != OS_GROUP_ROLE_MEMBER)
					label = label + "  [" + OS_Group.RoleName(memberRole) + "]";
				if (i < m_Data.MemberOnline.Count() && m_Data.MemberOnline[i] == 1)
					label = label + "  (online)";
				else
					label = label + "  (offline)";
				m_MemberList.AddItem(label, null, 0);
				if (m_Data.MemberIds[i] == previous)
					m_MemberList.SelectRow(i);
			}
		}

		if (m_FreeList)
		{
			m_FreeList.ClearItems();
			foreach (string freeName : m_Data.FreeNames)
			{
				m_FreeList.AddItem(freeName, null, 0);
			}
		}

		if (m_BalanceText)
			m_BalanceText.SetText("Ucet skupiny: " + m_Data.Balance.ToString() + cur + " (vklad / vyber v bankomatu)");

		if (m_BtnBuySlot)
		{
			if (m_Data.MaxMembers >= m_Data.MaxLimit)
				m_BtnBuySlot.SetText("Maximum mist (" + m_Data.MaxLimit.ToString() + ")");
			else
				m_BtnBuySlot.SetText("Koupit dalsi misto (" + m_Data.SlotPrice.ToString() + cur + ")");
		}

		if (m_InviteText && m_InviteText2)
		{
			if (hasInvite)
			{
				m_InviteText.SetText("Pozvanka do skupiny " + m_Data.InviteGroup);
				m_InviteText2.SetText("od hrace " + m_Data.InviteFrom);
			}
			else
			{
				m_InviteText.SetText("Zadna pozvanka");
				m_InviteText2.SetText("");
			}
		}
	}

	protected string SelectedMemberId()
	{
		if (!m_MemberList || !m_Data)
			return "";
		int row = m_MemberList.GetSelectedRow();
		if (row >= 0 && row < m_Data.MemberIds.Count())
			return m_Data.MemberIds[row];
		return "";
	}

	protected string SelectedFreeId()
	{
		if (!m_FreeList || !m_Data)
			return "";
		int row = m_FreeList.GetSelectedRow();
		if (row >= 0 && row < m_Data.FreeIds.Count())
			return m_Data.FreeIds[row];
		return "";
	}

	protected void SendAction(int action, string arg)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(arg);
		rpc.Send(null, OS_RPC_GROUP_ACTION, true, null);
	}

	protected void SendForMember(int action)
	{
		string memberId = SelectedMemberId();
		if (memberId == "" || memberId == m_Data.MyId)
		{
			SetStatus("Vyber jineho clena skupiny.");
			return;
		}
		SendAction(action, memberId);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Data)
			return super.OnClick(w, x, y, button);

		if (w == m_BtnCreate && m_EditName)
		{
			SendAction(OS_GroupAction.CREATE, m_EditName.GetText());
			return true;
		}
		if (w == m_BtnAccept)
		{
			SendAction(OS_GroupAction.ACCEPT, "");
			return true;
		}
		if (w == m_BtnDecline)
		{
			SendAction(OS_GroupAction.DECLINE, "");
			return true;
		}
		if (w == m_BtnInvite)
		{
			string freeId = SelectedFreeId();
			if (freeId == "")
				SetStatus("Vyber hrace, ktereho chces pozvat.");
			else
				SendAction(OS_GroupAction.INVITE, freeId);
			return true;
		}
		if (w == m_BtnKick)
		{
			SendForMember(OS_GroupAction.KICK);
			return true;
		}
		if (w == m_BtnPromote)
		{
			SendForMember(OS_GroupAction.PROMOTE);
			return true;
		}
		if (w == m_BtnSetAdmin)
		{
			SendForMember(OS_GroupAction.SET_ADMIN);
			return true;
		}
		if (w == m_BtnLeave)
		{
			SendAction(OS_GroupAction.LEAVE, "");
			return true;
		}
		if (w == m_BtnDisband)
		{
			SendAction(OS_GroupAction.DISBAND, "");
			return true;
		}
		if (w == m_BtnBuySlot)
		{
			SendAction(OS_GroupAction.BUY_SLOT, "");
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

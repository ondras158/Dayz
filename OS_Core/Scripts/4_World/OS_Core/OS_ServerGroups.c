// Skupiny hráčů: založení, pozvánky, členové, dokupování míst a společný účet.

class OS_GroupInvite
{
	int GroupId;
	string InviterName;
	float Expires;
}

class OS_ServerGroups
{
	ref OS_GroupConfig m_Cfg;
	protected ref OS_GroupsData m_Data;
	protected ref map<string, int> m_PlayerGroup = new map<string, int>;          // Steam ID -> ID skupiny
	protected ref map<string, ref OS_GroupInvite> m_Invites = new map<string, ref OS_GroupInvite>;
	protected OS_Payments m_Payments;
	protected OS_Wallets m_Wallets;
	protected string m_Currency;

	protected static const string DATA_FILE = "GroupsData.json";
	protected float m_PositionTimer;

	void OS_ServerGroups(OS_Payments payments, OS_Wallets wallets, string currency)
	{
		m_Payments = payments;
		m_Wallets = wallets;
		m_Currency = currency;
		m_Cfg = OS_ConfigFile<OS_GroupConfig>.Load("Groups.json", new OS_GroupConfig(), new OS_GroupConfig());
		LoadData();
	}

	// ------------------------------------------------------------ data

	protected void LoadData()
	{
		m_Data = new OS_GroupsData();
		string path = OS_DIR + DATA_FILE;
		string err;
		if (FileExist(path) && !JsonFileLoader<OS_GroupsData>.LoadFile(path, m_Data, err))
		{
			Print(OS_TAG + "CHYBA pri nacitani " + path + ": " + err);
			m_Data = new OS_GroupsData();
		}
		if (!m_Data.Groups)
			m_Data.Groups = new array<ref OS_Group>;

		m_PlayerGroup.Clear();
		foreach (OS_Group group : m_Data.Groups)
		{
			if (!group || !group.Members)
				continue;
			foreach (OS_GroupMember member : group.Members)
			{
				if (member)
					m_PlayerGroup.Set(member.Id, group.Id);
			}
		}
		Print(OS_TAG + "Skupiny nacteny: " + m_Data.Groups.Count().ToString());
	}

	protected void SaveData()
	{
		string err;
		if (!JsonFileLoader<OS_GroupsData>.SaveFile(OS_DIR + DATA_FILE, m_Data, err))
			Print(OS_TAG + "CHYBA pri ukladani skupin: " + err);
	}

	OS_Group GetGroupById(int id)
	{
		foreach (OS_Group group : m_Data.Groups)
		{
			if (group && group.Id == id)
				return group;
		}
		return null;
	}

	OS_Group GetGroupOf(string steamId)
	{
		int id;
		if (!m_PlayerGroup.Find(steamId, id))
			return null;
		return GetGroupById(id);
	}

	int MaxMembers(OS_Group group)
	{
		return Math.Min(m_Cfg.BaseMaxMembers + group.ExtraSlots, m_Cfg.MaxMembersLimit);
	}

	int NextSlotPrice(OS_Group group)
	{
		return m_Cfg.SlotPrice + group.ExtraSlots * m_Cfg.SlotPriceIncrease;
	}

	//! true = dva hráči jsou ve stejné skupině
	bool SameGroup(string idA, string idB)
	{
		if (idA == "" || idB == "")
			return false;
		int groupA;
		int groupB;
		if (!m_PlayerGroup.Find(idA, groupA) || !m_PlayerGroup.Find(idB, groupB))
			return false;
		return groupA == groupB;
	}

	//! zranění od spoluhráče se neprovede (když je FriendlyFire vypnutý)
	bool BlocksDamage(PlayerBase victim, EntityAI source)
	{
		if (!m_Cfg.Enabled || m_Cfg.FriendlyFire || !victim || !source)
			return false;

		PlayerBase attacker = OS_Util.ResolveKillerPlayer(source);
		if (!attacker || attacker == victim)
			return false;

		return SameGroup(OS_Util.SteamId(victim), OS_Util.SteamId(attacker));
	}

	bool CanWithdraw(OS_Group group, string steamId)
	{
		return group && (m_Cfg.MembersCanWithdraw || group.IsManager(steamId));
	}

	//! společný účet - používá bankomat
	bool AddBalance(OS_Group group, int amount)
	{
		if (!group || group.Balance + amount < 0)
			return false;
		group.Balance = group.Balance + amount;
		SaveData();
		return true;
	}

	protected void NotifyGroup(OS_Group group, string text, string exceptId = "")
	{
		foreach (OS_GroupMember member : group.Members)
		{
			if (member.Id == exceptId)
				continue;
			PlayerBase online = OS_Util.FindPlayerBySteamId(member.Id);
			if (online)
				OS_Util.Notify(online, OS_F("Skupina %1", group.Name), text);
		}
	}

	// ------------------------------------------------------------ RPC

	bool OnRPC(PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (rpcType != OS_RPC_GROUP_OPEN && rpcType != OS_RPC_GROUP_ACTION)
			return false;

		PlayerBase player = OS_Util.FindPlayer(sender);
		if (!player || !m_Cfg.Enabled)
			return true;

		if (rpcType == OS_RPC_GROUP_ACTION)
		{
			int action;
			string arg;
			if (ctx.Read(action) && ctx.Read(arg))
				HandleAction(player, action, arg);
		}

		SendData(player);
		return true;
	}

	protected void HandleAction(PlayerBase player, int action, string arg)
	{
		string id = OS_Util.SteamId(player);
		string name = OS_Util.PlayerName(player);
		OS_Group group = GetGroupOf(id);

		if (action == OS_GroupAction.CREATE)
			Create(player, id, name, group, arg);
		else if (action == OS_GroupAction.ACCEPT)
			Accept(player, id, name, group);
		else if (action == OS_GroupAction.DECLINE)
			m_Invites.Remove(id);
		else if (!group)
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Nejsi v zadne skupine."));
		else if (action == OS_GroupAction.INVITE)
			Invite(player, id, group, arg);
		else if (action == OS_GroupAction.LEAVE)
			RemoveMember(group, id, OS_F("%1 opustil skupinu.", name));
		else if (action == OS_GroupAction.KICK)
			Kick(player, id, group, arg);
		else if (action == OS_GroupAction.PROMOTE)
			Promote(player, id, group, arg);
		else if (action == OS_GroupAction.BUY_SLOT)
			BuySlot(player, id, group);
		else if (action == OS_GroupAction.DISBAND)
			Disband(player, id, group);
		else if (action == OS_GroupAction.SET_ADMIN)
			SetAdmin(player, id, group, arg);
	}

	protected void Create(PlayerBase player, string id, string name, OS_Group current, string groupName)
	{
		if (current)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Uz jsi ve skupine %1.", current.Name));
			return;
		}

		groupName.TrimInPlace();
		if (groupName.Length() < m_Cfg.MinNameLength || groupName.Length() > m_Cfg.MaxNameLength)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Nazev musi mit %1 az %2 znaku.", m_Cfg.MinNameLength.ToString(), m_Cfg.MaxNameLength.ToString()));
			return;
		}

		foreach (OS_Group existing : m_Data.Groups)
		{
			string a = OS_TraderScanner.Lower(existing.Name);
			if (a == OS_TraderScanner.Lower(groupName))
			{
				OS_Util.Notify(player, OS_T("Skupina"), OS_T("Skupina s timto nazvem uz existuje."));
				return;
			}
		}

		if (m_Cfg.CreatePrice > 0 && !m_Payments.TrySpend(player, m_Cfg.CreatePrice))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Zalozeni stoji %1 %2.", m_Cfg.CreatePrice.ToString(), m_Currency));
			return;
		}

		OS_Group group = new OS_Group();
		group.Id = m_Data.NextId;
		m_Data.NextId = m_Data.NextId + 1;
		group.Name = groupName;
		group.LeaderId = id;
		group.Members.Insert(new OS_GroupMember(id, name));
		m_Data.Groups.Insert(group);
		m_PlayerGroup.Set(id, group.Id);
		m_Invites.Remove(id);
		SaveData();

		SendGroupId(player);
		OS_Util.Notify(player, OS_T("Skupina"), OS_F("Skupina %1 zalozena. Pozvi hrace v menu skupiny.", groupName));
		Print(OS_TAG + "SKUPINA " + name + " zalozil " + groupName);
	}

	protected void Invite(PlayerBase player, string id, OS_Group group, string targetId)
	{
		if (!m_Cfg.MembersCanInvite && !group.IsManager(id))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Zvat muze jen vlastnik nebo admin skupiny."));
			return;
		}
		if (group.Members.Count() >= MaxMembers(group))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Skupina je plna. Dokup dalsi misto."));
			return;
		}

		PlayerBase target = OS_Util.FindPlayerBySteamId(targetId);
		if (!target)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Hrac neni online."));
			return;
		}
		if (GetGroupOf(targetId))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("%1 uz je v jine skupine.", OS_Util.PlayerName(target)));
			return;
		}

		OS_GroupInvite invite = new OS_GroupInvite();
		invite.GroupId = group.Id;
		invite.InviterName = OS_Util.PlayerName(player);
		invite.Expires = g_Game.GetTickTime() + m_Cfg.InviteValidSeconds;
		m_Invites.Set(targetId, invite);

		OS_Util.Notify(player, OS_T("Skupina"), OS_F("Pozvanka odeslana: %1", OS_Util.PlayerName(target)));
		OS_Util.Notify(target, OS_T("Pozvanka do skupiny"), OS_F("%1 te zve do skupiny %2. Otevri menu skupiny (klavesa J).", invite.InviterName, group.Name), 12);
	}

	protected OS_GroupInvite ValidInvite(string id)
	{
		OS_GroupInvite invite;
		if (!m_Invites.Find(id, invite))
			return null;
		if (g_Game.GetTickTime() > invite.Expires || !GetGroupById(invite.GroupId))
		{
			m_Invites.Remove(id);
			return null;
		}
		return invite;
	}

	protected void Accept(PlayerBase player, string id, string name, OS_Group current)
	{
		if (current)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Nejdriv opust svou skupinu."));
			return;
		}

		OS_GroupInvite invite = ValidInvite(id);
		if (!invite)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Pozvanka uz neplati."));
			return;
		}

		OS_Group group = GetGroupById(invite.GroupId);
		m_Invites.Remove(id);
		if (group.Members.Count() >= MaxMembers(group))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Skupina je mezitim plna."));
			return;
		}

		group.Members.Insert(new OS_GroupMember(id, name));
		m_PlayerGroup.Set(id, group.Id);
		SaveData();
		SendGroupId(player);
		NotifyGroup(group, OS_F("%1 se pripojil ke skupine.", name));
		Print(OS_TAG + "SKUPINA " + name + " vstoupil do " + group.Name);
	}

	protected void Kick(PlayerBase player, string id, OS_Group group, string targetId)
	{
		int myRole = group.RoleOf(id);
		if (myRole < OS_GROUP_ROLE_ADMIN)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Vyhazovat muze jen vlastnik nebo admin."));
			return;
		}
		if (targetId == id || group.FindMember(targetId) < 0)
			return;
		if (targetId == group.LeaderId)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Vlastnika skupiny nejde vyhodit."));
			return;
		}

		string targetName = group.Members[group.FindMember(targetId)].Name;
		PlayerBase target = OS_Util.FindPlayerBySteamId(targetId);
		if (target)
			OS_Util.Notify(target, OS_T("Skupina"), OS_F("Byl jsi vyhozen ze skupiny %1.", group.Name));
		RemoveMember(group, targetId, OS_F("%1 byl vyhozen ze skupiny.", targetName));
	}

	protected void Promote(PlayerBase player, string id, OS_Group group, string targetId)
	{
		if (group.LeaderId != id)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Vlastnictvi muze predat jen vlastnik."));
			return;
		}
		if (targetId == id || group.FindMember(targetId) < 0)
			return;

		// původní vlastník zůstane adminem
		group.Members[group.FindMember(id)].Role = OS_GROUP_ROLE_ADMIN;
		group.Members[group.FindMember(targetId)].Role = OS_GROUP_ROLE_MEMBER;
		group.LeaderId = targetId;
		SaveData();
		NotifyGroup(group, OS_F("%1 je novy vlastnik skupiny.", group.Members[group.FindMember(targetId)].Name));
	}

	//! vlastník jmenuje / odvolá admina
	protected void SetAdmin(PlayerBase player, string id, OS_Group group, string targetId)
	{
		if (!group.IsManager(id))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Adminy jmenuje jen vlastnik nebo admin."));
			return;
		}
		int index = group.FindMember(targetId);
		if (targetId == id || targetId == group.LeaderId || index < 0)
			return;

		OS_GroupMember target = group.Members[index];
		if (target.Role == OS_GROUP_ROLE_ADMIN)
		{
			target.Role = OS_GROUP_ROLE_MEMBER;
			NotifyGroup(group, OS_F("%1 uz neni admin skupiny.", target.Name));
		}
		else
		{
			target.Role = OS_GROUP_ROLE_ADMIN;
			NotifyGroup(group, OS_F("%1 je novy admin skupiny.", target.Name));
		}
		SaveData();
	}

	//! odebrání člena; když odejde vlastník, převezme to první admin (jinak první člen); poslední člen dostane zůstatek na svůj účet
	protected void RemoveMember(OS_Group group, string memberId, string message)
	{
		int index = group.FindMember(memberId);
		if (index < 0)
			return;

		group.Members.Remove(index);
		m_PlayerGroup.Remove(memberId);
		ClearPositions(memberId);

		if (group.Members.Count() == 0)
		{
			if (group.Balance > 0)
				m_Wallets.Add(memberId, group.Balance);
			Print(OS_TAG + "SKUPINA " + group.Name + " zanikla (zustatek " + group.Balance.ToString() + " -> " + memberId + ")");
			m_Data.Groups.RemoveItem(group);
			SaveData();
			return;
		}

		// odešel vlastník: převezme to první admin, jinak první člen
		if (group.LeaderId == memberId)
		{
			OS_GroupMember heir = group.Members[0];
			foreach (OS_GroupMember candidate : group.Members)
			{
				if (candidate.Role == OS_GROUP_ROLE_ADMIN)
				{
					heir = candidate;
					break;
				}
			}
			heir.Role = OS_GROUP_ROLE_MEMBER;
			group.LeaderId = heir.Id;
			message = OS_F("%1 Novy vlastnik: %2.", message, heir.Name);
		}

		SaveData();
		NotifyGroup(group, message);

		PlayerBase left = OS_Util.FindPlayerBySteamId(memberId);
		if (left)
			OS_Util.Notify(left, OS_T("Skupina"), message);
	}

	protected void BuySlot(PlayerBase player, string id, OS_Group group)
	{
		if (!group.IsManager(id))
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Mista kupuje vlastnik nebo admin skupiny."));
			return;
		}
		if (MaxMembers(group) >= m_Cfg.MaxMembersLimit)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Skupina uz ma maximum mist (%1).", m_Cfg.MaxMembersLimit.ToString()));
			return;
		}

		int price = NextSlotPrice(group);
		string paidFrom;
		if (group.Balance >= price)
		{
			group.Balance = group.Balance - price;
			paidFrom = OS_T("ze skupinoveho uctu");
		}
		else if (m_Payments.TrySpend(player, price))
		{
			paidFrom = OS_T("z tvych penez");
		}
		else
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Misto stoji %1 %2. Dopln skupinovy ucet v bankomatu nebo mej penize u sebe.", price.ToString(), m_Currency));
			return;
		}

		group.ExtraSlots = group.ExtraSlots + 1;
		SaveData();
		NotifyGroup(group, OS_F("Skupina ma nove misto (%1 hracu), zaplaceno %2.", MaxMembers(group).ToString(), paidFrom));
	}

	protected void Disband(PlayerBase player, string id, OS_Group group)
	{
		if (group.LeaderId != id)
		{
			OS_Util.Notify(player, OS_T("Skupina"), OS_T("Rozpustit skupinu muze jen vlastnik."));
			return;
		}

		NotifyGroup(group, OS_T("Skupina byla rozpustena."));
		if (group.Balance > 0)
		{
			m_Wallets.Add(id, group.Balance);
			OS_Util.Notify(player, OS_T("Skupina"), OS_F("Zustatek %1 %2 prevedeny na tvuj ucet.", group.Balance.ToString(), m_Currency));
		}

		foreach (OS_GroupMember member : group.Members)
		{
			m_PlayerGroup.Remove(member.Id);
			ClearPositions(member.Id);
		}
		Print(OS_TAG + "SKUPINA " + group.Name + " rozpustena (" + OS_Util.PlayerName(player) + ")");
		m_Data.Groups.RemoveItem(group);
		SaveData();
	}

	// ------------------------------------------------------------ polohy spoluhráčů

	void OnTick(float deltaTime)
	{
		if (!m_Cfg.Enabled || (!m_Cfg.ShowMapMarkers && !m_Cfg.ShowNameTags))
			return;

		m_PositionTimer += deltaTime;
		if (m_PositionTimer < m_Cfg.PositionUpdateSeconds)
			return;
		m_PositionTimer = 0;

		foreach (OS_Group group : m_Data.Groups)
		{
			if (!group || group.Members.Count() < 2)
				continue;

			// online členové skupiny
			array<PlayerBase> online = new array<PlayerBase>;
			foreach (OS_GroupMember member : group.Members)
			{
				PlayerBase p = OS_Util.FindPlayerBySteamId(member.Id);
				if (p && p.IsAlive())
					online.Insert(p);
			}
			if (online.Count() < 2)
				continue;

			foreach (PlayerBase recipient : online)
			{
				SendPositions(recipient, online);
			}
		}
	}

	//! pošle hráči polohy ostatních online členů (bez něj samotného)
	protected void SendPositions(PlayerBase recipient, array<PlayerBase> members)
	{
		if (!recipient.GetIdentity())
			return;

		array<string> names = new array<string>;
		array<float> posX = new array<float>;
		array<float> posY = new array<float>;
		array<float> posZ = new array<float>;
		array<int> netLow = new array<int>;
		array<int> netHigh = new array<int>;

		if (members)
		{
			foreach (PlayerBase other : members)
			{
				if (other == recipient)
					continue;
				int low;
				int high;
				other.GetNetworkID(low, high);
				names.Insert(OS_Util.PlayerName(other));
				vector pos = other.GetPosition();
				posX.Insert(pos[0]);
				posY.Insert(pos[1]);
				posZ.Insert(pos[2]);
				netLow.Insert(low);
				netHigh.Insert(high);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Cfg.ShowMapMarkers);
		rpc.Write(m_Cfg.ShowNameTags);
		rpc.Write(m_Cfg.NameTagMaxDistance);
		rpc.Write(m_Cfg.NameTagShowDistance);
		rpc.Write(names);
		rpc.Write(posX);
		rpc.Write(posY);
		rpc.Write(posZ);
		rpc.Write(netLow);
		rpc.Write(netHigh);
		rpc.Send(null, OS_RPC_GROUP_POSITIONS, true, recipient.GetIdentity());
	}

	//! vymaže hráči značky (po odchodu ze skupiny) a pošle mu nové ID skupiny
	protected void ClearPositions(string steamId)
	{
		PlayerBase player = OS_Util.FindPlayerBySteamId(steamId);
		if (player)
		{
			SendPositions(player, null);
			SendGroupId(player);
		}
	}

	//! klient potřebuje znát ID své skupiny (akce odemknout sdílené auto)
	void SendGroupId(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		int groupId = 0;
		OS_Group group = GetGroupOf(OS_Util.SteamId(player));
		if (group)
			groupId = group.Id;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(groupId);
		rpc.Send(null, OS_RPC_GROUP_ID, true, player.GetIdentity());
	}

	// ------------------------------------------------------------ data pro klienta

	protected void SendData(PlayerBase player)
	{
		string id = OS_Util.SteamId(player);
		OS_Group group = GetGroupOf(id);

		// aktualizovat jméno člena (hráč si ho mohl změnit)
		if (group)
		{
			int selfIndex = group.FindMember(id);
			if (selfIndex >= 0 && group.Members[selfIndex].Name != OS_Util.PlayerName(player))
			{
				group.Members[selfIndex].Name = OS_Util.PlayerName(player);
				SaveData();
			}
		}

		array<string> memberNames = new array<string>;
		array<string> memberIds = new array<string>;
		array<int> memberOnline = new array<int>;
		array<int> memberRoles = new array<int>;
		string leaderId = "";
		string groupName = "";
		int maxMembers = m_Cfg.BaseMaxMembers;
		int slotPrice = m_Cfg.SlotPrice;
		int balance = 0;

		if (group)
		{
			groupName = group.Name;
			leaderId = group.LeaderId;
			maxMembers = MaxMembers(group);
			slotPrice = NextSlotPrice(group);
			balance = group.Balance;
			foreach (OS_GroupMember member : group.Members)
			{
				memberNames.Insert(member.Name);
				memberIds.Insert(member.Id);
				memberRoles.Insert(group.RoleOf(member.Id));
				if (OS_Util.FindPlayerBySteamId(member.Id))
					memberOnline.Insert(1);
				else
					memberOnline.Insert(0);
			}
		}

		// hráči online bez skupiny (pro pozvání)
		array<string> freeNames = new array<string>;
		array<string> freeIds = new array<string>;
		array<Man> players = new array<Man>;
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			if (!man || !man.GetIdentity())
				continue;
			string otherId = man.GetIdentity().GetPlainId();
			if (otherId == id || GetGroupOf(otherId))
				continue;
			freeNames.Insert(man.GetIdentity().GetName());
			freeIds.Insert(otherId);
		}

		string inviteGroup = "";
		string inviteFrom = "";
		OS_GroupInvite invite = ValidInvite(id);
		if (invite)
		{
			inviteGroup = GetGroupById(invite.GroupId).Name;
			inviteFrom = invite.InviterName;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Currency);
		rpc.Write(id);
		rpc.Write(groupName);
		rpc.Write(leaderId);
		rpc.Write(memberNames);
		rpc.Write(memberIds);
		rpc.Write(memberOnline);
		rpc.Write(memberRoles);
		rpc.Write(maxMembers);
		rpc.Write(m_Cfg.MaxMembersLimit);
		rpc.Write(slotPrice);
		rpc.Write(balance);
		rpc.Write(m_Cfg.MembersCanInvite);
		rpc.Write(freeNames);
		rpc.Write(freeIds);
		rpc.Write(inviteGroup);
		rpc.Write(inviteFrom);
		rpc.Send(null, OS_RPC_GROUP_DATA, true, player.GetIdentity());
	}
}

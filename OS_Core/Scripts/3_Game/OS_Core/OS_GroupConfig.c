// Skupiny hráčů se společným účtem.
// Nastavení: $profile:OS_Core/Groups.json, data skupin: $profile:OS_Core/GroupsData.json

class OS_GroupConfig
{
	bool Enabled = true;

	// skupinová vysílačka: vysílačky členů (PersonalRadio) se naladí na kanál skupiny
	bool GroupRadio = true;
	int RadioChannels = 8;            // počet kanálů vysílačky (kanál 1 a výš; 0 nechává veřejný)
	bool AutoTuneOnJoin = true;       // naladit po založení / vstupu do skupiny a po připojení
	int BaseMaxMembers = 4;          // velikost skupiny zdarma
	int MaxMembersLimit = 10;        // víc míst koupit nejde
	int SlotPrice = 1000;            // cena prvního dokoupeného místa
	int SlotPriceIncrease = 500;     // každé další místo je dražší o tolik
	int CreatePrice = 0;             // cena založení skupiny
	// role: vlastník (vše), admin (vše kromě vyhození vlastníka, předání vlastnictví a rozpuštění skupiny), člen
	bool MembersCanInvite = false;   // false = zvát může jen vlastník a admini
	bool MembersCanWithdraw = true;  // false = ze společného účtu vybírají jen vlastník a admini
	bool FriendlyFire = false;       // false = členové skupiny si navzájem neublíží (střelba, nože, pěsti)
	int InviteValidSeconds = 300;
	int MinNameLength = 3;
	int MaxNameLength = 24;

	// spoluhráči na mapě a jména nad hlavou
	bool ShowMapMarkers = true;
	bool ShowNameTags = true;
	float NameTagMaxDistance = 1000;   // jména dál než tohle (m) se nezobrazí
	bool NameTagShowDistance = true;   // u jména ukázat i vzdálenost
	float PositionUpdateSeconds = 3;
}

class OS_GroupMember
{
	string Id;      // Steam64 ID
	string Name;    // poslední známé jméno
	int Role;       // OS_GROUP_ROLE_MEMBER / OS_GROUP_ROLE_ADMIN (vlastníka určuje LeaderId)

	void OS_GroupMember(string id = "", string name = "")
	{
		Id = id;
		Name = name;
	}
}

class OS_Group
{
	int Id;
	string Name;
	string LeaderId;
	int ExtraSlots;
	int Balance;     // společný účet
	ref array<ref OS_GroupMember> Members = new array<ref OS_GroupMember>;

	int FindMember(string id)
	{
		for (int i = 0; i < Members.Count(); i++)
		{
			if (Members[i] && Members[i].Id == id)
				return i;
		}
		return -1;
	}

	//! role hráče ve skupině, OS_GROUP_ROLE_NONE když v ní není
	int RoleOf(string id)
	{
		int index = FindMember(id);
		if (index < 0)
			return OS_GROUP_ROLE_NONE;
		if (id == LeaderId)
			return OS_GROUP_ROLE_OWNER;
		if (Members[index].Role == OS_GROUP_ROLE_ADMIN)
			return OS_GROUP_ROLE_ADMIN;
		return OS_GROUP_ROLE_MEMBER;
	}

	//! vlastník nebo admin
	bool IsManager(string id)
	{
		return RoleOf(id) >= OS_GROUP_ROLE_ADMIN;
	}

	static string RoleName(int role)
	{
		if (role == OS_GROUP_ROLE_OWNER)
			return OS_T("vlastnik");
		if (role == OS_GROUP_ROLE_ADMIN)
			return OS_T("admin");
		return OS_T("clen");
	}
}

class OS_GroupsData
{
	int NextId = 1;
	ref array<ref OS_Group> Groups = new array<ref OS_Group>;
}

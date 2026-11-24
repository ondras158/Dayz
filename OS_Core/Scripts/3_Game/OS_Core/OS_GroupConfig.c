// Skupiny hráčů se společným účtem.
// Nastavení: $profile:OS_Core/Groups.json, data skupin: $profile:OS_Core/GroupsData.json

class OS_GroupConfig
{
	bool Enabled = true;
	int BaseMaxMembers = 4;          // velikost skupiny zdarma
	int MaxMembersLimit = 10;        // víc míst koupit nejde
	int SlotPrice = 1000;            // cena prvního dokoupeného místa
	int SlotPriceIncrease = 500;     // každé další místo je dražší o tolik
	int CreatePrice = 0;             // cena založení skupiny
	bool MembersCanInvite = false;   // false = zvát může jen vedoucí
	bool MembersCanWithdraw = true;  // false = ze společného účtu vybírá jen vedoucí
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
}

class OS_GroupsData
{
	int NextId = 1;
	ref array<ref OS_Group> Groups = new array<ref OS_Group>;
}

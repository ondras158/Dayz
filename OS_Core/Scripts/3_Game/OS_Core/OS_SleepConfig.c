// Únava a spánek. Nastavení: $profile:OS_Core/Sleep.json
// Únava roste časem (rychleji při běhu). Vyspíš se v menu emocí (klávesa L) -> Spát.

class OS_SleepConfig
{
	bool Enabled = true;
	float FatiguePerMinute = 1.0;          // přírůstek únavy za minutu v klidu (0-100)
	float JogMultiplier = 1.5;             // při běhu
	float SprintMultiplier = 2.5;          // při sprintu
	float TiredAt = 70;                    // unavený: výdrž max. 60 %
	float ExhaustedAt = 90;                // vyčerpaný: výdrž max. 30 % a pomalu ubývá zdraví
	float ExhaustedHealthLossPerSecond = 0.02;
	float MinFatigueToSleep = 20;          // spát jde až od téhle únavy
	float SleepRecoveryPerMinute = 8;      // úbytek únavy za minutu spánku
	float ShelterMultiplier = 2;           // ve stanu (do 4 m) nebo ve vlastní base se spí rychleji
	float SleepHealthPerSecond = 0.05;     // doplňování zdraví ve spánku
	bool NoSleepInPvP = true;              // ve PvP zóně (a s běžícím PvP časovačem) nejde usnout
	float EmoteAnnounceRadius = 30;        // emoce z menu se oznámí hráčům v okolí (0 = ne)
}

//! stav spánku na klientovi
class OS_ClientSleep
{
	static float Fatigue;        // 0..1
	static bool Sleeping;
	static float SleepStarted;
}

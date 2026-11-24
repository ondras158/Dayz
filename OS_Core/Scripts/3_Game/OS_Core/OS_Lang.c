// Překlady módu (čeština, angličtina, němčina, polština + vlastní jazyky).
//
// Jazyk se nastavuje na serveru:   $profile:OS_Core/Language.json   { "Language": "en" }
// Texty každého jazyka:            $profile:OS_Core/Lang/<jazyk>.json
//   - soubory cs/en/de/pl se vytvoří samy při prvním startu a jdou libovolně upravovat,
//   - vlastní jazyk: zkopíruj třeba en.json na ru.json, přelož hodnoty a dej "Language": "ru".
// Klíčem je vždy původní český text, hodnotou překlad. %1, %2... jsou čísla / jména doplněná za běhu.
// Klienti dostanou jazyk a upravené texty od serveru po připojení.

class OS_LanguageConfig
{
	string Language = "cs";
	string Info = "cs = cestina, en = English, de = Deutsch, pl = Polski. Texty upravis v Lang/<jazyk>.json";
}

class OS_LangFile
{
	ref map<string, string> Texts = new map<string, string>;
}

class OS_Lang
{
	static const string LANG_DIR = "Lang/";
	static const int CHUNK = 60;   // textů v jedné zprávě klientovi

	protected static ref map<string, string> s_Texts = new map<string, string>;
	protected static string s_Language = "cs";

	// upravené texty (rozdíl proti vestavěným) - server je posílá klientům
	protected static ref array<string> s_OverrideKeys = new array<string>;
	protected static ref array<string> s_OverrideValues = new array<string>;

	static string Language()
	{
		return s_Language;
	}

	static string T(string text)
	{
		if (text == "")
			return text;
		string translated;
		if (s_Texts.Find(text, translated) && translated != "")
			return translated;
		return text;
	}

	static string F(string fmt, string p1 = "", string p2 = "", string p3 = "", string p4 = "", string p5 = "", string p6 = "", string p7 = "", string p8 = "", string p9 = "")
	{
		string result = T(fmt);
		result.Replace("%1", p1);
		result.Replace("%2", p2);
		result.Replace("%3", p3);
		result.Replace("%4", p4);
		result.Replace("%5", p5);
		result.Replace("%6", p6);
		result.Replace("%7", p7);
		result.Replace("%8", p8);
		result.Replace("%9", p9);
		return result;
	}

	//! vestavěné texty jazyka (bez úprav ze souboru)
	protected static void LoadBuiltIn(string lang)
	{
		s_Texts = new map<string, string>;
		s_Language = lang;
		OS_LangData.Fill(lang, s_Texts);
	}

	// ------------------------------------------------------------ server

	static void LoadServer()
	{
		MakeDirectory(OS_DIR);
		MakeDirectory(OS_DIR + LANG_DIR);

		OS_LanguageConfig cfg = OS_ConfigFile<OS_LanguageConfig>.Load("Language.json", new OS_LanguageConfig(), new OS_LanguageConfig());
		string lang = cfg.Language;
		lang.TrimInPlace();
		lang.ToLower();
		if (lang == "")
			lang = "cs";

		// soubory vestavěných jazyků pro úpravy
		foreach (string builtIn : OS_LangData.Languages())
		{
			WriteDefaultFile(builtIn);
		}

		LoadBuiltIn(lang);
		s_OverrideKeys = new array<string>;
		s_OverrideValues = new array<string>;

		string path = OS_DIR + LANG_DIR + lang + ".json";
		if (!FileExist(path))
		{
			Print(OS_TAG + "Jazyk " + lang + ": soubor " + path + " neexistuje, pouzivam vestavene texty.");
			return;
		}

		OS_LangFile file = new OS_LangFile();
		string err;
		if (!JsonFileLoader<OS_LangFile>.LoadFile(path, file, err) || !file.Texts)
		{
			Print(OS_TAG + "CHYBA v " + path + " - pouzivam vestavene texty. " + err);
			return;
		}

		foreach (string key, string value : file.Texts)
		{
			if (value == "")
				continue;
			string builtInValue;
			bool known = s_Texts.Find(key, builtInValue);
			if (known && builtInValue == value)
				continue;
			s_Texts.Set(key, value);
			s_OverrideKeys.Insert(key);
			s_OverrideValues.Insert(value);
		}

		Print(OS_TAG + "Jazyk: " + lang + " (upravenych textu: " + s_OverrideKeys.Count().ToString() + ")");
	}

	protected static void WriteDefaultFile(string lang)
	{
		string path = OS_DIR + LANG_DIR + lang + ".json";
		if (FileExist(path))
			return;

		OS_LangFile file = new OS_LangFile();
		OS_LangData.Fill(lang, file.Texts);
		string err;
		if (!JsonFileLoader<OS_LangFile>.SaveFile(path, file, err))
			Print(OS_TAG + "CHYBA při ukládání " + path + ": " + err);
	}

	//! jazyk + upravené texty pro připojeného hráče
	static void SendToClient(PlayerIdentity identity)
	{
		if (!identity)
			return;

		int total = s_OverrideKeys.Count();
		int start = 0;
		bool first = true;
		while (first || start < total)
		{
			array<string> keys = new array<string>;
			array<string> values = new array<string>;
			int end = Math.Min(start + CHUNK, total);
			for (int i = start; i < end; i++)
			{
				keys.Insert(s_OverrideKeys[i]);
				values.Insert(s_OverrideValues[i]);
			}

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(s_Language);
			rpc.Write(first);
			rpc.Write(keys);
			rpc.Write(values);
			rpc.Send(null, OS_RPC_LANG, true, identity);

			first = false;
			start = end;
		}
	}

	// ------------------------------------------------------------ klient

	static void OnClientRPC(ParamsReadContext ctx)
	{
		string lang;
		bool reset;
		array<string> keys = new array<string>;
		array<string> values = new array<string>;
		if (!ctx.Read(lang) || !ctx.Read(reset) || !ctx.Read(keys) || !ctx.Read(values))
			return;

		if (reset || lang != s_Language)
			LoadBuiltIn(lang);

		int count = Math.Min(keys.Count(), values.Count());
		for (int i = 0; i < count; i++)
		{
			s_Texts.Set(keys[i], values[i]);
		}
	}
}

string OS_T(string text)
{
	return OS_Lang.T(text);
}

string OS_F(string fmt, string p1 = "", string p2 = "", string p3 = "", string p4 = "", string p5 = "", string p6 = "", string p7 = "", string p8 = "", string p9 = "")
{
	return OS_Lang.F(fmt, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

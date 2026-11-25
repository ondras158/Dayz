// Kdo právě mluví (vanilla hlasový chat) - pro HUD.

class OS_ClientVoice
{
	static ref array<string> SpeakerIds = new array<string>;
	static ref array<string> SpeakerNames = new array<string>;

	static void Start(string name, string id)
	{
		if (SpeakerIds.Find(id) > -1)
			return;
		SpeakerIds.Insert(id);
		SpeakerNames.Insert(name);
	}

	static void Stop(string id)
	{
		int index = SpeakerIds.Find(id);
		if (index < 0)
			return;
		SpeakerIds.Remove(index);
		SpeakerNames.Remove(index);
	}
}

modded class VONManagerImplementation
{
	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);

		if (eventTypeId == VONStartSpeakingEventTypeID)
		{
			VONStartSpeakingEventParams startParams;
			if (Class.CastTo(startParams, params))
				OS_ClientVoice.Start(startParams.param1, startParams.param2);
		}
		else if (eventTypeId == VONStopSpeakingEventTypeID)
		{
			VONStopSpeakingEventParams stopParams;
			if (Class.CastTo(stopParams, params))
				OS_ClientVoice.Stop(stopParams.param2);
		}
	}
}

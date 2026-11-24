// Společný základ všech menu módu.

class BC_MenuBase : UIScriptedMenu
{
	string LayoutName()
	{
		return "";
	}

	override Widget Init()
	{
		layoutRoot = g_Game.GetWorkspace().CreateWidgets(BC_LAYOUT_DIR + LayoutName());
		OnCreated();
		return layoutRoot;
	}

	//! widgety jsou vytvořené
	void OnCreated()
	{
	}

	override void OnShow()
	{
		super.OnShow();
		if (g_Game.GetMission())
			g_Game.GetMission().AddActiveInputExcludes({"menu"});
	}

	override void OnHide()
	{
		super.OnHide();
		if (g_Game && g_Game.GetMission())
			g_Game.GetMission().RemoveActiveInputExcludes({"menu"}, true);
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
			OnBack();
	}

	//! Esc
	void OnBack()
	{
		Close();
	}

	TextWidget FindText(string name)
	{
		return TextWidget.Cast(layoutRoot.FindAnyWidget(name));
	}

	ButtonWidget FindButton(string name)
	{
		return ButtonWidget.Cast(layoutRoot.FindAnyWidget(name));
	}

	TextListboxWidget FindList(string name)
	{
		return TextListboxWidget.Cast(layoutRoot.FindAnyWidget(name));
	}

	EditBoxWidget FindEdit(string name)
	{
		return EditBoxWidget.Cast(layoutRoot.FindAnyWidget(name));
	}

	void SetStatus(string text)
	{
		TextWidget status = FindText("Status");
		if (status)
			status.SetText(text);
	}
}

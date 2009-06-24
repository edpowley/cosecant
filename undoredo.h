#pragma once

class Undoable : public Object
{
public:
	virtual bool operator()() = 0;
	virtual bool undo() = 0;
	virtual QString describe() = 0;

	virtual Ptr<Undoable> merge(const Ptr<Undoable>& other) { return NULL; }
};

class UndoRedo
{
public:
	void doAction(const Ptr<Undoable>& action);
	void undo();
	void redo();

//	void updateUI(const Glib::RefPtr<Gtk::ActionGroup>& actions);

	void clear();

protected:
	std::vector< Ptr<Undoable> > m_undoStack, m_redoStack;
};

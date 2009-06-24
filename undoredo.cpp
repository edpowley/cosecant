#include "stdafx.h"
#include "common.h"
#include "undoredo.h"
//#include "notebookwindow.h"

void UndoRedo::doAction(const Ptr<Undoable> &action)
{
	bool ret = (*action)();
	if (ret)
	{
		m_redoStack.clear();

		Ptr<Undoable> merged;
		if (!m_undoStack.empty())
			merged = m_undoStack.back()->merge(action);

		if (merged)
			m_undoStack.back() = merged;
		else
			m_undoStack.push_back(action);

//		NotebookWindow::updateUndoRedoUI(*this);
	}
}

void UndoRedo::undo()
{
	if (!m_undoStack.empty())
	{
		Ptr<Undoable> action = m_undoStack.back();
		m_undoStack.pop_back();
		m_redoStack.push_back(action);

		action->undo();

//		NotebookWindow::updateUndoRedoUI(*this);
	}
}

void UndoRedo::redo()
{
	if (!m_redoStack.empty())
	{
		Ptr<Undoable> action = m_redoStack.back();
		m_redoStack.pop_back();
		m_undoStack.push_back(action);

		(*action)();

//		NotebookWindow::updateUndoRedoUI(*this);
	}
}
/*
void UndoRedo::updateUI(const Glib::RefPtr<Gtk::ActionGroup>& actions)
{
	struct Local
	{
		static void updateAction(
			const Glib::RefPtr<Gtk::Action>& action,
			const std::vector< Ptr<Undoable> >& stack,
			const QString& baselabel)
		{
			bool sensitive;
			QString label;

			if (!stack.empty())
			{
				sensitive = true;
				label = baselabel + " " + stack.back()->describe();
			}
			else
			{
				sensitive = false;
				label = "Nothing to " + baselabel;
			}

			action->set_sensitive(sensitive);
			action->set_property("label", label);
		}
	};

	Local::updateAction(actions->get_action("Undo"), m_undoStack, "_Undo");
	Local::updateAction(actions->get_action("Redo"), m_redoStack, "_Redo");
}
*/
void UndoRedo::clear()
{
	m_undoStack.clear();
	m_redoStack.clear();
	// todo: updateUI();
}

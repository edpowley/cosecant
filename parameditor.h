#pragma once

#include "machine.h"
#include "undo_command_ids.h"

class ParamEditor : public QScrollArea
{
	Q_OBJECT

public:
	ParamEditor(const Ptr<Machine>& mac, QWidget* parent);

protected:
	Ptr<Machine> m_mac;
};

class ScalarChangeCommand : public QUndoCommand
{
public:
	ScalarChangeCommand(const Ptr<Parameter::Scalar>& param, double newval, bool mergeable);

	virtual int id() const { return ucidScalarParameterChange; }

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand* qcommand);

protected:
	bool m_mergeable;
	Ptr<Parameter::Scalar> m_param;
	double m_oldValue, m_newValue;
};

namespace ParamEditorWidget
{
	class ScalarSlider : public QSlider
	{
		Q_OBJECT

	public:
		ScalarSlider(const Ptr<Parameter::Scalar>& param);

	protected:
		Ptr<Parameter::Scalar> m_param;
		static const int c_intRangeMax = 1 << 30;

		int valueToInt(double value);

		bool m_valueChanging;

	protected slots:
		void onValueChanged(int value);
		void onParameterChanged(double value);
	};

	class ScalarEdit : public QLineEdit
	{
		Q_OBJECT

	public:
		ScalarEdit(const Ptr<Parameter::Scalar>& param);

	protected:
		Ptr<Parameter::Scalar> m_param;
		QLocale m_locale;

		void setTextFromValue(double value);

		virtual void focusOutEvent(QFocusEvent* ev);

	protected slots:
		void onParameterChanged(double value);
		void onReturnPressed();
	};

	class EnumCombo : public QComboBox
	{
		Q_OBJECT

	public:
		EnumCombo(const Ptr<Parameter::Enum>& param);

	protected:
		Ptr<Parameter::Enum> m_param;

		bool m_valueChanging;

	protected slots:
		void onParameterChanged(double value);
		void onCurrentIndexChanged(int index);
	};
};

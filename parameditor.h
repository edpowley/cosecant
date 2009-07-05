#pragma once

#include "machine.h"

class ParamEditor : public QScrollArea
{
	Q_OBJECT

public:
	ParamEditor(const Ptr<Machine>& mac, QWidget* parent);

protected:
	Ptr<Machine> m_mac;
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

	protected slots:
		void onParameterChanged(double value);
		void onCurrentIndexChanged(int index);
	};
};

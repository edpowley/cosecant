#pragma once

#include "machine.h"
#include "parameter.h"
#include "undo_command_ids.h"

class ParamEditor : public QScrollArea
{
	Q_OBJECT

public:
	ParamEditor(const Ptr<Machine>& mac, QDockWidget* parent);

	virtual ~ParamEditor();

	QDockWidget* getDock() { return m_parent; }

protected:
	Ptr<Machine> m_mac;
	QDockWidget* m_parent;

protected slots:
	void updateWindowTitle(const QString& macname);
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

////////////////////////////////////////////////////////////////////////////////

namespace ParamEditorWidget
{
	class ScalarSlider : public QSlider
	{
		Q_OBJECT

	public:
		ScalarSlider(const Ptr<Parameter::Scalar>& param);

	public slots:
		void initFromState();

	protected:
		Ptr<Parameter::Scalar> m_param;
		bool m_logarithmic;

		virtual int valueToInt(double value) = 0;
		virtual double intToValue(int i) = 0;

		bool m_valueChanging;

		QTimer m_stateUpdateTimer;

	protected slots:
		void onValueChanged(int value);
		void onParameterChanged(double value);
		void onParamPinAdded();
		void onParamPinRemoved();
	};

	////////////////////////////////////////////////////////////////////////////

	class ScalarEdit : public QLineEdit
	{
		Q_OBJECT

	public:
		ScalarEdit(const Ptr<Parameter::Scalar>& param);

	protected:
		Ptr<Parameter::Scalar> m_param;
		QLocale m_locale;

		virtual void setTextFromValue(double value);
		virtual double getValueFromText();

		virtual void focusOutEvent(QFocusEvent* ev);

		QTimer m_stateUpdateTimer;

	protected slots:
		void onParameterChanged(double value);
		void onReturnPressed();
		void onParamPinAdded();
		void onParamPinRemoved();
		void onStateUpdateTimer();
	};

	//////////////////////////////////////////////////////////////////////

	class RealSlider : public ScalarSlider
	{
	public:
		RealSlider(const Ptr<Parameter::Scalar>& param);

	protected:
		static const int c_intRangeMax = 1 << 30;

		virtual int valueToInt(double value);
		virtual double intToValue(int i);
	};

	////////////////////////////////////////////////////////////////////////////

	class IntSlider : public ScalarSlider
	{
	public:
		IntSlider(const Ptr<Parameter::Int>& param);

	protected:
		Ptr<Parameter::Int> m_param;

		virtual int valueToInt(double value);
		virtual double intToValue(int i);
	};

	///////////////////////////////////////////////////////////////////////////////

	class TimeSlider : public RealSlider
	{
		Q_OBJECT

	public:
		TimeSlider(const Ptr<Parameter::Time>& param);

	public slots:
		void setUnit(TimeUnit::e unit);

	protected:
		Ptr<Parameter::Time> m_param;
		TimeUnit::e m_unit;
		double m_min, m_max;

		virtual int valueToInt(double value);
		virtual double intToValue(int i);
	};

	//////////////////////////////////////////////////////////////////////////////

	class TimeEdit : public ScalarEdit
	{
		Q_OBJECT

	public:
		TimeEdit(const Ptr<Parameter::Time>& param);

	public slots:
		void setUnit(TimeUnit::e unit);

	protected:
		Ptr<Parameter::Time> m_param;
		TimeUnit::e m_unit;

		virtual void setTextFromValue(double value);
		virtual double getValueFromText();
	};

	//////////////////////////////////////////////////////////////////////////////

	class TimeUnitCombo : public QComboBox
	{
		Q_OBJECT

	public:
		TimeUnitCombo(const Ptr<Parameter::Time>& param);

	signals:
		void signalUnitChanged(TimeUnit::e unit);

	protected:
		Ptr<Parameter::Time> m_param;

		QString getUnitName(TimeUnit::e unit);
		QString getUnitName(unsigned int unit) { return getUnitName( static_cast<TimeUnit::e>(unit) ); }

		int findUnit(TimeUnit::e unit);

	protected slots:
		void onCurrentIndexChanged(int index);
	};

	//////////////////////////////////////////////////////////////////////////////////

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

}; // end namespace ParamEditorWidget

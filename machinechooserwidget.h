#pragma once

class MachineChooserTree : public QTreeWidget
{
	Q_OBJECT
	friend class MachineChooserWidget;

protected:
	MachineChooserTree() : QTreeWidget() {}

	virtual QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;
};

class MachineChooserWidget : public QWidget
{
	Q_OBJECT

public:
	MachineChooserWidget(QWidget* parent = NULL);

	static const char* c_dndMimeType;

protected:
	QVBoxLayout* m_layout;
	MachineChooserTree* m_tree;

	void populate();
	void populateIndexBranch();
	void populateIndexBranch(QTreeWidgetItem* parent, QDomElement el, std::map<QString, bool>& idSeenInIndex);
	void populateUnsortedBranch(QTreeWidgetItem* root, const std::map<QString, bool>& idSeenInIndex);

	static QTreeWidgetItem* addToHierarchy(
		QTreeWidgetItem* root, const QStringList& path, QHash<QStringList, QTreeWidgetItem*>& hierarchy);
};

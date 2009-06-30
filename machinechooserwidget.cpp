#include "stdafx.h"
#include "common.h"
#include "machinechooserwidget.h"

#include "machine.h"
#include "xmlutils.h"

const char* MachineChooserWidget::c_dndMimeType = "application/x-cosecant-machine-factory";

MachineChooserWidget::MachineChooserWidget(QWidget* parent)
: QWidget(parent)
{
	m_layout = new QVBoxLayout;
	m_layout->setContentsMargins(0,0,0,0);

	m_tree = new MachineChooserTree;
	m_tree->setHeaderHidden(true);
	m_tree->setColumnCount(2);
	m_tree->setHeaderLabels(QStringList() << "Name" << "Id");
	m_tree->setColumnHidden(1, true);
	m_tree->setDragEnabled(true);
	m_layout->addWidget(m_tree);

	setLayout(m_layout);

	populate();
}

QMimeData* MachineChooserTree::mimeData(const QList<QTreeWidgetItem*> items) const
{
	QMimeData* data = QTreeWidget::mimeData(items);

	QStringList ids;
	foreach(QTreeWidgetItem* item, items)
	{
		if (item->text(1).length() > 0)
			ids << item->text(1);
	}

	data->setData(MachineChooserWidget::c_dndMimeType, ids.join("\n").toUtf8());
	return data;
}

//////////////////////////////////////////////////////////////////////////////

void MachineChooserWidget::populate()
{
	populateIndexBranch();
}

void MachineChooserWidget::populateIndexBranch()
{
	QTreeWidgetItem* root = new QTreeWidgetItem(QStringList("Index"));
	m_tree->addTopLevelItem(root);

	std::map<QString, bool> idSeenInIndex;
	typedef std::pair< QString, Ptr<MachineFactory> > factorypair;
	BOOST_FOREACH(const factorypair& fp, MachineFactory::s_factories)
	{
		idSeenInIndex[fp.first] = false;
	}

	bpath indexpath = PrefsFile::getAppDataDir() / L"index.xml";
	try
	{
		QDomDocument doc = openXml(QString::fromStdWString(indexpath.file_string()));

		populateIndexBranch(root, doc.documentElement(), idSeenInIndex);
	}
	catch (const XmlError& err)
	{
		root->addChild(new QTreeWidgetItem(QStringList() << "Error" << err.msg()));
	}
}

void MachineChooserWidget::populateIndexBranch(QTreeWidgetItem* parentItem,
											   QDomElement parentEl,
											   std::map<QString, bool>& idSeenInIndex)
{
	for(QDomElement el = parentEl.firstChildElement();
		!el.isNull(); el = el.nextSiblingElement())
	{
		if (el.tagName() == "folder")
		{
			QString label = getAttribute<QString>(el, "label");
			QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(label));
			parentItem->addChild(item);
			populateIndexBranch(item, el, idSeenInIndex);
			if (item->childCount() == 0)
			{
				parentItem->removeChild(item);
				delete item;
			}
		}
		else if (el.tagName() == "machine")
		{
			QString id = getAttribute<QString>(el, "id");
			std::map<QString, bool>::iterator seeniter = idSeenInIndex.find(id);
			if (seeniter == idSeenInIndex.end())
			{
				// TODO: do something about missing machines
			}
			else
			{
				seeniter->second = true;
				bool hidden = getAttribute<bool>(el, "hidden", false);
				if (!hidden)
				{
					QString label = getAttribute<QString>(el, "label", id);
					QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << label << id);
					parentItem->addChild(item);
				}
			}
		}
	}
}

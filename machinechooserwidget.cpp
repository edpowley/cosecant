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
    for (const factorypair& fp : MachineFactory::s_factories)
	{
		idSeenInIndex[fp.first] = false;
	}

	QString indexpath = PrefsFile::getAppDataDir() + "/index.xml";
	try
	{
		QDomDocument doc = openXml(indexpath);

		populateIndexBranch(root, doc.documentElement(), idSeenInIndex);
	}
	catch (const XmlError& err)
	{
		qDebug() << "Error in populateIndexBranch:" << err.msg();
	}

	populateUnsortedBranch(root, idSeenInIndex);
}

void MachineChooserWidget::populateUnsortedBranch(QTreeWidgetItem* root, const std::map<QString, bool>& idSeenInIndex)
{
	QTreeWidgetItem* unsortedRoot = new QTreeWidgetItem(QStringList("Unsorted"));
	root->addChild(unsortedRoot);

	bool haveUnsortedMachines = false;
	typedef std::pair<QString, bool> stringboolpair;
	BOOST_FOREACH(const stringboolpair& sb, idSeenInIndex)
	{
		if (!sb.second)
		{
			haveUnsortedMachines = true;
			QStringList pathHead = sb.first.split('/');
			QString pathTail = pathHead.takeLast();
            QTreeWidgetItem* parent = addToHierarchy(unsortedRoot, pathHead);
			QString name = QString("%1 [%2]").arg(MachineFactory::get(sb.first)->getDesc()).arg(sb.first);
			new QTreeWidgetItem(parent, QStringList() << name << sb.first);
		}
	}
	if (!haveUnsortedMachines)
	{
		root->removeChild(unsortedRoot);
		delete unsortedRoot;
	}
}

QTreeWidgetItem* findChildItem(QTreeWidgetItem* parent, const QString& name)
{
    for (int childIndex = 0; childIndex < parent->childCount(); childIndex++)
    {
        auto child = parent->child(childIndex);
        if (child->text(0) == name)
            return child;
    }

    return nullptr;
}

QTreeWidgetItem* MachineChooserWidget::addToHierarchy(QTreeWidgetItem* root, const QStringList& path)
{
	if (path.isEmpty()) return root;

    QTreeWidgetItem* node = root;
    for (auto pathElement : path)
    {
        QTreeWidgetItem* item = findChildItem(node, pathElement);
        if (!item)
        {
            item = new QTreeWidgetItem(node, QStringList(pathElement));
        }

        node = item;
    }

    return node;
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
                //bool hidden = getAttribute<bool>(el, "hidden", false);
                bool hidden = false;
				if (!hidden)
				{
                    QString label = getAttribute<QString>(el, "label");
					QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << label << id);
					parentItem->addChild(item);
				}
			}
		}
	}
}

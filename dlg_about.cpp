#include "stdafx.h"
#include "common.h"
#include "dlg_about.h"
#include "version.h"

Dlg_About::Dlg_About(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
	ui.setupUi(this);

	QString html = ui.textBrowser->toHtml();
	html.replace("COSECANT_VERSION", getVersionString());
	ui.textBrowser->setHtml(html);

	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setScene(&m_scene);

	QPixmap pixmap(":/CosecantMainWindow/images/splash.png");
	m_scene.setSceneRect(pixmap.rect());
	int sx = pixmap.width() / c_numCols;
	int sy = pixmap.height() / c_numRows;

	for (int y=0; y<c_numRows; y++)
	{
		for (int x=0; x<c_numCols; x++)
		{
			Item* item = m_item[x][y] = new Item;
			item->setPixmap(pixmap.copy(x*sx, y*sy, sx, sy));
			m_scene.addItem(item);
			item->setPos(x*sx+sx/2, y*sy+sy/2);
			item->setOffset(-sx/2, -sy/2);

			if (x > 0)
			{
				item->addNeighbour(m_item[x-1][y]);
				m_item[x-1][y]->addNeighbour(item);
			}
			if (y > 0)
			{
				item->addNeighbour(m_item[x][y-1]);
				m_item[x][y-1]->addNeighbour(item);
			}
		}
	}

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimer()) );
	m_timer.setSingleShot(false);
	m_timer.start(1000 / 25);

	QTimer::singleShot(0, this, SLOT(onShow()));
}

void Dlg_About::resizeEvent(QResizeEvent* ev)
{
	QDialog::resizeEvent(ev);
	ui.graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
	ev->accept();
}

Dlg_About::Item::Item() : m_rotspeed(0), m_angle(0)
{
	setAcceptHoverEvents(true);
}

Dlg_About::~Dlg_About()
{
}

void Dlg_About::onShow()
{
	ui.graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
}

void Dlg_About::onTimer()
{
	for (int y=0; y<c_numRows; y++)
	{
		for (int x=0; x<c_numCols; x++)
		{
			m_item[x][y]->update();
		}
	}
}

void Dlg_About::Item::update()
{
	if (isUnderMouse()) m_rotspeed += 10;

	rotate(m_rotspeed);
	m_angle = fmod(m_angle + m_rotspeed + 180.0, 360.0) - 180.0;

	m_rotspeed -= 0.1 * m_angle;

	foreach(Item* n, m_neighbours)
	{
		m_rotspeed = 0.8 * m_rotspeed + 0.2 * n->m_rotspeed;
	}

	if (abs(m_rotspeed) < 0.01) m_rotspeed = 0;
}

void Dlg_About::Item::hoverEnterEvent(QGraphicsSceneHoverEvent* ev)
{
}

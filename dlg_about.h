#pragma once

#include "ui_about_dlg.h"

class Dlg_About : public QDialog
{
	Q_OBJECT

public:
	Dlg_About(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Dlg_About();

protected:
	QGraphicsScene m_scene;

	static const int c_numCols = 30;
	static const int c_numRows = 15;

	class Item : public QGraphicsPixmapItem
	{
	public:
		Item();
		void addNeighbour(Item* n) { m_neighbours.push_back(n); }
		void update();

	protected:
		double m_angle;
		double m_rotspeed;
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* ev);

		QList<Item*> m_neighbours;
	};

	Item* m_item[c_numCols][c_numRows];
	void setNeighbours(int x1, int y1, int x2, int y2);

	QTimer m_timer;

	virtual void resizeEvent(QResizeEvent* ev);

protected slots:
	void onShow();
	void onTimer();

private:
	Ui::AboutDlg ui;
};

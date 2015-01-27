/*
 * view.h: view of the auralisation world
 * Copyright 2013 Universidade de Aveiro
 * Funded by FCT project AcousticAVE (PTDC/EEA-ELC/112137/2009)
 *
 * Written by Andre B. Oliveira <abo@ua.pt>
 */

#include <QWidget>

class View : public QWidget
{
	Q_OBJECT

public:
	View();

protected:
	void keyPressEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);

private slots:
	void setOrientation(float, float, float);

private:
	/*
	 * Id of the currently selected item (the item being dragged).
	 * -2   -> no selection
	 * -1   -> listener
	 * >= 0 -> sources
	 */
	short selectedItem;

	/* Current orientation (radians). */
	float roll, pitch, yaw;

	/* The bounding box of the room. */
	float xmin, xmax, ymin, ymax, zmin, zmax;

	/*
	 * Values for converting auralisation coordinates to pixels:
	 * px = x0p + (x - x0) * scale;
	 * py = y0p - (y - y0) * scale;
	 */
	float x0p, y0p, x0, y0, scale;

	/* The projected polygons of the surfaces of the room. */
	QVector<QPolygon> polygons;

	/* Auxiliary function to convert a point from 3D to projected 2D. */
	void convert(float xyz[3], int *x, int *y);
};

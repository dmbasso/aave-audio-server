/*
 * view.cpp: view of the auralisation world
 * Copyright 2013 Universidade de Aveiro
 * Funded by FCT project AcousticAVE (PTDC/EEA-ELC/112137/2009)
 *
 * Written by Andre B. Oliveira <abo@ua.pt>
 */

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <math.h> /* M_PI */

#include "view.h"
#include "../src/test.h"

#define NSOURCES 2

View::View()
	: QWidget()
	, selectedItem(-2)
	, roll(0)
	, pitch(0)
    , yaw(-M_PI/2)
{
	setFocusPolicy(Qt::StrongFocus);
	setMinimumSize(400, 250);

    /* Calculate the boundary box of the room. */
	float c;
	struct aave_surface *surface;
	xmin = ymin = zmin = +9999;
	xmax = ymax = zmax = -9999;
	for (surface = get_aave_engine()->surfaces; surface; surface = surface->next) {
		for (unsigned i = 0; i < surface->npoints; i++) {
			c = surface->points[i][0];
			if (c > xmax)
				xmax = c;
			if (c < xmin)
				xmin = c;
			c = surface->points[i][1];
			if (c > ymax)
				ymax = c;
			if (c < ymin)
				ymin = c;
			c = surface->points[i][2];
			if (c > zmax)
				zmax = c;
			if (c < zmin)
				zmin = c;
		}
	}

	/* The world point that goes to the center of the widget. */
    x0 = (xmax + xmin) / 2;
    y0 = (ymax + ymin) / 2;

	/*
	 * Timer to repaint the widget, to reflect the incremental changes
	 * being made by the geometry thread and the IMU thread.
	 */
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(update()));
	timer->start(200); /* 5Hz */
}

/*
 * Convert point [x,y,z] in auralisation world coordinates
 * to pixel coordinates.
 */
void View::convert(float xyz[3], int *x, int *y)
{
	float zscale = (1 + (xyz[2] - zmin) / (zmax - zmin) * 0.3) * scale;
	*x = x0p + (xyz[0] - x0) * zscale;
	*y = y0p - (xyz[1] - y0) * zscale;
}

void View::keyPressEvent(QKeyEvent *event)
{
	float x;
	float* pos;

	switch (event->key()) {
	case Qt::Key_0:
		set_reflection_order(0);
		break;
	case Qt::Key_1:
		set_reflection_order(1);
		break;
	case Qt::Key_2:
		set_reflection_order(2);
		break;
	case Qt::Key_3:
		set_reflection_order(3);
		break;
	case Qt::Key_4:
		set_reflection_order(4);
		break;
	case Qt::Key_5:
		set_reflection_order(5);
		break;
	case Qt::Key_6:
		set_reflection_order(6);
		break;
	case Qt::Key_7:
		set_reflection_order(7);
		break;
	case Qt::Key_8:
		set_reflection_order(8);
		break;
	case Qt::Key_9:
		set_reflection_order(9);
		break;
	case Qt::Key_Left:
		x = yaw - (M_PI / 180 * 5);
		if (x < -M_PI)
			x += 2 * M_PI;
		yaw = x;
		set_listener_orientation(roll, pitch, yaw);
		break;
	case Qt::Key_Right:
		x = yaw + (M_PI / 180 * 5);
		if (x > M_PI)
			x -= 2 * M_PI;
		yaw = x;
		set_listener_orientation(roll, pitch, yaw);
		break;
	case Qt::Key_Up:
		pos = get_listener_position();
		set_listener_position(pos[0] + 0.2, pos[1], pos[2]);
		break;
	case Qt::Key_Down:
		pos = get_listener_position();
		set_listener_position(pos[0] - 0.2, pos[1], pos[2]);
		break;
	case Qt::Key_G:
        increase_gain();
		break;
	case Qt::Key_H:
        decrease_gain();
		break;
    case Qt::Key_R:
        enable_disable_reverb();
		break;
    case Qt::Key_T:
        get_aave_engine()->reverb->level += 0.1;
        printf("reve level = %f\n",get_aave_engine()->reverb->level);
        break;
    case Qt::Key_Y:
        get_aave_engine()->reverb->level -= 0.1;
        printf("reve level = %f\n",get_aave_engine()->reverb->level);
        break;
	default:
		QWidget::keyPressEvent(event);
		return;
	}

	update();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
	float x, y, z, zscale;

	if (selectedItem==-2)
		return;

	if (selectedItem == -1) {
		float* pos = get_listener_position();
		z = pos[2];
		zscale = (1 + (z - zmin) / (zmax - zmin) * 0.3) * scale;
		x = x0 + (event->x() - x0p) / zscale;
		y = y0 - (event->y() - y0p) / zscale;
		set_listener_position(x, y, z);
	} else {
		float* pos = get_source_position(selectedItem);
		z = pos[2];
		zscale = (1 + (z - zmin) / (zmax - zmin) * 0.3) * scale;
		x = x0 + (event->x() - x0p) / zscale;
		y = y0 - (event->y() - y0p) / zscale;
		printf("moving source %d\n", selectedItem);
		set_source_position(selectedItem, x, y, z);
	}

	update();
}

void View::mousePressEvent(QMouseEvent *event)
{
	int x, y, dx, dy;

	/* See if the selected itemr is the listener. */
	convert(get_listener_position(), &x, &y);
	dx = event->x() - x;
	dy = event->y() - y;
	if (dx * dx + dy * dy < 32) {
		selectedItem = -1;
		return;
	}

	/* See if the selected item is any of the sound sources. */
	for (short i = 0; i < NSOURCES; i++) {
		convert(get_source_position(i), &x, &y);
		dx = event->x() - x;
		dy = event->y() - y;
		if (dx * dx + dy * dy < 32) {
			selectedItem = i;
			printf("moving source %d\n",i);
			return;
		}
	}

	/* The selected item is not the listener nor sound sources. */
	selectedItem = -2;
}

void View::paintEvent(QPaintEvent *event)
{
	int x, y;

	string image_file_path("../../images/");
	const char *images[] = {"cello", "clarinet","double_bass", "flute", "harp", "oboe", "percussion", "trombone", "violin"};
	vector<string> image_files(images, images + NSOURCES);
	string file_type(".png");

	QPainter painter(this);

	/* White background. */
	painter.fillRect(event->rect(), Qt::white);

	/* Draw sound paths. */
	struct aave_sound *snd;
	for (unsigned k = 0; k <= get_reflection_order(); k++) {
		unsigned i = get_reflection_order() - k;
		static const QPen pens[] = {
			QPen(Qt::red),
			QPen(Qt::blue),
			QPen(Qt::green),
			QPen(Qt::cyan),
			QPen(Qt::magenta),
			QPen(Qt::yellow),
			QPen(Qt::black),
			QPen(Qt::gray),
		};
		painter.setPen(pens[i & 7]);
		for (snd = get_aave_engine()->sounds[i]; snd; snd = snd->next) {
			if (!snd->audible)
				continue;
			float *a = snd->source->position;
			for (unsigned j = 0; j < i; j++) {
				float *b = snd->reflection_points[j];
				int x1, y1, x2, y2;
				convert(a, &x1, &y1);
				convert(b, &x2, &y2);
				painter.drawLine(x1, y1, x2, y2);
				a = b;
			}
			int x1, y1, x2, y2;
			convert(a, &x1, &y1);
			convert(get_listener_position(), &x2, &y2);
			painter.drawLine(x1, y1, x2, y2);
		}
	}

	/* Draw surfaces (projected polygons). */
	static const QPen polygonPen(Qt::black);
	painter.setPen(polygonPen);
	for (int i = 0; i < polygons.size(); i++)
		painter.drawPolygon(polygons.at(i));

	/* Draw sound sources. */
	for (int i = 0; i < NSOURCES; i++) {
		string img_file_path = image_file_path + image_files.at(i) + file_type;
		const QPixmap sourcePixmap(img_file_path.c_str());
		convert(get_source_position(i), &x, &y);
		x -= sourcePixmap.width() / 2;
		y -= sourcePixmap.height() / 2;
		painter.drawPixmap(x, y, sourcePixmap);
	}

	/* Draw listener. */
	static const QImage listenerImage("../../images/listener.png");
	QTransform transform;
	transform.rotate(yaw * (180 / M_PI));
	QImage image = listenerImage.transformed(transform,	Qt::SmoothTransformation);
	convert(get_listener_position(), &x, &y);
	x -= image.width() / 2;
	y -= image.height() / 2;
	painter.drawImage(x, y, image);
}

void View::resizeEvent(QResizeEvent * /* event */)
{
	struct aave_surface *surface;
	int x, y;

	/* The pixel at the center of the widget. */
	x0p = width() * 0.5;
	y0p = height() * 0.5;

	/* The scale to apply (constant width/height ratio). */
	float xscale = width() / (xmax - xmin);
	float yscale = height() / (ymax - ymin);
	scale = xscale > yscale ? yscale : xscale;

	/* Leave 22% for altitude and 2% for margins. */
	scale *= 0.76;

	/* Calculate the polygon coordinates for the new window size. */
	polygons.clear();
	for (surface = get_aave_engine()->surfaces; surface; surface = surface->next) {
		QPolygon polygon(surface->npoints);
		for (unsigned i = 0; i < surface->npoints; i++) {
			convert(surface->points[i], &x, &y);
			polygon.setPoint(i, x, y);
		}
		polygons.append(polygon);
	}
}

void View::setOrientation(float roll, float pitch, float yaw)
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
	set_listener_orientation(roll, pitch, yaw);
}

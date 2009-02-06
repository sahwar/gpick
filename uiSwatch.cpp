/*
 * Copyright (c) 2009, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "uiSwatch.h"
#include "Color.h"
#include "MathUtil.h"
#include <math.h>

#define GTK_SWATCH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_SWATCH, GtkSwatchPrivate))

G_DEFINE_TYPE (GtkSwatch, gtk_swatch, GTK_TYPE_DRAWING_AREA);

static gboolean gtk_swatch_expose(GtkWidget *swatch, GdkEventExpose *event);

enum {
	ACTIVE_COLOR_CHANGED, COLOR_CHANGED, LAST_SIGNAL
};

static guint gtk_swatch_signals[LAST_SIGNAL] = { 0 };

static gboolean gtk_swatch_button_release(GtkWidget *swatch, GdkEventButton *event);

static gboolean gtk_swatch_button_press(GtkWidget *node_system, GdkEventButton *event);

/*
 static gboolean
 gtk_swatch_motion_notify (GtkWidget *node_system, GdkEventMotion *event);
 */

typedef struct GtkSwatchPrivate GtkSwatchPrivate;

typedef struct GtkSwatchPrivate {
	Color color[7];
	gint32 current_color;
} GtkSwatchPrivate;

static void gtk_swatch_class_init(GtkSwatchClass *swatch_class) {
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS(swatch_class);
	widget_class = GTK_WIDGET_CLASS(swatch_class);

	/* GtkWidget signals */

	widget_class->expose_event = gtk_swatch_expose;
	widget_class->button_release_event = gtk_swatch_button_release;
	widget_class->button_press_event = gtk_swatch_button_press;

	/*widget_class->button_press_event = gtk_node_view_button_press;
	 widget_class->button_release_event = gtk_node_view_button_release;
	 widget_class->motion_notify_event = gtk_node_view_motion_notify;*/

	g_type_class_add_private(obj_class, sizeof(GtkSwatchPrivate));

	gtk_swatch_signals[ACTIVE_COLOR_CHANGED] = g_signal_new("active_color_changed", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(GtkSwatchClass, active_color_changed), NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

	gtk_swatch_signals[COLOR_CHANGED] = g_signal_new("color_changed", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(GtkSwatchClass, color_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void gtk_swatch_init(GtkSwatch *swatch) {
	gtk_widget_add_events(GTK_WIDGET(swatch), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
}

GtkWidget *
gtk_swatch_new(void) {
	GtkWidget* widget = (GtkWidget*) g_object_new(GTK_TYPE_SWATCH, NULL);
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(widget);

	gtk_widget_set_size_request(GTK_WIDGET(widget), 150, 150);

	for (gint32 i = 0; i < 7; ++i)
		color_set(&ns->color[i], i/7.0);
	ns->current_color = 1;

	return widget;
}

void gtk_swatch_set_color_to_main(GtkSwatch* swatch) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(&ns->color[0], &ns->color[ns->current_color]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}

void gtk_swatch_move_active(GtkSwatch* swatch, gint32 direction) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);

	if (direction < 0) {
		if (ns->current_color == 1) {
			ns->current_color = 7 - 1;
		} else {
			ns->current_color--;
		}
	} else {
		ns->current_color++;

		if (ns->current_color >= 7)
			ns->current_color = 1;

	}

}

void gtk_swatch_get_color(GtkSwatch* swatch, guint32 index, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(&ns->color[index], color);
}

void gtk_swatch_get_main_color(GtkSwatch* swatch, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(&ns->color[0], color);
}

gint32 gtk_swatch_get_active_index(GtkSwatch* swatch) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	return ns->current_color;
}

void gtk_swatch_get_active_color(GtkSwatch* swatch, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(&ns->color[ns->current_color], color);
}

void gtk_swatch_set_color(GtkSwatch* swatch, guint32 index, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(color, &ns->color[index]);

	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}

void gtk_swatch_set_main_color(GtkSwatch* swatch, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(color, &ns->color[0]);

	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}

void gtk_swatch_set_active_index(GtkSwatch* swatch, guint32 index) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	ns->current_color = index;
	//if (ns->current_color<=0) ns->current_color=1;
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}

void gtk_swatch_set_active_color(GtkSwatch* swatch, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);
	color_copy(color, &ns->color[ns->current_color]);

	gtk_widget_queue_draw(GTK_WIDGET(swatch));
	g_signal_emit(GTK_WIDGET(swatch), gtk_swatch_signals[COLOR_CHANGED], 0);
}

void gtk_swatch_set_main_color(GtkSwatch* swatch, guint index, Color* color) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(swatch);

	color_copy(color, &ns->color[0]);

	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}

static void gtk_swatch_draw_hexagon(cairo_t *cr, float x, float y, float radius) {
	cairo_new_sub_path(cr);
	for (int i = 0; i < 6; ++i) {
		cairo_line_to(cr, x + sin(i * PI / 3) * radius, y + cos(i * PI / 3) * radius);
	}
	cairo_close_path(cr);
}

static gboolean gtk_swatch_expose(GtkWidget *widget, GdkEventExpose *event) {
	cairo_t *cr;

	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(widget);

	/* get a cairo_t */
	cr = gdk_cairo_create(widget->window);

	cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_clip(cr);

	//const char* label[]={"1", "2", "3", "4", "5", "6"};

	cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 12);

	//int edges=ns->colors;

#if 0

	cairo_matrix_t matrix;
	cairo_get_matrix(cr, &matrix);

	//cairo_rotate(cr, PI*2/edges/4);

	int dimension_x=int(floor(sqrt(ns->colors+1)));
	int dimension_y=int(ceil((ns->colors+1)/float(dimension_x)));

	cairo_translate(cr, 30, 30);

	int color_index=0;

	for (int y=0;y<4;++y) {
		if (color_index>ns->colors) break;
		for (int x=0;x<3;++x)
		{
			gtk_swatch_draw_ngon2(cr, x, y, 30.0, 6, &ns->color[color_index], 1);
			color_index++;
			if (color_index>ns->colors) break;
		}
	}

	/*for (int r=1;r<=2;++r)
	 for (int i=0;i<edges*r;++i)
	 {
	 gtk_swatch_draw_ngon2(cr, 20, i, r, edges, &ns->color[i], 1);
	 //cairo_rotate(cr, PI*2/(edges*r));


	 Color c;
	 color_get_contrasting(&ns->color[i], &c);
	 cairo_text_extents_t extends;
	 cairo_text_extents(cr,label[i],&extends);
	 cairo_set_source_rgb (cr, c.rgb.red, c.rgb.green, c.rgb.blue);
	 cairo_move_to(cr, 64*cos((180/edges)/(180/PI))*cos(i*(2*PI)/edges)-extends.width/2,  64*cos((180/edges)/(180/PI))*sin(i*(2*PI)/edges)+extends.height/2);
	 cairo_show_text(cr, label[i]);
	 }*/
	cairo_set_matrix(cr, &matrix);
#endif

	cairo_matrix_t matrix;
	cairo_get_matrix(cr, &matrix);
	cairo_translate(cr, 75, 75);

	int edges = 6;

	cairo_set_source_rgb(cr, 0, 0, 0);

	float radius_multi = 50 * cos((180 / edges) / (180 / PI));
	float rotation = -(PI/6 * 4);

	//Draw stroke
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 3);
	for (int i = 1; i < 7; ++i) {
		if (i == ns->current_color)
			continue;
		gtk_swatch_draw_hexagon(cr, radius_multi * cos(rotation + i * (2 * PI) / edges), radius_multi * sin(rotation + i * (2 * PI) / edges), 27);
	}

	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	gtk_swatch_draw_hexagon(cr, radius_multi * cos(rotation + (ns->current_color) * (2 * PI) / edges), radius_multi * sin(rotation + (ns->current_color) * (2
			* PI) / edges), 27);
	cairo_stroke(cr);

	//Draw fill
	for (int i = 1; i < 7; ++i) {
		if (i == ns->current_color)
			continue;
		cairo_set_source_rgb(cr, ns->color[i].rgb.red, ns->color[i].rgb.green, ns->color[i].rgb.blue);
		gtk_swatch_draw_hexagon(cr, radius_multi * cos(rotation + i * (2 * PI) / edges), radius_multi * sin(rotation + i * (2 * PI) / edges), 25.5);
		cairo_fill(cr);
	}

	cairo_set_source_rgb(cr, ns->color[ns->current_color].rgb.red, ns->color[ns->current_color].rgb.green, ns->color[ns->current_color].rgb.blue);
	gtk_swatch_draw_hexagon(cr, radius_multi * cos(rotation + (ns->current_color) * (2 * PI) / edges), radius_multi * sin(rotation + (ns->current_color) * (2
			* PI) / edges), 25.5);
	cairo_fill(cr);

	//Draw center
	cairo_set_source_rgb(cr, ns->color[0].rgb.red, ns->color[0].rgb.green, ns->color[0].rgb.blue);
	gtk_swatch_draw_hexagon(cr, 0, 0, 25.5);
	cairo_fill(cr);

	//Draw numbers
	char numb[2] = " ";
	for (int i = 1; i < 7; ++i) {
		Color c;
		color_get_contrasting(&ns->color[i], &c);

		cairo_text_extents_t extends;
		numb[0] = '0' + i;
		cairo_text_extents(cr, numb, &extends);
		cairo_set_source_rgb(cr, c.rgb.red, c.rgb.green, c.rgb.blue);
		cairo_move_to(cr, radius_multi * cos(rotation + i * (2 * PI) / edges) - extends.width / 2, radius_multi * sin(rotation + i * (2 * PI) / edges)
				+ extends.height / 2);
		cairo_show_text(cr, numb);
	}

	cairo_set_matrix(cr, &matrix);

	cairo_destroy(cr);

	return FALSE;
}

static gboolean gtk_swatch_button_press(GtkWidget *widget, GdkEventButton *event) {
	GtkSwatchPrivate *ns = GTK_SWATCH_GET_PRIVATE(widget);

	if ((event->type == GDK_BUTTON_PRESS) && ((event->button == 1) || (event->button == 3))) {

		vector2 a, b;
		vector2_set(&a, 1, 0);
		vector2_set(&b, event->x - 75, event->y - 75);
		vector2_normalize(&b, &b);

		float angle = acos(vector2_dot(&a, &b));
		if (b.y < 0)
			angle = 2 * PI - angle;
		angle += (PI/6) * 3;

		if (angle < 0)
			angle += PI * 2;
		if (angle > 2 * PI)
			angle -= PI * 2;

		//printf("%f %d\n",angle,(int)floor(angle/((PI*2)/6)));

		ns->current_color = 1 + (int) floor(angle / ((PI*2) / 6));

		g_signal_emit(widget, gtk_swatch_signals[ACTIVE_COLOR_CHANGED], 0, ns->current_color);
		gtk_widget_queue_draw(GTK_WIDGET(widget));


	}
	return FALSE;
}

static gboolean gtk_swatch_button_release(GtkWidget *widget, GdkEventButton *event) {
	//GtkSwatchPrivate *ns=GTK_SWATCH_GET_PRIVATE(widget);

	if ((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {

	}
	return FALSE;
}
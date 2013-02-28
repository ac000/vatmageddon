/*
 * vatmageddon
 *
 * A Gross/Net/VAT calculator
 *
 * Copyright (C) 2010-2013, Andrew Clayton <andrew@digital-domain.net>
 *
 * Released under the General Public License (GPL) version 2.
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libround.h>

#include <glib.h>
#include <gtk/gtk.h>

/* Update for application version. */
#define VERSION		"007.90"

#define DEF_VAT_RATE	20.0	/* Set the default VAT rate */
#define DEF_DP		2	/* Set the default number of decimal places */

#define RND_CEIL		0
#define RND_CEIL0		1
#define RND_FLOOR		2
#define RND_FLOOR0		3
#define RND_ROUND		4
#define RND_ROUND_HALF_UP	5
#define RND_ROUND_HALF_UP0	6
#define RND_ROUND_HALF_DOWN	7
#define RND_ROUND_HALF_DOWN0	8
#define RND_ROUND_HALF_EVEN	9

#define DEF_RND_FUNC	RND_ROUND	/* Set the default rounding function */

GtkWidget *gross_entry;
GtkWidget *net_entry;
GtkWidget *vat_view;
GtkWidget *vat_rate_entry;
GtkWidget *dp_entry;
GtkWidget *rounding_combo;

char val_fmt[5];

static void cb_quit(void)
{
	gtk_main_quit();
}

static void cb_about(void)
{
	GtkWidget *about;
	const gchar *authors[2] = {
		"Andrew Clayton <andrew@digital-domain.net>",
		(const char *)NULL };

	about = gtk_about_dialog_new();

	gtk_window_set_title(GTK_WINDOW(about), "About vatmageddon");
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about),
			"vatmageddon");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
			"Copyright (C) 2010-2013 Andrew Clayton");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),
			(const gchar **)&authors);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
			"http://github.com/ac000/vatmageddon");

	/* Connect the close_button to destroy the widget */
	g_signal_connect(G_OBJECT(about), "response",
			G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show(about);
}

static void cb_help_close(GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_destroy(window);
}

static void cb_help(void)
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *view;
	GtkWidget *button;
	GtkTextBuffer *buf;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(cb_help_close), window);
	gtk_window_set_title(GTK_WINDOW(window), "vatmageddon / help");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 6);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show(box);
	gtk_container_add(GTK_CONTAINER(window), box);

	view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_widget_show(view);
	gtk_container_add(GTK_CONTAINER(box), view);

	button = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(cb_help_close), window);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(box), button);

	buf = gtk_text_buffer_new(NULL);
	gtk_text_buffer_insert_at_cursor(buf, "vatmageddon\n\n", -1);
	gtk_text_buffer_insert_at_cursor(buf, "This is a simple program to "
			"calculate the VAT amount and Net or Gross amount, \n"
			"given either an initial Net or Gross amount.\n\n",
			-1);
	gtk_text_buffer_insert_at_cursor(buf, "Simply enter a Net or Gross "
			"amount and press <enter>. You will then get the \n"
			"VAT amount and the Net/Gross amount.\n\n", -1);
	gtk_text_buffer_insert_at_cursor(buf, "You can set the VAT rate and "
			"the number of decimal places (0..6) you want. You\n"
			"can also set the rounding function to use. See "
			"vatmageddon(1) for details. It\ndefaults to the "
			"glibc round(3) function.\n\n", -1);
	gtk_text_buffer_insert_at_cursor(buf, "The Reset button clears all "
			"entries _and_ resets the VAT rate, decimal places "
			"and\nthe rounding function to their default values."
			"\n\n", -1);
	gtk_text_buffer_insert_at_cursor(buf, "The Clear button _only_ clears "
			"the Gross, Net and VAT values.\n", -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buf);

	gtk_widget_show(window);
}

static void cb_reset(GtkWidget *widget, gpointer data)
{
	GtkTextBuffer *vat_buf = gtk_text_buffer_new(NULL);

	gtk_entry_set_text(GTK_ENTRY(gross_entry), "");
	gtk_entry_set_text(GTK_ENTRY(net_entry), "");
	gtk_text_buffer_set_text(vat_buf, "", -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(vat_view), vat_buf);

	if (strcmp(data, "reset") == 0) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(vat_rate_entry),
				DEF_VAT_RATE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dp_entry), DEF_DP);
		gtk_combo_box_set_active(GTK_COMBO_BOX(rounding_combo),
				DEF_RND_FUNC);
	}
}

static double do_rounding(double to_round)
{
	int dp = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dp_entry));
	char *rounder = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(
				rounding_combo));
	double rf = lr_get_rounding_factor(dp);
	double ret = 0.0;

	if (strstr(rounder, "ceil -"))
		ret = ceil(to_round * rf) / rf;
	else if (strstr(rounder, "ceil0 -"))
		ret = lr_ceil0(to_round, rf);
	else if (strstr(rounder, "floor -"))
		ret = floor(to_round * rf) / rf;
	else if (strstr(rounder, "floor0 -"))
		ret = lr_floor0(to_round, rf);
	else if (strstr(rounder, "round -"))
		ret = round(to_round * rf) / rf;
	else if (strstr(rounder, "round_half_up -"))
		ret = lr_round_half_up(to_round, rf);
	else if (strstr(rounder, "round_half_up0 -"))
		ret = lr_round_half_up0(to_round, rf);
	else if (strstr(rounder, "round_half_down -"))
		ret = lr_round_half_down(to_round, rf);
	else if (strstr(rounder, "round_half_down0 -"))
		ret = lr_round_half_down0(to_round, rf);
	else if (strstr(rounder, "round_half_even -"))
		ret = lr_round_half_even(to_round, rf);

	/* Create the format string for displaying the gross and net values */
	snprintf(val_fmt, sizeof(val_fmt), "%%.%df",
			gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON(dp_entry)));

	return ret;
}

static void calculate_gross(void)
{
	double gross;
	double net;
	double vat_rate;
	double rounded;
	char gross_e[128];

	net = strtod(gtk_entry_get_text(GTK_ENTRY(net_entry)), NULL);
	vat_rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(vat_rate_entry));

	gross =	net * (vat_rate / 100 + 1);
	rounded = do_rounding(gross);
	sprintf(gross_e, val_fmt, rounded);
	gtk_entry_set_text(GTK_ENTRY(gross_entry), gross_e);
}

static void calculate_net(void)
{
	double gross;
        double net;
        double vat_rate;
        double rounded;
        char net_e[128];

	gross = strtod(gtk_entry_get_text(GTK_ENTRY(gross_entry)), NULL);
	vat_rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(vat_rate_entry));

	net = gross / (vat_rate / 100 + 1);
	rounded = do_rounding(net);
	sprintf(net_e, val_fmt, rounded);
	gtk_entry_set_text(GTK_ENTRY(net_entry), net_e);
}

static void calculate_vat(void)
{
	double gross;
	double vat;
	double vat_rate;
	double rounded;
	char vat_e[128];
	GtkTextBuffer *vat_buf = gtk_text_buffer_new(NULL);

	gross = strtod(gtk_entry_get_text(GTK_ENTRY(gross_entry)), NULL);
	vat_rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(vat_rate_entry));

	vat = gross - (gross / (vat_rate / 100 + 1));	
	rounded = do_rounding(vat);
	sprintf(vat_e, val_fmt, rounded);
	gtk_text_buffer_set_text(vat_buf, vat_e, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(vat_view), vat_buf);
}

static void cb_calculate(GtkWidget *widget, gpointer data)
{
	if (strcmp(data, "net") == 0)
		calculate_gross();
	else if (strcmp(data, "gross") == 0)
		calculate_net();

	calculate_vat();
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	
	GtkWidget *gross_hbox;
	GtkWidget *net_hbox;
	GtkWidget *vat_hbox;
	GtkWidget *vr_hbox;
	GtkWidget *ccq_hbox;

	GtkWidget *gross_label;
	GtkWidget *net_label;
	GtkWidget *vat_label;
	GtkWidget *vat_rate_label;
	GtkWidget *dp_label;
	GtkWidget *rounding_label;
	
	GtkWidget *reset_button;
	GtkWidget *clear_button;
	GtkWidget *help_button;
	GtkWidget *about_button;
	GtkWidget *quit_button;

	GdkRGBA rgba;

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(cb_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(window), "vatmageddon");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);

	/* Main widget container */
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Gross Container */
	gross_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(gross_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), gross_hbox);

	gross_label = gtk_label_new("Gross (incl. vat)");
	gtk_label_set_width_chars(GTK_LABEL(gross_label), 20);
	gtk_widget_show(gross_label);
	gtk_container_add(GTK_CONTAINER(gross_hbox), gross_label);

	gross_entry = gtk_entry_new();
	gtk_widget_show(gross_entry);
	gtk_container_add(GTK_CONTAINER(gross_hbox), gross_entry);
	g_signal_connect(G_OBJECT(gross_entry), "activate",
			G_CALLBACK(cb_calculate), "gross");

	/* Net Container */
	net_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(net_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), net_hbox);

	net_label = gtk_label_new("Net (excl. vat)");
	gtk_label_set_width_chars(GTK_LABEL(net_label), 20);
	gtk_widget_show(net_label);
	gtk_container_add(GTK_CONTAINER(net_hbox), net_label);

	net_entry = gtk_entry_new();
	gtk_widget_show(net_entry);
	gtk_container_add(GTK_CONTAINER(net_hbox), net_entry);
	g_signal_connect(G_OBJECT(net_entry), "activate",
			G_CALLBACK(cb_calculate), "net");

	/* VAT Container */
	vat_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(vat_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), vat_hbox);

	vat_label = gtk_label_new("VAT");
	gtk_label_set_width_chars(GTK_LABEL(vat_label), 20);
	gtk_widget_show(vat_label);
	gtk_container_add(GTK_CONTAINER(vat_hbox), vat_label);

	vat_view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(vat_view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(vat_view), FALSE);
	g_object_set(vat_view, "width_request", 155, NULL);
	gdk_rgba_parse(&rgba, "#ebf5ff");
	gtk_widget_override_background_color(vat_view, GTK_STATE_FLAG_NORMAL,
			&rgba);
	gtk_widget_show(vat_view);
	gtk_container_add(GTK_CONTAINER(vat_hbox), vat_view);

	/* VAT Rate and Rounding method container */
	vr_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_show(vr_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), vr_hbox);
	
	vat_rate_label = gtk_label_new(" VAT Rate: ");
	gtk_widget_show(vat_rate_label);
	gtk_container_add(GTK_CONTAINER(vr_hbox), vat_rate_label);
	
	vat_rate_entry = gtk_spin_button_new_with_range(0.0, 100.00, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(vat_rate_entry),
			DEF_VAT_RATE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(vat_rate_entry), TRUE);
	gtk_widget_show(vat_rate_entry);
	gtk_container_add(GTK_CONTAINER(vr_hbox), vat_rate_entry);

	dp_label = gtk_label_new("DP: ");
	gtk_widget_show(dp_label);
	gtk_container_add(GTK_CONTAINER(vr_hbox), dp_label);

	dp_entry = gtk_spin_button_new_with_range(0, 6, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dp_entry), DEF_DP);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dp_entry), TRUE);
	gtk_widget_show(dp_entry);
	gtk_container_add(GTK_CONTAINER(vr_hbox), dp_entry);

	rounding_label = gtk_label_new("Rounding Function: ");
	gtk_widget_show(rounding_label);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_label);

	rounding_combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"ceil - ceil()");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"ceil0 - round away from 0");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"floor - floor()");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"floor0 - round toward 0");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round - round()");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round_half_up - round towards +infinity ");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round_half_up0 - round away from 0");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round_half_down - round towards -infinity");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round_half_down0 - round towards 0");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rounding_combo),
			"round_half_even - aka bankers rounding");
	gtk_combo_box_set_active(GTK_COMBO_BOX(rounding_combo), DEF_RND_FUNC);
	gtk_widget_show(rounding_combo);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_combo);

	/* Reset, Clear, About, Quit */
	ccq_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(ccq_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), ccq_hbox);

	reset_button = gtk_button_new_with_label("Reset");
	g_object_set(reset_button, "expand", TRUE, NULL);
	g_signal_connect(G_OBJECT(reset_button), "clicked",
			G_CALLBACK(cb_reset), "reset");
	gtk_widget_show(reset_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), reset_button);

	clear_button = gtk_button_new_with_label("Clear");
	g_object_set(clear_button, "expand", TRUE, NULL);
	g_signal_connect(G_OBJECT(clear_button), "clicked",
			G_CALLBACK(cb_reset), "clear");
	gtk_widget_show(clear_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), clear_button);

	help_button = gtk_button_new_with_label("Help");
	g_object_set(help_button, "expand", TRUE, NULL);
	g_signal_connect(G_OBJECT(help_button), "clicked",
			G_CALLBACK(cb_help), NULL);
	gtk_widget_show(help_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), help_button);

	about_button = gtk_button_new_with_label("About");
	g_object_set(about_button, "expand", TRUE, NULL);
	g_signal_connect(G_OBJECT(about_button), "clicked",
			G_CALLBACK(cb_about), NULL);
	gtk_widget_show(about_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), about_button);

	quit_button = gtk_button_new_with_label("Quit");
	g_object_set(quit_button, "expand", TRUE, NULL);
	g_signal_connect(G_OBJECT(quit_button), "clicked",
			G_CALLBACK(cb_quit), NULL);
	gtk_widget_show(quit_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), quit_button);

	gtk_widget_show(window);
	gtk_main();

	exit(EXIT_SUCCESS);
}

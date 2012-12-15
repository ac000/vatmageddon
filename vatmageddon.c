/*
 * vatmageddon
 *
 * A Gross/Net/VAT calculator
 *
 * Copyright (C) 2010-2012, Andrew Clayton <andrew@digital-domain.net>
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
#define VERSION		"006"

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
GtkWidget *vat_entry;
GtkWidget *vat_rate_entry;
GtkWidget *dp_entry;
GtkWidget *rounding_combo;


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
				"Copyright (C) 2010-2012 Andrew Clayton");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),
						(const gchar **)&authors);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
					"http://github.com/ac000/vatmageddon");

	/* Connect the close_button to destroy the widget */
	g_signal_connect(G_OBJECT(about), "response",
					G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show(about);
}

static void cb_reset(GtkWidget *widget, gpointer data)
{
	gtk_entry_set_text(GTK_ENTRY(gross_entry), "");
	gtk_entry_set_text(GTK_ENTRY(net_entry), "");
	gtk_entry_set_text(GTK_ENTRY(vat_entry), "");

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
	char *rounder = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
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
	sprintf(gross_e, "%f", rounded);
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
	sprintf(net_e, "%f", rounded);
	gtk_entry_set_text(GTK_ENTRY(net_entry), net_e);
}

static void calculate_vat(void)
{
	double gross;
	double vat;
	double vat_rate;
	double rounded;
	char vat_e[128];

	gross = strtod(gtk_entry_get_text(GTK_ENTRY(gross_entry)), NULL);
	vat_rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(vat_rate_entry));

	vat = gross - (gross / (vat_rate / 100 + 1));	
	rounded = do_rounding(vat);
	sprintf(vat_e, "%f", rounded);
	gtk_entry_set_text(GTK_ENTRY(vat_entry), vat_e);
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
	GtkWidget *quit_button;
	GtkWidget *about_button;

	GdkColor colour;

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy",
						G_CALLBACK(cb_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(window), "vatmageddon");
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	gtk_widget_set_size_request(window, 670, 220);

	/* Main widget container */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Gross Container */
	gross_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(gross_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), gross_hbox);

	gross_label = gtk_label_new("Gross (incl. vat)");
	gtk_widget_show(gross_label);
	gtk_container_add(GTK_CONTAINER(gross_hbox), gross_label);

	gross_entry = gtk_entry_new();
	gtk_widget_show(gross_entry);
	gtk_container_add(GTK_CONTAINER(gross_hbox), gross_entry);
	g_signal_connect(G_OBJECT(gross_entry), "activate",
					G_CALLBACK(cb_calculate), "gross");

	/* Net Container */
	net_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(net_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), net_hbox);

	net_label = gtk_label_new("Net (excl. vat)");
	gtk_widget_show(net_label);
	gtk_container_add(GTK_CONTAINER(net_hbox), net_label);

	net_entry = gtk_entry_new();
	gtk_widget_show(net_entry);
	gtk_container_add(GTK_CONTAINER(net_hbox), net_entry);
	g_signal_connect(G_OBJECT(net_entry), "activate",
					G_CALLBACK(cb_calculate), "net");

	/* VAT Container */
	vat_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(vat_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), vat_hbox);

	vat_label = gtk_label_new("VAT");
	gtk_widget_show(vat_label);
	gtk_container_add(GTK_CONTAINER(vat_hbox), vat_label);

	vat_entry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(vat_entry), FALSE);
	gdk_color_parse("grey", &colour);
	gtk_widget_modify_base(vat_entry, GTK_STATE_NORMAL, &colour);
	gtk_widget_show(vat_entry);
	gtk_container_add(GTK_CONTAINER(vat_hbox), vat_entry);

	/* VAT Rate and Rounding method container */
	vr_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(vr_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), vr_hbox);
	
	vat_rate_label = gtk_label_new("VAT Rate: ");
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

	dp_entry = gtk_spin_button_new_with_range(0, 10, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dp_entry), DEF_DP);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dp_entry), TRUE);
	gtk_widget_show(dp_entry);
	gtk_container_add(GTK_CONTAINER(vr_hbox), dp_entry);

	rounding_label = gtk_label_new("Rounding Function: ");
	gtk_widget_show(rounding_label);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_label);

	rounding_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"ceil - ceil()");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
						"ceil0 - round away from 0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"floor - floor()");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
						"floor0 - round toward 0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round - round()");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
				"round_half_up - round towards +infinity ");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
					"round_half_up0 - round away from 0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
				"round_half_down - round towards -infinity");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
					"round_half_down0 - round towards 0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
				"round_half_even - aka bankers rounding");
	gtk_combo_box_set_active(GTK_COMBO_BOX(rounding_combo), DEF_RND_FUNC);
	gtk_widget_show(rounding_combo);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_combo);

	/* Reset, Clear, About, Quit */
	ccq_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(ccq_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), ccq_hbox);

	reset_button = gtk_button_new_with_label("Reset");
	g_signal_connect(G_OBJECT(reset_button), "clicked",
						G_CALLBACK(cb_reset), "reset");
	gtk_widget_show(reset_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), reset_button);
	
	clear_button = gtk_button_new_with_label("Clear");
	g_signal_connect(G_OBJECT(clear_button), "clicked",
						G_CALLBACK(cb_reset), "clear");
	gtk_widget_show(clear_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), clear_button);

	about_button = gtk_button_new_with_label("About");
	g_signal_connect(G_OBJECT(about_button), "clicked",
						G_CALLBACK(cb_about), NULL);
	gtk_widget_show(about_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), about_button);

	quit_button = gtk_button_new_with_label("Quit");
	g_signal_connect(G_OBJECT(quit_button), "clicked",
						G_CALLBACK(cb_quit), NULL);
	gtk_widget_show(quit_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), quit_button);

	gtk_widget_show(window);
	gtk_main();

	exit(EXIT_SUCCESS);
}	

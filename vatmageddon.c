/*
 * vatmageddon
 *
 * A Gross/Net/VAT calculator
 *
 * Copyright (C) 2010-2011, Andrew Clayton <andrew@digital-domain.net>
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

#include "vatmageddon.h"

/* Update for application version. */
#define VERSION		"002"

/* Set the default VAT rate */
#define DEF_VAT_RATE	20.0

GtkWidget *gross_entry;
GtkWidget *net_entry;
GtkWidget *vat_entry;
GtkWidget *vat_rate_entry;
GtkWidget *dp_entry;

GtkWidget *rounding_combo;


static void cb_quit()
{
	gtk_main_quit();
}

static void cb_about()
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
				"Copyright (C) 2010-2011 Andrew Clayton");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),
						(const gchar **)&authors);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
					"http://github.com/ac000/vatmageddon");

	/* Connect the close_button to destroy the widget */
	g_signal_connect(G_OBJECT(about), "response",
					G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show(about);
}

static void cb_clear_all()
{
	gtk_entry_set_text(GTK_ENTRY(gross_entry), "");
	gtk_entry_set_text(GTK_ENTRY(net_entry), "");
	gtk_entry_set_text(GTK_ENTRY(vat_entry), "");

	gtk_entry_set_editable(GTK_ENTRY(gross_entry), TRUE);
	gtk_entry_set_editable(GTK_ENTRY(net_entry), TRUE);
}

static void cb_calculate(GtkWidget *widget, gpointer data)
{
	if (strcmp(data, "net") == 0)
		calculate_gross();
	else if (strcmp(data, "gross") == 0)
		calculate_net();

	calculate_vat();
}

static double do_rounding(double to_round)
{
	int dp = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dp_entry));
	char *rounder = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
							rounding_combo));
	double rf = get_rounding_factor(dp);
	double ret = 0.0;

	if ((strcmp(rounder, "ceil")) == 0)
		ret = ceil(to_round * rf) / rf;
	else if ((strcmp(rounder, "ceil0")) == 0)
		ret = ceil0(to_round, rf);
	else if ((strcmp(rounder, "floor")) == 0)
		ret = floor(to_round * rf) / rf;
	else if ((strcmp(rounder, "floor0")) == 0)
		ret = floor0(to_round, rf);
	else if ((strcmp(rounder, "round")) == 0)
		ret = round(to_round * rf) / rf;
	else if ((strcmp(rounder, "round_half_up")) == 0)
		ret = round_half_up(to_round, rf);
	else if ((strcmp(rounder, "round_half_up0")) == 0)
		ret = round_half_up0(to_round, rf);
	else if ((strcmp(rounder, "round_half_down")) == 0)
		ret = round_half_down(to_round, rf);
	else if ((strcmp(rounder, "round_half_down0")) == 0)
		ret = round_half_down0(to_round, rf);
	else if ((strcmp(rounder, "round_half_even")) == 0)
		ret = round_half_even(to_round, rf);

	return ret;
}

static void calculate_gross()
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

static void calculate_net()
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

static void calculate_vat()
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
	
	GtkWidget *calculate_button;
	GtkWidget *new_button;
	GtkWidget *quit_button;
	GtkWidget *about_button;

	GdkColor colour;

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy",
						G_CALLBACK(cb_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(window), "vatmageddon");
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	gtk_widget_set_size_request(window, 500, 220);

	/* Main widget container */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Gross Container */
	gross_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(gross_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), gross_hbox);

	gross_label = gtk_label_new("Gross: ");
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

	net_label = gtk_label_new("Net: ");
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

	vat_label = gtk_label_new("VAT: ");
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dp_entry), 2);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dp_entry), TRUE);
	gtk_widget_show(dp_entry);
	gtk_container_add(GTK_CONTAINER(vr_hbox), dp_entry);

	rounding_label = gtk_label_new("Rounding Function: ");
	gtk_widget_show(rounding_label);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_label);

	rounding_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo), "ceil");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo), "ceil0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo), "floor");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo), "floor0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo), "round");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round_half_up");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round_half_up0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round_half_down");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round_half_down0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(rounding_combo),
							"round_half_even");
	gtk_combo_box_set_active(GTK_COMBO_BOX(rounding_combo), 4);
	gtk_widget_show(rounding_combo);
	gtk_container_add(GTK_CONTAINER(vr_hbox), rounding_combo);

	/* Calculate, Clear, Quit */
	ccq_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(ccq_hbox);
	gtk_container_add(GTK_CONTAINER(vbox), ccq_hbox);

	calculate_button = gtk_button_new_with_label("Calculate");
	g_signal_connect(G_OBJECT(calculate_button), "clicked",
						G_CALLBACK(cb_calculate), NULL);
	gtk_widget_show(calculate_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), calculate_button);

	new_button = gtk_button_new_with_label("New");
	g_signal_connect(G_OBJECT(new_button), "clicked",
						G_CALLBACK(cb_clear_all), NULL);
	gtk_widget_show(new_button);
	gtk_container_add(GTK_CONTAINER(ccq_hbox), new_button);
	
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

	exit(0);
}	

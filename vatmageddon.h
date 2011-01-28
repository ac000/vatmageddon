/* 
 * vatmageddon.h
 *
 * Function definitions for vatmadeggon.c
 *
 * Copyright (C) 2010-2011 Andrew Clayton <andrew@digital-domain.net>
 *
 * Released under the General Public License (GPL) version 2
 * See COPYING
 */

static void cb_quit();
static void cb_about();
static void cb_reset(GtkWidget *widget, gpointer data);
static void cb_calculate(GtkWidget *widget, gpointer data);
static double do_rounding(double to_round);
static void calculate_gross();
static void calculate_net();
static void calculate_vat();

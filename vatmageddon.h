/* 
 * vatmageddon.h
 *
 * Function definitions for vatmadeggon.c
 *
 * Copyright (C) 2010 Andrew Clayton <andrew@digital-domain.net>
 *
 * Released under the General Public License (GPL) version 2
 * See COPYING
 */

static void cb_quit();
static void cb_about();
static void cb_clear_all();
static void cb_calculate();
static double do_rounding(double to_round);
static void calculate_gross();
static void calculate_net();
static void calculate_vat();

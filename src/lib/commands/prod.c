/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  prod.c: Calculate production levels
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1987
 *     Steve McClure, 1997-2000
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "product.h"

static void prprod(coord, coord, int, double, double, int, char,
		   double, double, double, char[], int[], int[], int);

int
count_pop(int n)
{
    int i;
    int pop = 0;
    struct sctstr *sp;

    for (i = 0; NULL != (sp = getsectid(i)); i++) {
	if (sp->sct_own != n)
	    continue;
	if (sp->sct_oldown != n)
	    continue;
	pop += sp->sct_item[I_CIVIL];
    }
    return pop;
}

int
prod(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    struct pchrstr *pp;
    double p_e;
    double maxr;		/* floating version of max */
    double prodeff;
    double real;		/* floating pt version of act */
    int work;
    int totpop;
    int material_consume;	/* actual production */
    double cost;
    int i;
    int max_consume;		/* production w/infinite materials */
    int nsect;
    double take;
    double mtake;
    int there;
    int unit_work;		/* sum of component amounts */
    int mat_limit, res_limit;
    double worker_limit;
    i_type it;
    i_type vtype;
    unsigned char *resource;
    char cmnem[MAXPRCON];
    int cuse[MAXPRCON], cmax[MAXPRCON];
    int lcms, hcms;
    int civs;
    int uws;
    int bwork;
    int twork;
    int type;
    int eff;
    int maxpop;
    int otype;
    char mnem;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;

	civs = (1.0 + obrate * etu_per_update) * sect.sct_item[I_CIVIL];
	uws = (1.0 + uwbrate * etu_per_update) * sect.sct_item[I_UW];
	natp = getnatp(sect.sct_own);
	maxpop = max_pop(natp->nat_level[NAT_RLEV], &sect);

	work = new_work(&sect,
			total_work(sect.sct_work, etu_per_update,
				   civs, sect.sct_item[I_MILIT], uws,
				   maxpop));
	bwork = work / 2;

	if (sect.sct_off)
	    continue;
	type = sect.sct_type;
	eff = sect.sct_effic;
	if (sect.sct_newtype != type) {
	    twork = (eff + 3) / 4;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    bwork -= twork;
	    eff -= twork * 4;
	    otype = type;
	    if (eff <= 0) {
		type = sect.sct_newtype;
		eff = 0;
	    }
	    if (!eff && IS_BIG_CITY(otype) && !IS_BIG_CITY(type)) {
		maxpop = max_population(natp->nat_level[NAT_RLEV],
					type, eff);
		work = new_work(&sect,
				total_work(sect.sct_work, etu_per_update,
					   civs, sect.sct_item[I_MILIT],
					   uws, maxpop));
		bwork = MIN(work / 2, bwork);
	    }
	    twork = 100 - eff;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    if (dchr[type].d_lcms > 0) {
		lcms = sect.sct_item[I_LCM];
		lcms /= dchr[type].d_lcms;
		if (twork > lcms)
		    twork = lcms;
	    }
	    if (dchr[type].d_hcms > 0) {
		hcms = sect.sct_item[I_HCM];
		hcms /= dchr[type].d_hcms;
		if (twork > hcms)
		    twork = hcms;
	    }
	    bwork -= twork;
	    eff += twork;
	} else if (eff < 100) {
	    twork = 100 - eff;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    bwork -= twork;
	    eff += twork;
	}
	work = (work + 1) / 2 + bwork;
	if (eff < 60)
	    continue;

	if (type == SCT_ENLIST) {
	    int maxmil;
	    int enlisted;

	    if (sect.sct_own != sect.sct_oldown)
		continue;
	    enlisted = 0;
	    maxmil = MIN(civs, maxpop) / 2 - sect.sct_item[I_MILIT];
	    if (maxmil > 0) {
		enlisted = (etu_per_update
			    * (10 + sect.sct_item[I_MILIT])
			    * 0.05);
		if (enlisted > maxmil)
		    enlisted = maxmil;
	    }
	    if (enlisted < 0)
		enlisted = 0;
	    prprod(sect.sct_x, sect.sct_y, type, eff / 100.0, 1.0, work,
		   ichr[I_MILIT].i_mnem, enlisted, maxmil, enlisted * 3,
		   "c\0\0", &enlisted, &enlisted, nsect++);
	    continue;
	}

	if (dchr[type].d_prd < 0)
	    continue;
	pp = &pchr[dchr[type].d_prd];
	vtype = pp->p_type;
	if (pp->p_nrndx)
	    resource = (unsigned char *)&sect + pp->p_nrndx;
	else
	    resource = NULL;

	mat_limit = prod_materials_cost(pp, sect.sct_item, &unit_work);

	/* sector p.e. */
	p_e = eff / 100.0;
	if (resource) {
	    unit_work++;
	    p_e *= *resource / 100.0;
	}
	if (unit_work == 0)
	    unit_work = 1;

	worker_limit = work * p_e / (double)unit_work;
	res_limit = prod_resource_limit(pp, resource);

	max_consume = res_limit;
	if (max_consume > worker_limit)
	    max_consume = (int)worker_limit;
	material_consume = MIN(max_consume, mat_limit);

	prodeff = prod_eff(type, natp->nat_level[pp->p_nlndx]);
	real = (double)material_consume * prodeff;
	maxr = (double)max_consume * prodeff;

	if (vtype != I_NONE) {
	    real = MIN(999.0, real);
	    maxr = MIN(999.0, maxr);
	    if (real < 0.0)
		real = 0.0;
	    /* production backlog? */
	    there = MIN(ITEM_MAX, sect.sct_item[vtype]);
	    real = MIN(real, ITEM_MAX - there);
	}

	if (prodeff != 0) {
	    take = real / prodeff;
	    mtake = maxr / prodeff;
	} else
	    mtake = take = 0.0;

	cost = take * pp->p_cost;
	if (opt_TECH_POP) {
	    if (pp->p_level == NAT_TLEV) {
		totpop = count_pop(sect.sct_own);
		if (totpop > 50000)
		    cost *= totpop / 50000.0;
	    }
	}

	for (i = 0; i < MAXPRCON; ++i) {
	    cmnem[i] = cuse[i] = cmax[i] = 0;
	    if (!pp->p_camt[i])
		continue;
	    it = pp->p_ctype[i];
	    if (CANT_HAPPEN(it <= I_NONE || I_MAX < it))
		continue;
	    cmnem[i] = ichr[it].i_mnem;
	    cuse[i] = (int)(take * pp->p_camt[i] + 0.5);
	    cmax[i] = (int)(mtake * pp->p_camt[i] + 0.5);
	}

	if (pp->p_type != I_NONE)
	    mnem = ichr[vtype].i_mnem;
	else if (pp->p_level == NAT_TLEV || pp->p_level == NAT_RLEV)
	    mnem = '.';
	else
	    mnem = 0;
	prprod(sect.sct_x, sect.sct_y, type, p_e, prodeff, work,
	       mnem, real, maxr, cost,
	       cmnem, cuse, cmax, nsect++);
    }
    player->simulation = 0;
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}

static void
prprod(coord x, coord y, int type, double p_e, double prodeff, int work,
       char mnem, double make, double max, double cost,
       char cmnem[], int cuse[], int cmax[], int nsect)
{
    int i;

    if (nsect == 0) {
	pr("PRODUCTION SIMULATION\n");
	pr("   sect  des eff avail  make p.e. cost   use1 use2 use3  max1 max2 max3   max\n");
    }

    prxy("%4d,%-4d", x, y);
    pr(" %c %3.0f%% %5d", dchr[type].d_mnem, p_e * 100.0, work);
    if (mnem == '.')
	pr(" %5.2f", make);
    else
	pr(" %4.0f%c", make, mnem ? mnem : ' ');
    pr(" %.2f $%-5.0f", prodeff, cost);
    for (i = 0; i < 3; i++) {
	if (i < MAXPRCON && cmnem[i])
	    pr("%4d%c", cuse[i], cmnem[i]);
	else
	    pr("     ");
    }
    pr(" ");
    for (i = 0; i < 3; i++) {
	if (i < MAXPRCON && cmnem[i])
	    pr("%4d%c", cmax[i], cmnem[i]);
	else
	    pr("     ");
    }
    if (mnem == '.')
	pr(" %5.2f\n", max);
    else
	pr(" %5.0f\n", max);
}
